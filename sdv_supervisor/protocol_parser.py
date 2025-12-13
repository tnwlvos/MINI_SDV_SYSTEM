
def parse_message(line: str):
    if line.startswith("OTA:"):
        print("[OTA]", line)
    elif line.startswith("STATE:"):
        parse_state(line)
    else:
        print("[MCU]", line)
def parse_state(line: str):
    try:
        payload = line.replace("STATE:", "").strip()
        parts = payload.split(";")

        state={}
        for p in parts:
            if "=" in p:
                key, value = p.split("=")
                state[key] = value
        print(f"[STATE] ULTRA={state.get('ULTRA')} | MODE={state.get('MODE')} | MOTOR={state.get('MOTOR')} | Speed={state.get('Speed')} | FCW={state.get('FCW')} | TTC={state.get('TTC')}")

    except Exception as e:
        print(f"[ERROR] Failed to parse state message: {e} | line",line)
        