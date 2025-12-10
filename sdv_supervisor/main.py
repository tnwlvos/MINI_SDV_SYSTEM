from serial_manager import SerialManager
from protocol_parser import parse_message

PORT = "/dev/ttyUSB0"
BAUD = 38400

def on_serial_line(line: str):
    parse_message(line)

def main():
    print("=== SDV Supervisor Started ===")
    print("[INFO] Opening serial port:",PORT)

    ser = SerialManager(PORT, BAUD, callback = on_serial_line)

    ser.open()
    print("[INFO] Ready. Type commands to send to MCU.")
    print("Examples:")
    print(" OTA: START")
    print(" SET: CAUTION=70")
    print("===============================")

    while True:
        try:
            cmd = input("> ").strip()
            if cmd:
                ser.send_line(cmd) 
        except KeyboardInterrupt:
            print("\n[INFO] Exiting...")
            ser.close()
            break

if __name__ == "__main__":
    main()
