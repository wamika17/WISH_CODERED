// ------------------- PIN DEFINITIONS -------------------
#define IR_ENTRY 13   // Sensor A
#define IR_EXIT  14   // Sensor B
#define IR_C     16   // Sensor C (cash counter)
#define FSR_PIN  15   // FSR

// ------------------- A & B COUNTS -------------------
int entryCount = 0;
int exitCount  = 0;
int queueCount = 0;

int lastEntryState = HIGH;
int lastExitState  = HIGH;

// ------------------- SENSOR C COUNTS -------------------
#define TRIGGERED HIGH

const unsigned long DEBOUNCE_MS = 50;
const unsigned long REARM_MS    = 300;

int sensorCCount = 0;

int lastRawC = LOW;
int stableStateC = LOW;
unsigned long lastChangeTimeC = 0;

bool armedC = true;
unsigned long lastLowStableAtC = 0;

// ------------------- FSR -------------------
int vacantSeats = 1;   // 1 = vacant, 0 = occupied

// ------------------- CSV TIMING -------------------
unsigned long lastCsvTime = 0;
const unsigned long CSV_INTERVAL = 1000; // 1 second

// ------------------- SETUP -------------------
void setup() {
  Serial.begin(115200);

  pinMode(IR_ENTRY, INPUT_PULLUP);
  pinMode(IR_EXIT,  INPUT_PULLUP);

  pinMode(IR_C, INPUT_PULLDOWN);

  pinMode(FSR_PIN, INPUT_PULLUP);
}

// ------------------- LOOP -------------------
void loop() {

  // ---------- SENSOR A (ENTRY) ----------
  int entryState = digitalRead(IR_ENTRY);
  if (lastEntryState == HIGH && entryState == LOW) {
    entryCount++;
    updateQueue();
    delay(200);
  }
  lastEntryState = entryState;

  // ---------- SENSOR B (EXIT) ----------
  int exitState = digitalRead(IR_EXIT);
  if (lastExitState == HIGH && exitState == LOW) {
    if (exitCount < entryCount) {
      exitCount++;
    }
    updateQueue();
    delay(200);
  }
  lastExitState = exitState;

  // ---------- SENSOR C ----------
  unsigned long now = millis();
  int rawC = digitalRead(IR_C);

  if (rawC != lastRawC) {
    lastChangeTimeC = now;
    lastRawC = rawC;
  } 
  else if (now - lastChangeTimeC >= DEBOUNCE_MS && stableStateC != rawC) {
    stableStateC = rawC;

    if (stableStateC == TRIGGERED) {
      if (armedC) {
        sensorCCount++;
        armedC = false;
      }
    } else {
      lastLowStableAtC = now;
    }
  }

  if (!armedC && stableStateC != TRIGGERED) {
    if (now - lastLowStableAtC >= REARM_MS) {
      armedC = true;
    }
  }

  // ---------- FSR (VACANT SEATS) ----------
  int fsrVal = analogRead(FSR_PIN);

  if (fsrVal < 3000) {
    vacantSeats = 0;  // occupied
  } else {
    vacantSeats = 1;  // vacant
  }

  // ---------- CSV OUTPUT ----------
  if (millis() - lastCsvTime >= CSV_INTERVAL) {
    lastCsvTime = millis();
    printCSV();
  }
}

// ------------------- HELPERS -------------------
void updateQueue() {
  queueCount = entryCount - exitCount;
}

void printCSV() {
  float renegingPercent = 0.0;

  if (entryCount > 0) {
    renegingPercent = ((float)(entryCount - sensorCCount) * 100.0) 
                      / (float)entryCount;
  }

  Serial.print(entryCount);        // A
  Serial.print(",");
  Serial.print(exitCount);         // B
  Serial.print(",");
  Serial.print(sensorCCount);      // C
  Serial.print(",");
  Serial.print(queueCount);        // A - B
  Serial.print(",");
  Serial.print(renegingPercent, 2); // (A-C)*100/A
  Serial.print(",");
  Serial.println(vacantSeats);     // F
}


