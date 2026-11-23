const int IR_PIN  = 27;
const int LED_PIN = 2;

const bool ACTIVE_LOW = true;          // most IR modules: LOW = object detected
const unsigned long MIN_DETECT_MS = 40;   // object must be seen this long to count (noise filter)
const unsigned long CLEAR_MS      = 300;  // gap needed between vehicles / after a vehicle passes

enum State { IDLE, DETECTING, COUNTED_WAIT_CLEAR };
State state = IDLE;

unsigned long stateStart = 0;
unsigned long lastSeen = 0;
unsigned long vehicleCount = 0;

bool objectPresent() {
  int v = digitalRead(IR_PIN);
  return ACTIVE_LOW ? (v == LOW) : (v == HIGH);
}

void setup() {
  Serial.begin(115200);
  pinMode(IR_PIN, INPUT);          // use INPUT_PULLUP if your module's output is open-collector
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Vehicle counter ready");
}

void loop() {
  unsigned long now = millis();
  bool present = objectPresent();
  digitalWrite(LED_PIN, present ? HIGH : LOW);

  switch (state) {
    case IDLE:
      if (present) {
        state = DETECTING;
        stateStart = now;
      }
      break;

    case DETECTING:
      if (!present) {
        state = IDLE;                       // too short, treat as noise
      } else if (now - stateStart >= MIN_DETECT_MS) {
        vehicleCount++;
        Serial.print("Vehicle counted. Total = ");
        Serial.println(vehicleCount);
        state = COUNTED_WAIT_CLEAR;
        lastSeen = now;
      }
      break;

    case COUNTED_WAIT_CLEAR:
      if (present) {
        lastSeen = now;                     // still blocked (vehicle still passing)
      } else if (now - lastSeen >= CLEAR_MS) {
        state = IDLE;                       // clear long enough, ready for next vehicle
      }
      break;
  }

  delay(2);
}
