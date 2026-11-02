// KNOCKER-TALKERS firmware - Arduino Uno (Elegoo Uno works the same)
//
// Wrist-flex binary keyboard + heart-rate watchdog.
//   Left forearm flex  = bit 0
//   Right forearm flex = bit 1
//   Both at once       = commit the current letter
//   5 bits             = one ASCII character
// Sends text and status lines to the laptop over USB serial (115200 baud).
// The laptop script (laptop/speak.py) speaks the text and plays the alarm.
//
// Wiring (see hardware/emg_lang_proc_uno_wiring.pdf):
//   A0 <- MyoWare LEFT  ENV      (+Vs = 5V)
//   A1 <- MyoWare RIGHT ENV      (+Vs = 5V)
//   A2 <- AD8232 OUTPUT          (3.3V ONLY)
//   A3 <- AD8232 LO+  (digital)
//   A4 <- AD8232 LO-  (digital)
//
// Serial protocol (one line per message):
//   BIT <0|1> <bits so far>   a bit was registered
//   CHAR <c>                  a letter was committed
//   CLEAR                     current bits were discarded (bad code)
//   HR <bpm>                  heart rate, about once per second
//   LEADOFF                   an ECG electrode is off (fault, not an emergency)
//   ALARM                     sustained abnormal heart rate -> laptop calls for help
//   CAL <restL> <restR> <thrL> <thrR>   calibration result

// The sketch is split into tabs so two people can work at the same time:
//   firmware.ino  - setup() and loop(): sampling timer + serial commands (rarely changes)
//   emg.ino       - EMG: calibration, wrist flex -> bits -> ASCII letters   (EMG owner)
//   ecg.ino       - ECG: heart rate, lead-off, alarm                        (ECG owner)
// Arduino IDE joins all .ino files in this folder into one program; each shows as a tab.

const unsigned long SAMPLE_US = 2000;  // 500 Hz sampling for both EMG and ECG

void setup() {
  Serial.begin(115200);
  ecgSetup();
  calibrate();   // in emg.ino
}

void loop() {
  static unsigned long nextSample = micros();
  if ((long)(micros() - nextSample) < 0) return;
  nextSample += SAMPLE_US;

  unsigned long now = millis();
  updateEmg(now);   // emg.ino
  updateEcg(now);   // ecg.ino

  // send 'c' from the laptop to recalibrate
  if (Serial.available() && Serial.read() == 'c') calibrate();
}
