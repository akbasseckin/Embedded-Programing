const int p_meter = 32;
const int Red = 21;
const int Green = 22;
const int Blue = 23;
int sensorValue = 0;

const int startSOS = 35;
const int endSOS = 34;
int startButtonState = 0;
int endButtonState = 0;

bool sosActive = false;

const char* sosChar = "...---...";
const int sosLength = 9;

const unsigned long dotDuration = 200;
const unsigned long dashDuration = dotDuration * 3;
const unsigned long symbolGap = dotDuration;
const unsigned long letterGap = dotDuration * 2;

// --- SOS state machine variables ---
enum SosPhase { SIGNAL_ON, SIGNAL_OFF };
SosPhase sosPhase = SIGNAL_ON;
int sosIndex = 0;
unsigned long sosPhaseStart = 0;

void setup() {
  Serial.begin(115200);

  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);

  pinMode(startSOS, INPUT);
  pinMode(endSOS, INPUT);
}

void loop() {
  startButtonState = digitalRead(startSOS);
  endButtonState = digitalRead(endSOS);

  if (startButtonState == HIGH && !sosActive) {
    sosActive = true;
    sosIndex = 0;
    sosPhase = SIGNAL_ON;
    sosPhaseStart = millis();
    allOff();
  }
  if (endButtonState == HIGH) {
    sosActive = false;
    allOff();
  }

  sensorValue = analogRead(p_meter);

  if (sosActive) {
    updateSOS();
  } else {
    setColorFromSensor();
  }
}

void setColorFromSensor() {
  if (sensorValue <= 1000) {
    digitalWrite(Blue, HIGH);
    digitalWrite(Green, LOW);
    digitalWrite(Red, LOW);
  } else if (sensorValue <= 2500) {
    digitalWrite(Green, HIGH);
    digitalWrite(Red, LOW);
    digitalWrite(Blue, LOW);
  } else {
    digitalWrite(Red, HIGH);
    digitalWrite(Green, LOW);
    digitalWrite(Blue, LOW);
  }
}

void allOff() {
  digitalWrite(Red, LOW);
  digitalWrite(Green, LOW);
  digitalWrite(Blue, LOW);
}

void updateSOS() {
  unsigned long now = millis();
  char symbol = sosChar[sosIndex];

  if (sosPhase == SIGNAL_ON) {
    setColorFromSensor(); // color follows pot live, even mid-signal
    unsigned long onTime = (symbol == '.') ? dotDuration : dashDuration;

    if (now - sosPhaseStart >= onTime) {
      allOff();
      sosPhase = SIGNAL_OFF;
      sosPhaseStart = now;
    }

  } else { // SIGNAL_OFF
    unsigned long offTime = symbolGap;
    if (sosIndex == 2 || sosIndex == 5) {
      offTime += letterGap; // extra pause between S / O / S
    }

    if (now - sosPhaseStart >= offTime) {
      sosIndex++;
      if (sosIndex >= sosLength) {
        sosIndex = 0; // loop the SOS pattern; remove this line to run once
      }
      sosPhase = SIGNAL_ON;
      sosPhaseStart = now;
    }
  }
}