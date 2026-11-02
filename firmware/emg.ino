// ===== EMG: wrist flexes -> bits -> letters (5-bit code) =====
// Owner: ______   (only this person edits this tab)
//   A0 <- MyoWare LEFT  ENV  (+Vs = 5V)   left flex  = bit 0
//   A1 <- MyoWare RIGHT ENV  (+Vs = 5V)   right flex = bit 1
//   5 bits = one letter (sent automatically); both together = erase bits so far

const int PIN_EMG_L = A0;
const int PIN_EMG_R = A1;

// ---------------- EMG tuning ----------------
const float  EMG_SMOOTH           = 0.2;   // extra low-pass on the MyoWare ENV signal (0..1)
const float  THRESH_FRACTION      = 0.30;  // active threshold = rest + 30% of (max - rest)
const float  HYSTERESIS_FRACTION  = 0.6;   // release threshold = 60% of active threshold
const unsigned long MIN_HOLD_MS   = 150;   // flex must be held this long to count
const unsigned long BOTH_WINDOW_MS = 200;  // both arms within this window = commit
const unsigned long REFRACTORY_MS = 300;   // ignore input right after a bit
const int    BITS_PER_CHAR        = 5;     // 5-bit code: 32 symbols (see docs/code_table.md)
const bool   AUTO_COMMIT          = true;  // true: letter sends itself after the 5th bit,
                                           //       both arms = erase the bits typed so far
                                           // false: both arms = commit (old behaviour)

// ---------------- EMG state ----------------
float envL = 0, envR = 0;
float restL = 0, restR = 0;
float thrL = 200, thrR = 200;   // Edit during calibration

bool activeL = false, activeR = false;
unsigned long startL = 0, startR = 0;
bool countedL = false, countedR = false;
unsigned long lastBitMs = 0;

int  bitCount = 0;
byte code = 0;


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

// ---------------- EMG -> bits -> letters (5-bit code) ----------------
// Code = letter's position in the alphabet, in binary:  A = 00001 ... Z = 11010
//   11011 = space      11100 = .  (speaks the sentence)    11101 = ?
//   11110 = delete last letter      11111 = HELP (manual call for help)
//   00000 = cancel alarm (false alarm)
void sendSymbol(byte c) {
  if (c >= 1 && c <= 26) { Serial.print(F("CHAR ")); Serial.println((char)('A' + c - 1)); }
  else if (c == 27) Serial.println(F("CHAR  "));      // space
  else if (c == 28) Serial.println(F("CHAR ."));
  else if (c == 29) Serial.println(F("CHAR ?"));
  else if (c == 30) Serial.println(F("DEL"));
  else if (c == 31) Serial.println(F("HELP"));
  else              Serial.println(F("CANCEL"));      // 00000
}

void clearBits() {
  code = 0; bitCount = 0;
  Serial.println(F("CLEAR"));
}

void commitChar() {
  lastBitMs = millis();
  if (bitCount == BITS_PER_CHAR) {
    sendSymbol(code);
    code = 0; bitCount = 0;
  } else {
    clearBits();
  }
}

void addBit(int b) {
  code = (code << 1) | b;
  bitCount++;
  lastBitMs = millis();
  Serial.print(F("BIT ")); Serial.print(b); Serial.print(' ');
  for (int i = bitCount - 1; i >= 0; i--) Serial.print((code >> i) & 1);
  Serial.println();
  if (AUTO_COMMIT && bitCount == BITS_PER_CHAR) commitChar();
  else if (bitCount > BITS_PER_CHAR) clearBits();   // too many bits: start over
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

  // both arms flexed close together -> erase current bits (or commit, if AUTO_COMMIT is off)
  if (activeL && activeR && !countedL && !countedR &&
      (unsigned long)abs((long)startL - (long)startR) <= BOTH_WINDOW_MS &&
      now - max(startL, startR) >= MIN_HOLD_MS) {
    countedL = countedR = true;
    lastBitMs = now;
    if (AUTO_COMMIT) clearBits(); else commitChar();
    return;
  }
  // single arm -> bit (wait out the both-arms window first)
  if (heldL && !activeR && now - startL >= BOTH_WINDOW_MS) { countedL = true; addBit(0); }
  else if (heldR && !activeL && now - startR >= BOTH_WINDOW_MS) { countedR = true; addBit(1); }
}
