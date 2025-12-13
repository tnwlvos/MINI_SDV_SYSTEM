import serial
import threading
import queue

class SerialManager:
    def __init__(self, port, baud, callback):
        self.port = port
        self.baud = baud
        self.callback = callback
        self.serial= None
        self.running = False
        self.rxq = queue.Queue()   # ★ 추가

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
            raw = self.serial.readline()
            if not raw:
                continue
            line = raw.decode(errors="ignore").strip()
            if not line:
                continue

            # 큐에 넣기 (OTA가 기다릴 수 있게)
            self.rxq.put(line)

            # 기존 콜백 유지
            if self.callback:
                self.callback(line)

    def send_line(self, msg: str):
        if self.serial:
            self.serial.write((msg + "\n").encode())

    def close(self):
        self.running = False
        if self.serial:
            self.serial.close()
            print("[INFO] Serial port closed.")
