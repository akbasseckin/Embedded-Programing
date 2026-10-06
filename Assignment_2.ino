// ---- Pins ----
const int CAR_GREEN  = 25;
const int CAR_YELLOW = 26;
const int CAR_RED    = 27;
const int PED_BLUE   = 32;
const int BUTTON_PIN = 4;

// ---- Timing (in 50 ms ticks) ----
const uint32_t TICK_US       = 50000;  // timer fires every 50 ms
const uint32_t YELLOW_TICKS  = 10;     // 0.5 s
const uint32_t PED_TICKS     = 80;     // 4 s

// ---- States ----
const int CAR_GREEN_STATE  = 0;
const int CAR_YELLOW_STATE = 1;
const int PED_WALK_STATE   = 2;
int state = CAR_GREEN_STATE;

// ---- Shared with ISRs ----
volatile bool     buttonPressed = false;
volatile uint32_t tickCount     = 0;

hw_timer_t *timer = NULL;

// Hardware interrupt: button
void IRAM_ATTR onButton() {
  buttonPressed = true;
}

// Timer interrupt: count ticks
void IRAM_ATTR onTimer() {
  tickCount++;
}

void setLights(bool green, bool yellow, bool red, bool blue) {
  digitalWrite(CAR_GREEN,  green);
  digitalWrite(CAR_YELLOW, yellow);
  digitalWrite(CAR_RED,    red);
  digitalWrite(PED_BLUE,   blue);
}

void resetTicks() {
  noInterrupts();
  tickCount = 0;
  interrupts();
}

uint32_t getTicks() {
  noInterrupts();
  uint32_t t = tickCount;
  interrupts();
  return t;
}

void setup() {
  pinMode(CAR_GREEN,  OUTPUT);
  pinMode(CAR_YELLOW, OUTPUT);
  pinMode(CAR_RED,    OUTPUT);
  pinMode(PED_BLUE,   OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButton, FALLING);

  // Timer: 1 MHz base, alarm every TICK_US 
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, TICK_US, true, 0);

  setLights(HIGH, LOW, LOW, LOW);  // car green, pedestrian stop
}

void loop() {
  switch (state) {

    case CAR_GREEN_STATE:
      if (buttonPressed) {
        buttonPressed = false;
        resetTicks();
        setLights(LOW, HIGH, LOW, LOW);  // car yellow, pedestrian stop
        state = CAR_YELLOW_STATE;
      }
      break;

    case CAR_YELLOW_STATE:
      if (getTicks() >= YELLOW_TICKS) {
        resetTicks();
        setLights(LOW, LOW, HIGH, HIGH);  // car red, pedestrian walk (blue ON)
        state = PED_WALK_STATE;
      }
      break;

    case PED_WALK_STATE:
      if (getTicks() >= PED_TICKS) {
        buttonPressed = false;            // ignore presses during sequence
        setLights(HIGH, LOW, LOW, LOW);   // car green, pedestrian stop (blue OFF)
        state = CAR_GREEN_STATE;
      }
      break;
  }
}