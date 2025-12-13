from serial_manager import SerialManager
from protocol_parser import parse_message
from ota_protocol import OTAUploader
PORT = "/dev/ttyUSB0"
BAUD = 38400

def on_serial_line(line: str):
    parse_message(line)

def main():
    print("=== SDV Supervisor Started ===")
    print("[INFO] Opening serial port:",PORT)
    print("[INFO] Ready. Type commands to send to MCU.")
    print("Commands:")
    print("  ota main <hexfile>")
    print("  ota sub  <hexfile>")
    print("  (others are sent directly to MCU)")
    print("===============================")
    ser = SerialManager(PORT, BAUD, callback = on_serial_line)

    ser.open()
    ota = OTAUploader(ser)
    while True:
        try:
            cmd = input("> ").strip()
            if not cmd:
                continue

            parts = cmd.split()

            # 🔥 OTA는 사용자가 명시적으로 입력했을 때만
            if parts[0] == "ota" and len(parts) == 3:
                target = parts[1].upper()
                hexfile = parts[2]
                ota.upload_hex(hexfile, target)

            else:
                # 그 외 명령은 MCU로 그대로 전달
                ser.send_line(cmd)

        except KeyboardInterrupt:
            print("\n[INFO] Exiting...")
            ser.close()
            break

if __name__ == "__main__":
    main()
