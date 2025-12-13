import time
class OTAUploader:
    def __init__(self, serial_mgr):
        self.ser = serial_mgr
    def _wait_ack(self, prefix="OTA:ACK", timeout=2.0):
        t0 = time.time()
        while time.time() - t0 < timeout:
            try:
                line = self.ser.rxq.get(timeout=0.2)
            except Exception:
                continue
            if line.startswith(prefix):
                return line
            if line.startswith("OTA:NAK"):
                raise RuntimeError(line)
        raise TimeoutError(f"ACK timeout ({prefix})")
    def _send(self, line):
        self.ser.send_line(line)
        time.sleep(0.01)
    def upload_hex(self, hex_path, target):
        assert target in ("MAIN", "SUB")
        print(f"[OTA] Upload start -> {target}")

        self.ser.send_line(f"OTA:BEGIN:{target}")
        self._wait_ack(prefix="OTA:ACK:BEGIN")
        with open(hex_path, "r") as f:
            for raw in f:
                line = raw.strip()
                if not line:
                    continue
                # BEGIN 이후엔 IHEX 원문만 전송
                if line[0] != ':':
                    continue
                self._send(line)

        self._send("OTA:END") 

def on_serial_line(line):
    if line.startswith("OTA:"):
        print("[OTA]", line)
    elif line.startswith("STATE:"):
        parse_state(line)
    else:
        print("[MCU]", line)