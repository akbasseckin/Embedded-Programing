#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// pin definition
const int LED_RED = 21;  
const int LED_ORANGE = 4;   
const int LED_GREEN = 22;  

const int BTN_START = 34;
const int BTN_STOP = 35;

const int I2C_SDA = 27;
const int I2C_SCL = 26;

enum SystemState {
  STATE_IDLE,
  STATE_MONITOR,
  STATE_WARNING
};

volatile SystemState currentState = STATE_IDLE;
volatile bool startButtonPressed = false;
volatile bool stopButtonPressed = false;

unsigned long warningStateStartTime = 0; 
const unsigned long warningDuration = 10000;

Adafruit_BME280 bme; 

// interrupts
void IRAM_ATTR onStartButton() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 200) {
    if (currentState == STATE_IDLE) {
      startButtonPressed = true;
    }
    lastInterruptTime = interruptTime;
  }
}

void IRAM_ATTR onStopButton() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 200) {
    if (currentState == STATE_MONITOR) {
      stopButtonPressed = true;
    }
    lastInterruptTime = interruptTime;
  }
}

//leds
void setLEDs(bool red, bool orange, bool green) {
  digitalWrite(LED_RED, red ? HIGH : LOW);
  digitalWrite(LED_ORANGE, orange ? HIGH : LOW);
  digitalWrite(LED_GREEN, green ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  Serial.println("\n=== Environmental Control System Starting ===");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_ORANGE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  pinMode(BTN_START, INPUT);
  pinMode(BTN_STOP, INPUT);

  attachInterrupt(digitalPinToInterrupt(BTN_START), onStartButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_STOP), onStopButton, FALLING);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (bme.begin(0x76)) {  
    Serial.println("Sensor Status: BME280 found at 0x76");
  } else if (bme.begin(0x77)) {
    Serial.println("Sensor Status: BME280 found at 0x77");
  } else {
    Serial.println("CRITICAL ERROR: BME280 not detected!");
    while (1); 
  }
  
  setLEDs(true, false, false); 
}

void loop() {
  switch (currentState) {
    
    case STATE_IDLE:
      setLEDs(true, false, false);
      if (startButtonPressed) {
        startButtonPressed = false; 
        Serial.println("Transition: IDLE -> MONITOR");
        currentState = STATE_MONITOR;
      }
      break;

    case STATE_MONITOR: { 
      setLEDs(false, false, true);

      if (stopButtonPressed) {
        stopButtonPressed = false; 
        Serial.println("Transition: MONITOR -> IDLE");
        currentState = STATE_IDLE;
        break;
      }

      float temperature = bme.readTemperature();
      float pressure = bme.readPressure() / 100.0F; 

      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C | Pressure: ");
      Serial.print(pressure);
      Serial.println(" hPa");

      if (temperature > 25.0) {
        Serial.println("Alert: Temp > 25°C! Transition: MONITOR -> WARNING");
        
        
        warningStateStartTime = millis(); // Take a timestamp snapshot of exactly when we entered WARNING state
        currentState = STATE_WARNING;
      }
      
      delay(1000); 
      break;
    } 

    case STATE_WARNING:
      setLEDs(false, true, false);

      if (millis() - warningStateStartTime >= warningDuration) {
        Serial.println("Timer Expired. Re-initializing sensor and transitioning to MONITOR...");
        
        
        bme.begin(0x76);
        
        currentState = STATE_MONITOR;
      }
      break;

  }
}