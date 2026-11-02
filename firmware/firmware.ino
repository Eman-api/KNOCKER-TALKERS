// KNOCKER-TALKERS firmware - Arduino Uno (Elegoo Uno works the same)
//
// Wrist-flex binary keyboard + heart-rate watchdog.
//   Left forearm flex  = bit 0
//   Right forearm flex = bit 1
//   Both at once       = commit the current letter
//   7 bits             = one ASCII character
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

// ---------------- pins ----------------
const int PIN_EMG_L = A0;
const int PIN_EMG_R = A1;
const int PIN_ECG   = A2;
const int PIN_LO_P  = A3;
const int PIN_LO_N  = A4;

// ---------------- EMG tuning ----------------
const unsigned long SAMPLE_US     = 2000;  // 500 Hz sampling
const float  EMG_SMOOTH           = 0.2;   // extra low-pass on the MyoWare ENV signal (0..1)
const float  THRESH_FRACTION      = 0.30;  // active threshold = rest + 30% of (max - rest)
const float  HYSTERESIS_FRACTION  = 0.6;   // release threshold = 60% of active threshold
const unsigned long MIN_HOLD_MS   = 150;   // flex must be held this long to count
const unsigned long BOTH_WINDOW_MS = 200;  // both arms within this window = commit
const unsigned long REFRACTORY_MS = 300;   // ignore input right after a bit
const int    BITS_PER_CHAR        = 7;

// ---------------- ECG tuning ----------------
const int    HR_LOW_BPM    = 40;
const int    HR_HIGH_BPM   = 130;
const unsigned long ALARM_AFTER_MS = 10000;  // abnormal for 10 s -> ALARM
const unsigned long MIN_RR_MS = 300;         // ignore beats faster than 200 bpm (noise)

// ---------------- state ----------------
float envL = 0, envR = 0;
float restL = 0, restR = 0;
float thrL = 200, thrR = 200;   // replaced by calibration

bool activeL = false, activeR = false;
unsigned long startL = 0, startR = 0;
bool countedL = false, countedR = false;
unsigned long lastBitMs = 0;

int  bitCount = 0;
byte code = 0;

// ECG
float ecgBaseline = 512;
bool  inBeat = false;
unsigned long lastBeatMs = 0;
float bpm = 0;
unsigned long abnormalSince = 0;
bool alarmSent = false;
unsigned long lastHrReport = 0;

// 60 Hz notch for the ECG channel (biquad, designed for 500 Hz sampling, Q = 10)
float nx1 = 0, nx2 = 0, ny1 = 0, ny2 = 0;
float notch60(float x) {
  // coefficients precomputed: f0 = 60 Hz, fs = 500 Hz, Q = 10
  const float b0 = 0.96691f, b1 = -1.40969f, b2 = 0.96691f;
  const float a1 = -1.40969f, a2 = 0.93381f;
  float y = b0 * x + b1 * nx1 + b2 * nx2 - a1 * ny1 - a2 * ny2;
  nx2 = nx1; nx1 = x; ny2 = ny1; ny1 = y;
  return y;
}

// ---------------- calibration ----------------
float averageFor(int pin, unsigned long ms) {
  unsigned long t0 = millis();
  float sum = 0; long n = 0;
  while (millis() - t0 < ms) { sum += analogRead(pin); n++; delay(2); }
  return n ? sum / n : 0;
}

float peakFor(int pin, unsigned long ms) {
  unsigned long t0 = millis();
  float peak = 0;
  while (millis() - t0 < ms) { peak = max(peak, (float)analogRead(pin)); delay(2); }
  return peak;
}

void calibrate() {
  Serial.println(F("MSG Relax both arms..."));
  delay(1500);
  restL = averageFor(PIN_EMG_L, 2000);
  restR = averageFor(PIN_EMG_R, 2000);

  Serial.println(F("MSG Flex LEFT wrist hard..."));
  delay(1000);
  float maxL = peakFor(PIN_EMG_L, 2000);
  Serial.println(F("MSG Relax. Now flex RIGHT wrist hard..."));
  delay(1500);
  float maxR = peakFor(PIN_EMG_R, 2000);

  thrL = restL + THRESH_FRACTION * max(maxL - restL, 20.0f);
  thrR = restR + THRESH_FRACTION * max(maxR - restR, 20.0f);
  envL = restL; envR = restR;

  Serial.print(F("CAL ")); Serial.print(restL); Serial.print(' ');
  Serial.print(restR); Serial.print(' '); Serial.print(thrL); Serial.print(' ');
  Serial.println(thrR);
  Serial.println(F("MSG Ready."));
}

// ---------------- EMG -> bits -> ASCII ----------------
void addBit(int b) {
  code = (code << 1) | b;
  bitCount++;
  lastBitMs = millis();
  Serial.print(F("BIT ")); Serial.print(b); Serial.print(' ');
  for (int i = bitCount - 1; i >= 0; i--) Serial.print((code >> i) & 1);
  Serial.println();
  if (bitCount > BITS_PER_CHAR) {        // too many bits: discard and start over
    code = 0; bitCount = 0;
    Serial.println(F("CLEAR"));
  }
}

void commitChar() {
  lastBitMs = millis();
  if (bitCount == BITS_PER_CHAR && code >= 32 && code < 127) {
    Serial.print(F("CHAR ")); Serial.println((char)code);
  } else {
    Serial.println(F("CLEAR"));
  }
  code = 0; bitCount = 0;
}

void updateEmg(unsigned long now) {
  envL += EMG_SMOOTH * (analogRead(PIN_EMG_L) - envL);
  envR += EMG_SMOOTH * (analogRead(PIN_EMG_R) - envR);

  // hysteresis: turn on above thr, off below rest + 60% of (thr - rest)
  bool nowL = activeL ? envL > restL + HYSTERESIS_FRACTION * (thrL - restL) : envL > thrL;
  bool nowR = activeR ? envR > restR + HYSTERESIS_FRACTION * (thrR - restR) : envR > thrR;

  if (nowL && !activeL) { startL = now; countedL = false; }
  if (nowR && !activeR) { startR = now; countedR = false; }
  activeL = nowL; activeR = nowR;

  if (now - lastBitMs < REFRACTORY_MS) return;

  bool heldL = activeL && !countedL && now - startL >= MIN_HOLD_MS;
  bool heldR = activeR && !countedR && now - startR >= MIN_HOLD_MS;

  // both arms flexed close together -> commit
  if (activeL && activeR && !countedL && !countedR &&
      (unsigned long)abs((long)startL - (long)startR) <= BOTH_WINDOW_MS &&
      now - max(startL, startR) >= MIN_HOLD_MS) {
    countedL = countedR = true;
    commitChar();
    return;
  }
  // single arm -> bit (wait out the both-arms window first)
  if (heldL && !activeR && now - startL >= BOTH_WINDOW_MS) { countedL = true; addBit(0); }
  else if (heldR && !activeL && now - startR >= BOTH_WINDOW_MS) { countedR = true; addBit(1); }
}

// ---------------- ECG -> heart rate -> alarm ----------------
void updateEcg(unsigned long now) {
  bool leadOff = digitalRead(PIN_LO_P) || digitalRead(PIN_LO_N);
  if (leadOff) {
    abnormalSince = 0;               // a loose electrode is a fault, not an emergency
    if (now - lastHrReport > 1000) { Serial.println(F("LEADOFF")); lastHrReport = now; }
    return;
  }

  float x = notch60(analogRead(PIN_ECG));
  ecgBaseline += 0.005f * (x - ecgBaseline);   // slow-moving baseline
  float v = x - ecgBaseline;

  // simple R-peak detector: rising past a fixed offset above baseline
  const float R_THRESH = 120;  // ADC counts; tune on real signals
  if (!inBeat && v > R_THRESH && now - lastBeatMs > MIN_RR_MS) {
    inBeat = true;
    if (lastBeatMs) {
      float inst = 60000.0f / (now - lastBeatMs);
      bpm = bpm == 0 ? inst : 0.8f * bpm + 0.2f * inst;
    }
    lastBeatMs = now;
  } else if (inBeat && v < R_THRESH * 0.5f) {
    inBeat = false;
  }
  if (lastBeatMs && now - lastBeatMs > 3000) bpm = 0;   // no beat for 3 s

  bool abnormal = bpm > 0 && (bpm < HR_LOW_BPM || bpm > HR_HIGH_BPM);
  if (abnormal) {
    if (!abnormalSince) abnormalSince = now;
    if (!alarmSent && now - abnormalSince >= ALARM_AFTER_MS) {
      Serial.println(F("ALARM"));
      alarmSent = true;
    }
  } else {
    abnormalSince = 0;
    alarmSent = false;
  }

  if (now - lastHrReport > 1000) {
    Serial.print(F("HR ")); Serial.println((int)bpm);
    lastHrReport = now;
  }
}

// ---------------- main ----------------
void setup() {
  Serial.begin(115200);
  pinMode(PIN_LO_P, INPUT);
  pinMode(PIN_LO_N, INPUT);
  calibrate();
}

void loop() {
  static unsigned long nextSample = micros();
  if ((long)(micros() - nextSample) < 0) return;
  nextSample += SAMPLE_US;

  unsigned long now = millis();
  updateEmg(now);
  updateEcg(now);

  // send 'c' from the laptop to recalibrate
  if (Serial.available() && Serial.read() == 'c') calibrate();
}
