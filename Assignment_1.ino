const int p_meter = 32;
const int Red = Red;
const int Green = Green;
const int Blue = Blue;
int sensorValue = 0;
const int startSOS = 35;
const int endSOS = 34;
int startButtonState = 0;
int endButtonState = 0;


void setup() {
  Serial.begin(115200);

  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);

  pinMode(startSOS, INPUT);
  pinMode(endSOS, INPUT);
  
}

void loop() {
  sensorValue = analogRead(p_meter);
  Serial.println(sensorValue);
  if (sensorValue <= 1000){
    digitalWrite(Blue, HIGH);
    digitalWrite(Green, LOW);
    digitalWrite(Red, LOW);
  }else if(sensorValue <= 2500){
    digitalWrite(Green, HIGH);
    digitalWrite(Red, LOW);
    digitalWrite(Blue, LOW);
  }else{
    digitalWrite(Red, HIGH);
    digitalWrite(Green, LOW);
    digitalWrite(Blue, LOW);
  }
}
