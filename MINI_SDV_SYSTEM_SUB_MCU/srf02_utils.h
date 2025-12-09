/*
 * srf_02_utils.h
 *
 * Created: 8/28/2025 2:00:01 PM
 *  Author: jm
 */ 

#ifndef F_CPU
#define F_CPU 14745600UL
#endif

#define _USE_SAFETY_TWI_

// Standard headers for AVR programming
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Standard C libraries
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// User-defined library headers
#include "lcd_gcc.h"
#include "twi_gcc.h"
#include "usart_gcc.h"


typedef enum{
	SRF02_ADDR_0 = 0XE0,
	SRF02_ADDR_1 = 0XE2,
	SRF02_ADDR_2 = 0XE4,
	SRF02_ADDR_3 = 0XE6,
	SRF02_ADDR_4 = 0XE8,
	SRF02_ADDR_5 = 0XEA,
	SRF02_ADDR_6 = 0XEC,
	SRF02_ADDR_7 = 0XEE,
	SRF02_ADDR_8 = 0XF0,
	SRF02_ADDR_9 = 0XF2,
	SRF02_ADDR_10 = 0XF4,
	SRF02_ADDR_11 = 0XF6,
	SRF02_ADDR_12 = 0XF8,
	SRF02_ADDR_13 = 0XFA,
	SRF02_ADDR_14 = 0XFC,
	SRF02_ADDR_15 = 0XFE,
	
}srf02_addr_t;


typedef enum{
	COM_REG=0,
	SRF02_1st_Seq_change= 0xA0,
	SRF02_2nd_Seq_change=0xAA,
	SRF02_3rd_Seq_change=0xA5
}srf02_seq_change_t;

#define SRF02_CMD_RANGE_IN 0x50
#define SRF02_CMD_RANGE_CM 0x51
#define SRF02_CMD_RANGE_US 0x52



unsigned char SRF02_i2C_Write(char address,char reg, char data){
	unsigned char ret_err=0;
	ret_err=TWI_Start();
	ret_err=TWI_Write_SLAW(address);
	if(ret_err!=0)return ret_err;
	ret_err= TWI_Write_Data(reg);
	if(ret_err!=0)return ret_err;
	ret_err= TWI_Write_Data(data);
	if(ret_err!=0)return ret_err;
	TWI_Stop();
	return 0;
	
}
unsigned char SRF02_i2C_Read(char address, char reg, unsigned char* Data){
	char read_data=0;
	unsigned char ret_err=0;
	ret_err=TWI_Start();
	
	ret_err=TWI_Write_SLAW(address);
	if(ret_err!=0)return ret_err;
	ret_err= TWI_Write_Data(reg);
	if(ret_err!=0)return ret_err;
	
	ret_err= TWI_Restart();
	if(ret_err!=0)return ret_err;
	
	ret_err=TWI_Write_SLAR(address);
	if(ret_err!=0)return ret_err;
	ret_err=TWI_Read_Data_NACK(&read_data);
	if(ret_err!=0) return ret_err;
	TWI_Stop();
	*Data=read_data;
	return 0;
}

unsigned char startRanging(char addr){
	return SRF02_i2C_Write(addr,COM_REG,SRF02_CMD_RANGE_CM);
}

unsigned int getRange(char addr, unsigned int *pDistance){
	unsigned char temp;
	unsigned char res =0;
	res= SRF02_i2C_Read(addr,2,&temp);
	if(res) return res;
	*pDistance =temp<<8;
	res = SRF02_i2C_Read(addr,3,&temp);
	if(res) return res;
	*pDistance |=temp;
	return res;
}

unsigned char change_Sonar_Addr(unsigned char ori , unsigned char des){
	unsigned char res =0;
	switch(des)
	{
		case 0xE0:
		case 0xE2:
		case 0xE4:
		case 0xE6:
		case 0xE8:
		case 0xEA:
		case 0xEC:
		case 0xEE:
		case 0xF0:
		case 0xF2:
		case 0xF4:
		case 0xF6:
		case 0xF8:
		case 0xFA:
		case 0xFC:
		case 0xFE:
		res=SRF02_i2C_Write(ori, COM_REG,SRF02_1st_Seq_change);
		if(res) return res;
		res =SRF02_i2C_Write(ori,COM_REG,SRF02_2nd_Seq_change);
		if(res) return res;
		res =SRF02_i2C_Write(ori,COM_REG,SRF02_3rd_Seq_change);
		if(res) return res;
		
		res= SRF02_i2C_Write(ori, COM_REG,des);
		if(res) return res;
		break;
		default:
		return -1;
	}
	return 0;
}
