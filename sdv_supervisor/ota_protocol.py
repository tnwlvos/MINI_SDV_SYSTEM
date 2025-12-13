import time
class OTAUploader:
    def __init__(self, serial_mgr):
        self.ser = serial_mgr
    def _send(self, line):
        self.ser.send_line(line)
        time.sleep(0.01)
    def upload_hex(self, hex_path, target):
        assert target in ("MAIN", "SUB")
        print(f"[OTA] Upload start -> {target}")
        self._send(f"OTA:BEGIN:{target}")
        with open(hex_path, "r") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                self._send(f"OTA:DATA:{line}")
        self._send("OTA:END")
        print(f"[OTA] Upload done -> {target}")    

def on_serial_line(line):
    if line.startswith("OTA:"):
        print("[OTA]", line)
    elif line.startswith("STATE:"):
        parse_state(line)
    else:
        print("[MCU]", line)