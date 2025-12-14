/*
 * MINI_SDV_SYSTEM_MAIN_BOOTLOADER.c
 *
 * Created: 2025-12-14 오후 4:08:50
 * Author : LEESANGHUN
 */ 
#define F_CPU 14745600UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef SPM_PAGESIZE
#  define SPM_PAGESIZE 256
#endif

// ==== BOOT 크기(너 fuses에 맞게 조정) ====
// 보통 8KB면 8192
#define BOOTLOADER_SIZE_BYTES   (8UL * 1024UL)

#define FLASH_SIZE_BYTES        ((uint32_t)FLASHEND + 1UL)   // ATmega128: 0x20000
#define APP_END                 (FLASH_SIZE_BYTES - BOOTLOADER_SIZE_BYTES)

// EEPROM OTA 요청 플래그 (앱이 써둠)
#define EE_OTA_REQ_ADDR ((uint8_t*)0)
#define OTA_REQ_MAGIC 0xA5
// ===== UART1 (polling) =====
static void uart1_init(uint32_t baud)
{
	// UBRR = F_CPU/(16*baud)-1
	uint16_t ubrr = (uint16_t)((F_CPU / (16UL * baud)) - 1UL);
	UBRR1H = (uint8_t)(ubrr >> 8);
	UBRR1L = (uint8_t)(ubrr & 0xFF);

	UCSR1A = 0;
	UCSR1B = (1<<RXEN1) | (1<<TXEN1);
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10); // 8N1
}

static void uart1_putc(char c)
{
	while (!(UCSR1A & (1<<UDRE1))) {}
	UDR1 = c;
}

static void uart1_puts(const char *s)
{
	while (*s) uart1_putc(*s++);
}

static void uart1_puts_ln(const char *s)
{
	uart1_puts(s);
	uart1_putc('\n');
}

static int uart1_getc_block(void)
{
	while (!(UCSR1A & (1<<RXC1))) {}
	return UDR1;
}

static bool uart1_readline(char *buf, uint16_t maxlen)
{
	// '\n'까지 읽고 buf에 '\0' 종료. '\r'은 무시
	uint16_t idx = 0;
	while (1)
	{
		int c = uart1_getc_block();
		if (c == '\r') continue;
		if (c == '\n') {
			buf[idx] = '\0';
			return true;
		}
		if (idx < (maxlen - 1)) {
			buf[idx++] = (char)c;
			} else {
			// overflow: 개행까지 버림
			while (1) {
				int d = uart1_getc_block();
				if (d == '\n') break;
			}
			buf[0] = '\0';
			uart1_puts_ln("OTA:NAK:RX_OVERFLOW");
			return false;
		}
	}
}

// ===== Intel HEX 파서 =====
static uint8_t hex_nibble(char c){
	if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
	if (c >= 'A' && c <= 'F') return (uint8_t)(c - 'A' + 10);
	if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
	return 0xFF;
}
static uint8_t parse_u8_hex2(const char *p){
	uint8_t hi = hex_nibble(p[0]);
	uint8_t lo = hex_nibble(p[1]);
	if (hi == 0xFF || lo == 0xFF) return 0xFF;
	return (uint8_t)((hi << 4) | lo);
}
static uint16_t parse_u16_hex4(const char *p){
	uint8_t b0 = parse_u8_hex2(p);
	uint8_t b1 = parse_u8_hex2(p+2);
	if (b0 == 0xFF || b1 == 0xFF) return 0xFFFF;
	return (uint16_t)((uint16_t)b0 << 8) | b1;
}
static bool ihex_checksum_ok(const char *hex){
	if (!hex || hex[0] != ':') return false;

	uint8_t len   = parse_u8_hex2(hex + 1);
	uint16_t addr = parse_u16_hex4(hex + 3);
	uint8_t type  = parse_u8_hex2(hex + 7);
	if (len==0xFF || addr==0xFFFF || type==0xFF) return false;

	uint8_t sum = 0;
	sum += len;
	sum += (uint8_t)(addr >> 8);
	sum += (uint8_t)(addr & 0xFF);
	sum += type;

	const char *dp = hex + 9;
	for (uint8_t i=0;i<len;i++){
		uint8_t b = parse_u8_hex2(dp + i*2);
		if (b == 0xFF) return false;
		sum += b;
	}

	uint8_t chk = parse_u8_hex2(dp + len*2);
	if (chk == 0xFF) return false;
	sum += chk;

	return (sum == 0);
}

// ===== Flash write (page buffer) =====
static uint8_t  page_buf[SPM_PAGESIZE];
static uint32_t cur_page = 0xFFFFFFFFUL;
static uint32_t ext_addr = 0;

static void flash_flush_page(uint32_t page_base)
{
	boot_page_erase(page_base);
	boot_spm_busy_wait();

	for (uint16_t i=0;i<SPM_PAGESIZE;i+=2){
		uint16_t w = (uint16_t)page_buf[i] | ((uint16_t)page_buf[i+1] << 8);
		boot_page_fill(page_base + i, w);
	}

	boot_page_write(page_base);
	boot_spm_busy_wait();
	boot_rww_enable();
}

static void jump_to_app(void)
{
	cli();
	eeprom_update_byte(EE_OTA_REQ_ADDR, 0);
	wdt_disable();
	void (*app_start)(void) = (void(*)(void))0x0000;
	app_start();
}

static void ota_loop_main(void)
{
	uart1_puts_ln("OTA:BOOT:READY:MAIN");

	memset(page_buf, 0xFF, sizeof(page_buf));
	cur_page = 0xFFFFFFFFUL;
	ext_addr = 0;

	char line[128];

	while (1)
	{
		if (!uart1_readline(line, sizeof(line))) {
			continue; // overflow면 NAK 이미 보냄
		}
		if (line[0] == '\0') continue;
		if (line[0] != ':') continue;

		if (!ihex_checksum_ok(line)) {
			uart1_puts_ln("OTA:NAK:BAD_IHEX");
			continue;
		}

		uint8_t  len  = parse_u8_hex2(line + 1);
		uint16_t addr = parse_u16_hex4(line + 3);
		uint8_t  type = parse_u8_hex2(line + 7);
		const char *dp = line + 9;

		if (type == 0x04) {
			// Extended Linear Address
			uint16_t upper = parse_u16_hex4(dp);
			ext_addr = ((uint32_t)upper) << 16;
			uart1_puts_ln("OTA:ACK:DATA:ELA");
			continue;
		}

		if (type == 0x01) {
			// EOF
			if (cur_page != 0xFFFFFFFFUL) {
				flash_flush_page(cur_page);
				cur_page = 0xFFFFFFFFUL;
			}
			// OTA 요청 플래그 클리어
			eeprom_update_byte(EE_OTA_REQ_ADDR, 0);

			uart1_puts_ln("OTA:ACK:END");
			jump_to_app();
		}

		if (type != 0x00) {
			// 나머지 레코드는 스킵
			uart1_puts_ln("OTA:ACK:DATA:SKIP");
			continue;
		}

		// Data record
		for (uint8_t i=0;i<len;i++){
			uint8_t b = parse_u8_hex2(dp + i*2);

			uint32_t abs = ext_addr + (uint32_t)addr + (uint32_t)i;
			if (abs >= APP_END) {
				uart1_puts_ln("OTA:NAK:ADDR_OOB");
				goto abort;
			}

			uint32_t page = (abs / SPM_PAGESIZE) * SPM_PAGESIZE;

			if (page != cur_page) {
				if (cur_page != 0xFFFFFFFFUL) {
					flash_flush_page(cur_page);
				}
				memset(page_buf, 0xFF, sizeof(page_buf));
				cur_page = page;
			}

			page_buf[abs % SPM_PAGESIZE] = b;
		}

		uart1_puts_ln("OTA:ACK:DATA:MAIN");
	}

	abort:
	// 실패 시 플래그 유지(재시도 가능)
	uart1_puts_ln("OTA:NAK:ABORT");
	while (1) { }
}

int main(void)
{
	cli();
	uart1_init(38400);

	uint8_t req = eeprom_read_byte(EE_OTA_REQ_ADDR);

	if (req == OTA_REQ_MAGIC) {
		 ota_loop_main(); 
		} 
	else {
		jump_to_app();
	} 

	while (1) { }
}
