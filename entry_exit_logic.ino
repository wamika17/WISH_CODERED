// Debug + reliable entry/exit detection for two active-HIGH IR sensors
// Sensor A -> GPIO13
// Sensor B -> GPIO16

#define IR_A 13
#define IR_B 14

#define TRIGGERED HIGH
#define NOT_TRIGGERED LOW

// debounce / timing (ms)
const unsigned long DEBOUNCE_MS = 50;      // ignore very fast jitter
const unsigned long SEQUENCE_TIMEOUT = 1500; // max time to complete A->B or B->A

// state machine
// 0 = idle
// 1 = A triggered first (waiting for B)
// 2 = B triggered first (waiting for A)
int state = 0;

// remember last stable reading to detect edges
int lastA = NOT_TRIGGERED;
int lastB = NOT_TRIGGERED;

// when the first sensor triggered (ms)
unsigned long firstTriggerTime = 0;

// simple debounce timestamps
unsigned long lastAChange = 0;
unsigned long lastBChange = 0;

void setup() {
  Serial.begin(115200);
  // ensure pins are not floating
  pinMode(IR_A, INPUT_PULLDOWN);
  pinMode(IR_B, INPUT_PULLDOWN);

  Serial.println();
  Serial.println("Entry/Exit detector (active-HIGH). Debug prints ON.");
  Serial.println("A -> GPIO13   B -> GPIO16");
}

void loop() {
  unsigned long now = millis();

  // read raw values
  int rawA = digitalRead(IR_A);
  int rawB = digitalRead(IR_B);

  // ---- debounce A ----
  if (rawA != lastA) {
    // value changed — check debounce
    if (now - lastAChange >= DEBOUNCE_MS) {
      lastAChange = now;
      lastA = rawA;
      // Serial.println("A debounce update");
    }
  }

  // ---- debounce B ----
  if (rawB != lastB) {
    if (now - lastBChange >= DEBOUNCE_MS) {
      lastBChange = now;
      lastB = rawB;
      // Serial.println("B debounce update");
    }
  }

  // For debugging: print raw values every 300 ms
  static unsigned long lastDebug = 0;
  if (now - lastDebug >= 300) {
    Serial.print("RAW A=");
    Serial.print(lastA);
    Serial.print("  B=");
    Serial.print(lastB);
    Serial.print("  STATE=");
    Serial.println(state);
    lastDebug = now;
  }

  // Edge detection: we act only when a sensor goes from NOT_TRIGGERED -> TRIGGERED
  // (this prevents repeated prints while sensor remains HIGH)
  static int prevA = NOT_TRIGGERED;
  static int prevB = NOT_TRIGGERED;

  bool A_rising = (prevA == NOT_TRIGGERED && lastA == TRIGGERED);
  bool B_rising = (prevB == NOT_TRIGGERED && lastB == TRIGGERED);

  prevA = lastA;
  prevB = lastB;

  // state machine
  if (state == 0) {
    if (A_rising) {
      state = 1;
      firstTriggerTime = now;
      Serial.println("A first -> possible ENTRY (A_rising)");
    } else if (B_rising) {
      state = 2;
      firstTriggerTime = now;
      Serial.println("B first -> possible EXIT (B_rising)");
    }
  } 
  else if (state == 1) { // A triggered first, waiting for B
    // timeout
    if (now - firstTriggerTime > SEQUENCE_TIMEOUT) {
      Serial.println("Timeout waiting for B -> Reset to idle");
      state = 0;
    } else if (B_rising) {
      Serial.println("ENTRY detected! (A then B)");
      state = 0;
      // small cooldown so you don't double count
      delay(300);
    }
  } 
  else if (state == 2) { // B triggered first, waiting for A
    if (now - firstTriggerTime > SEQUENCE_TIMEOUT) {
      Serial.println("Timeout waiting for A -> Reset to idle");
      state = 0;
    } else if (A_rising) {
      Serial.println("EXIT detected! (B then A)");
      state = 0;
      delay(300);
    }
  }

  delay(10); // tiny delay for loop stability
}
