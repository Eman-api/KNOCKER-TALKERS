// ===== ECG: heart rate -> lead-off check -> alarm =====
// Owner: ______   (only this person edits this tab)
//   A2 <- AD8232 OUTPUT  (module on 3.3V ONLY)
//   A3 <- AD8232 LO+  (digital, HIGH = electrode off)
//   A4 <- AD8232 LO-  (digital, HIGH = electrode off)

const int PIN_ECG  = A2;
const int PIN_LO_P = A3;
const int PIN_LO_N = A4;

// ---------------- ECG tuning ----------------
const int    HR_LOW_BPM    = 40;
const int    HR_HIGH_BPM   = 130;
const unsigned long ALARM_AFTER_MS = 10000;  // abnormal for 10 s -> ALARM
const unsigned long MIN_RR_MS = 300;         // ignore beats faster than 200 bpm (noise)

// ---------------- ECG state ----------------
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

void ecgSetup() {
  pinMode(PIN_LO_P, INPUT);
  pinMode(PIN_LO_N, INPUT);
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
