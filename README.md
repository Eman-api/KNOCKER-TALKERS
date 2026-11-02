# KNOCKER-TALKERS
KNOCK KNOCK TALK TALK

A wrist-flex communication aid. Two EMG sensors on the forearms turn deliberate wrist flexes into binary, the Arduino turns 7 bits into an ASCII letter, and the laptop speaks the finished sentence. An ECG channel watches heart rate and calls for help if it stays abnormal.

| Input | Meaning |
|---|---|
| Flex **left** wrist | bit `0` |
| Flex **right** wrist | bit `1` |
| Flex **both** together | commit the letter (7 bits) |
| Letter `.` `?` or `!` | speak the sentence |

Codes are in [`docs/ascii_cheatsheet.md`](docs/ascii_cheatsheet.md).

## Hardware

| Qty | Part | Notes |
|---|---|---|
| 1 | Arduino Uno (Elegoo Uno is fine) | USB to the laptop: power in, text out |
| 2 | MyoWare 2.0 muscle sensor | one per forearm, on the wrist **flexors** (palm side, upper third of the forearm) |
| 1 | AD8232 heart-monitor module | **3.3 V only**; comes with a 3-lead cable |
| 9 | Ag/AgCl snap electrodes | single-use: 3 per MyoWare + 3 for ECG |
| ~15 | jumper wires, USB cable | |

### Wiring

| Sensor pin | Uno pin |
|---|---|
| MyoWare LEFT +Vs / GND / ENV | 5V / GND / **A0** |
| MyoWare RIGHT +Vs / GND / ENV | 5V / GND / **A1** |
| AD8232 3.3V / GND / OUTPUT | **3.3V** / GND / **A2** |
| AD8232 LO+ / LO- | **A3** / **A4** |
| AD8232 SDN | leave unconnected |

ECG electrodes: RA (red) inside of right wrist, LA (yellow) inside of left wrist, RL (green) right forearm.

Full drawing: [`hardware/emg_lang_proc_uno_wiring.pdf`](hardware/emg_lang_proc_uno_wiring.pdf) (KiCad project in `hardware/`).

> **Safety:** run the laptop on battery (unplugged from the wall) whenever electrodes are on a person. There is no electrical isolation. The heart-rate alarm is a warning aid, not a medical device.

## Running it

1. **Firmware.** Open `firmware/firmware.ino` in Arduino IDE 2, pick **Board: Arduino Uno** and the right port, and upload.
2. **Laptop.**
   ```
   pip install -r laptop/requirements.txt
   python laptop/speak.py          # auto-detects the Uno
   ```
   Close the Arduino Serial Monitor first, since only one program can use the port.
3. **Calibrate.** On startup the Uno asks you to relax, then flex left, then flex right. Type `c` + Enter in `speak.py` to recalibrate anytime.

Thresholds and timings are constants at the top of `firmware.ino` (`THRESH_FRACTION`, `MIN_HOLD_MS`, `R_THRESH`, ...). Tune them on real signals.

## Repo layout

```
firmware/firmware.ino     Arduino code (folder name must match the .ino name)
laptop/speak.py           serial -> text-to-speech + alarm
hardware/                 KiCad schematic, wiring PDF, BOM
docs/                     cheat sheet, slides, diagrams
```

## Working together

1. **Pull** before you start, every time.
2. Make small commits with clear messages, e.g. `tune EMG threshold`.
3. **Push** when you're done.
4. Split work by file to avoid merge conflicts. **KiCad files can't be merged**, so only one person edits the schematic at a time.

Every push that touches `firmware/` is compile-checked for the Uno automatically (see the **Actions** tab). A red X means the code won't build: fix it before uploading.
