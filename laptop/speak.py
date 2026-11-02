"""KNOCKER-TALKERS laptop side.

Reads the Arduino's serial lines, builds words, speaks them, and plays the alarm.

Setup:   pip install -r laptop/requirements.txt
Run:     python laptop/speak.py            (auto-detects the Uno)
         python laptop/speak.py COM5       (Windows, explicit port)
         python laptop/speak.py /dev/ttyACM0

Text rules (5-bit code, see docs/code_table.md):
  CHAR <c>   adds a letter or space; '.' or '?' speaks the sentence
  DEL        deletes the last letter
  HELP       user asked for help -> speak a call for help
  CANCEL     user cancelled a false heart-rate alarm
Keys while running:  c = recalibrate   q = quit   (type then press Enter)
"""
import sys
import threading
import time

import pyttsx3
import serial
from serial.tools import list_ports

BAUD = 115200


def find_port():
    for p in list_ports.comports():
        desc = f"{p.description} {p.manufacturer or ''}".lower()
        if any(k in desc for k in ("arduino", "ch340", "usb serial", "usb-serial", "wch")):
            return p.device
    ports = list_ports.comports()
    return ports[0].device if ports else None


class Speaker:
    def __init__(self):
        self.engine = pyttsx3.init()
        self.engine.setProperty("rate", 160)
        self.lock = threading.Lock()

    def say(self, text):
        print(f"\n>>> SPEAKING: {text}")
        with self.lock:
            self.engine.say(text)
            self.engine.runAndWait()


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else find_port()
    if not port:
        sys.exit("No serial port found. Plug in the Uno or pass the port, e.g. python speak.py COM5")
    print(f"Connecting to {port} at {BAUD} baud ...")
    ser = serial.Serial(port, BAUD, timeout=0.1)
    time.sleep(2)  # Uno resets when the port opens

    spk = Speaker()
    sentence = ""
    last_alarm = 0.0
    alarm_muted_until = 0.0
    running = True

    def keyboard():
        nonlocal running
        for line in sys.stdin:
            cmd = line.strip().lower()
            if cmd == "c":
                ser.write(b"c")
            elif cmd == "q":
                running = False
                break

    threading.Thread(target=keyboard, daemon=True).start()

    while running:
        raw = ser.readline()
        if not raw:
            continue
        line = raw.decode(errors="ignore").strip()
        if not line:
            continue
        kind, _, rest = line.partition(" ")

        if kind == "MSG":
            print(rest)
        elif kind == "BIT":
            print(f"bit {rest}", end="\r")
        elif kind == "CLEAR":
            print("\n(code discarded - start the letter again)")
        elif kind == "CHAR":
            ch = rest[:1] if rest else " "
            if ch in ".?!":
                sentence += ch
                spk.say(sentence.strip())
                sentence = ""
            else:
                sentence += ch
                print(f"\ntext: {sentence}")
        elif kind == "DEL":
            sentence = sentence[:-1]
            print(f"\ntext: {sentence}")
        elif kind == "HELP":
            print("\n!!! USER ASKED FOR HELP !!!")
            spk.say("Help. I need help.")
        elif kind == "CANCEL":
            alarm_muted_until = time.time() + 60
            print("\nalarm cancelled by user (muted for 60 s)")
            spk.say("False alarm. I am okay.")
        elif kind == "HR":
            print(f"heart rate: {rest} bpm   ", end="\r")
        elif kind == "LEADOFF":
            print("ECG electrode off - check the leads   ", end="\r")
        elif kind == "ALARM":
            if time.time() > alarm_muted_until and time.time() - last_alarm > 15:
                last_alarm = time.time()
                print("\n!!! HEART RATE ALARM !!!")
                spk.say("Help. I need help. My heart rate is abnormal.")
        elif kind == "CAL":
            print(f"calibration: {rest}")

    ser.close()


if __name__ == "__main__":
    main()
