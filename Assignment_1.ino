//--------------------------------------------------------------------
const int p_meter = 32;
const int Red = 21;
const int Green = 22;
const int Blue = 23;
int sensorValue = 0;

const int startSOS = 35;
const int endSOS = 34;
int startButtonState = 0;
int endButtonState = 0;

bool sosActive = false; //sos state

const char* sosChar = "...---...";  // sos array char* pointer to memory address
const int sosLength = 9;

const unsigned long dotDuration = 500;
const unsigned long dashDuration = dotDuration * 2;
const unsigned long symbolGap = dotDuration;
const unsigned long letterGap = dotDuration * 2;

// --- Array to hold the ON duration for each symbol (built in setup) ---
unsigned long sosDurations[sosLength];

// --- SOS state machine variables (no enum, just a bool) ---
bool sosOn = true;   //if true = ON phase, if false = OFF/gap phase
int sosIndex = 0; 
unsigned long sosPhaseStart = 0;
//--------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);

  pinMode(startSOS, INPUT);
  pinMode(endSOS, INPUT);

  //for loop to convert the sosChar string into an integer duration array
  for (int i = 0; i < sosLength; i++) {
    if (sosChar[i] == '.') {
      sosDurations[i] = dotDuration;
    } else {
      sosDurations[i] = dashDuration;
    }
  }
}
//--------------------------------------------------------------------
void loop() {
  startButtonState = digitalRead(startSOS);
  endButtonState = digitalRead(endSOS);

  if (startButtonState == HIGH && !sosActive) {
    sosActive = true;
    sosIndex = 0;
    sosOn = true;
    sosPhaseStart = millis();
    allOff();
  }
  if (endButtonState == HIGH) {
    sosActive = false;
    allOff();
  }

  sensorValue = analogRead(p_meter);
  Serial.print("The sensor value is: ");
  Serial.println(sensorValue);

  if (sosActive) {
    updateSOS();
  } else {
    setColorFromSensor();
  }
}//end loop
//--------------------------------------------------------------------
void setColorFromSensor() { //sends arguments to control_LEDs function
  if (sensorValue <= 1000) {
    control_LEDs (LOW, LOW, HIGH);
  } else if (sensorValue <= 2500) {
    control_LEDs (LOW, HIGH, LOW);
  } else {
    control_LEDs (HIGH, LOW, LOW);
  }
}
void control_LEDs (bool R, bool G, bool B) { //turns on/off the LEDs
  digitalWrite(Red, R);
  digitalWrite(Green, G);
  digitalWrite(Blue, B);
}
//--------------------------------------------------------------------
void allOff() { //shuts off all LEDs
  digitalWrite(Red, LOW);
  digitalWrite(Green, LOW);
  digitalWrite(Blue, LOW);
}
//--------------------------------------------------------------------
void updateSOS() {
  unsigned long now = millis();

  if (sosOn) {
    setColorFromSensor(); // color tracks potentiometer

    if (now - sosPhaseStart >= sosDurations[sosIndex]) {
      allOff();
      sosOn = false;
      sosPhaseStart = now;
    }

  } else { // OFF/gap phase
    unsigned long offTime = symbolGap;
    if (sosIndex == 8) {
      offTime += letterGap; // extra pause between each SOS sequence
    }

    if (now - sosPhaseStart >= offTime) {
      sosIndex++;
      if (sosIndex >= sosLength) {
        sosIndex = 0; // setting the index to 0 so after the last character it starts again
      }
      sosOn = true;
      sosPhaseStart = now;
    }
  }
}
