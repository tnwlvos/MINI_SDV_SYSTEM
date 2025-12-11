import serial
import threading

class SerialManager:
    def __init__(self, port, baud, callback):
        self.port = port
        self.baud = baud
        self.callback = callback
        self.serial= None
        self.running = False

    def open(self):
        try:
            self.serial = serial.Serial(self.port, self.baud, timeout=0.1)
            self.running = True
            thread = threading.Thread(target=self.read_loop,daemon=True)
            thread.start()
        except serial.SerialException as e:
            print(f"Error opening serial port: {e}")
    def read_loop(self):
        while self.running:
            try:
                raw = self.serial.readline()   # ★ raw bytes 받기

                if raw:
                    print("[RAW]", raw)        # ★ 디버깅용 출력

                try:
                    line = raw.decode(errors="ignore").strip()
                    if line:
                        if self.callback:
                            self.callback(line)
                    # else:
                    #     print("[MCU] (empty line)")
                except Exception as e:
                    print("[DECODE ERROR]", raw, e)
            except serial.SerialException as e:
                print(f"[ERROR] Serial read error: {e}")
                self.running = False
                break

    def send_line(self, msg: str):
        if self.serial:
            self.serial.write((msg + "\n").encode())

    def close(self):
        self.running = False
        if self.serial:
            self.serial.close()
            print("[INFO] Serial port closed.")
