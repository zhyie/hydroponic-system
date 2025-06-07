/*
  NOT YET INCLUDED: (will write later)
    NUTRIENT DOSING
    CALIBRATION

  RELAY: NORMALLY OPEN
    HIGH = OFF
    LOW = ON
*/

// ==== LIBRARIES ==== //
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DFRobot_PH.h>
#include <DFRobot_EC10.h>
#include <GravityTDS.h>
#include <Nextion.h>
#include <EEPROM.h>

// ==== COMPONENT PINS ON ARDUINO ==== //
#define phSensorPin A0
#define ecSensorPin A1
#define tdsSensorPin A2
#define tempSensorPin 13
#define phUpPump 12
#define phDownPump 11
#define nutrientAPump 10
#define nutrientBPump 9
#define coolerPump 8
#define waterCooler 7
#define coolerFan 6
#define nexSerial Serial3

// ==== SENSOR INSTANCES ==== //
OneWire oneWire(tempSensorPin);
DallasTemperature tempSensor(&oneWire);
DFRobot_PH phSensor;
DFRobot_EC10 ecSensor;
GravityTDS tdsSensor; 

// ==== VARIABLES ==== //
float tempValue = 0.0;
float phValue = 0.0;
float phVoltage = 0.0;
float ecValue = 0.0;
float ecVoltage = 0.0;
float tdsValue = 0.0;
//following variables were set according to plant used: lettuce
//can be customize through nextion display for user
float tempMin = 18;
float tempMax = 26;
float phMin = 5.5;
float phMax = 6.5;
float ecMin = 1.6;
float ecMax = 1.2;
float tdsMin = 768;
float tdsMax = 1024;

// ==== Nextion Objects : (page id, obj id, "objname") ==== //
NexText tempDisplay = NexText(0, 2, "tempDisplay");
NexText phDisplay = NexText(0, 3, "phDisplay");
NexText ecDisplay = NexText(0, 4, "ecDisplay");
NexText tdsDisplay = NexText(0, 5, "tdsDisplay");

// ==== FOR NON BLOCKING MILLIS USE ==== //
unsigned long previousMillis = 0;
const long interval = 10000;

int pwmValue = 0;
bool rampingUp = false;
bool rampingDown = false;
unsigned long lastRampTime = 0;
const unsigned long rampInterval = 10;

enum PumpState { IDLE, PUMPING, WAITING };
PumpState phUpState = IDLE;
PumpState phDownState = IDLE;

unsigned long pumpStartTimeUp = 0;
unsigned long pumpStartTimeDown = 0;
unsigned long waitStartTimeUp = 0;
unsigned long waitStartTimeDown = 0;

const unsigned long pumpDuration = 30000;  // 30 seconds pump ON (~9.5 mL at 19 mL/min)
const unsigned long waitDuration = 60000;  // 60 seconds wait for sensor stabilization


// ==== for calibration use ==== //
bool calibrationMode = false;
char cmd[10];

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////  MAIN PROGRAM   /////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  //program uses 115200 baud rate
  Serial.begin(115200);
  nexSerial.begin(115200);

  nexInit(); // initialize nextion
  programinit(); // setup codes at the bottom
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    tempValue = readTemp();
    phValue = readPh(tempValue);
    ecValue = readEc(tempValue);
    tdsValue = readTds(tempValue);

    displayReadings(tempDisplay, tempValue);
    displayReadings(phDisplay, phValue);
    displayReadings(ecDisplay, ecValue);
    displayReadings(tdsDisplay, tdsValue);

    // //for debugging display
    // Serial.print("TempC: "); Serial.println(tempValue);
    // Serial.print("pH Level: "); Serial.println(phValue);
    // Serial.print("EC Value: "); Serial.println(ecValue);
    // Serial.print("TDS Value: "); Serial.println(tdsValue);

    regulateTemp(tempValue);
    regulatePH(phValue);
    // fertigation(ecValue, tdsValue);
  }

  // if (currentMillis - previousMillisDisplay >= intervalDisplay) {
  //   previousMillisDisplay = currentMillis;
  //   displayReadings(tempDisplay, tempValue);
  //   displayReadings(phDisplay, phValue);
  //   displayReadings(ecDisplay, ecValue);
  //   displayReadings(tdsDisplay, tdsValue);
  // }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////  SENSOR READINGS   //////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

float readTemp() {
  tempSensor.requestTemperatures();           // request temperature data from sensor
  tempValue = tempSensor.getTempCByIndex(0);  // get the data in Celsius

  return tempValue;
}

float readPh(float temperature) {
  int adcValue = analogRead(phSensorPin);
  phVoltage = adcValue / 1024.0 * 5000;               // mV, read the pH voltage
  phValue = phSensor.readPH(phVoltage, temperature);  // convert voltage to pH with temp compensation

  return phValue;
}

float readEc(float temperature) {
  int adcValue = analogRead(ecSensorPin);
  ecVoltage = adcValue / 1024.0 * 5000;               // mV, read the ec voltage
  ecValue = ecSensor.readEC(ecVoltage, temperature);  // convert voltage to EC with temp compensation

  return ecValue;
}

float readTds(float temperature) {
  tdsSensor.setTemperature(temperature);  // set and execute temp compensation
  tdsSensor.update();                     // sample and calculate
  tdsValue = tdsSensor.getTdsValue();     // get the value

  return tdsValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////  LOWERS WATER TEMPERATURE   /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void regulateTemp(float temperature) {
  unsigned long currentMillis = millis();

  if (temperature > tempMax && !rampingUp) {
    // start ramping up PWM
    rampingUp = true;
    rampingDown = false;
    pwmValue = 0;
    lastRampTime = currentMillis;
  } else if (temperature < tempMin && !rampingDown) {
    // start ramping down PWM
    rampingDown = true;
    rampingUp = false;
    pwmValue = 255;
    lastRampTime = currentMillis;
  }

  if (rampingUp) {
    if (currentMillis - lastRampTime >= rampInterval) {
      lastRampTime = currentMillis;
      if (pwmValue < 255) {
        pwmValue++;
        analogWrite(waterCooler, pwmValue);
        analogWrite(coolerFan, pwmValue);
      } else {
        rampingUp = false; // done ramping
      }
    }
    digitalWrite(coolerPump, LOW);
  } else if (rampingDown) {
    if (currentMillis - lastRampTime >= rampInterval) {
      lastRampTime = currentMillis;
      if (pwmValue > 0) {
        pwmValue--;
        analogWrite(waterCooler, pwmValue);
        analogWrite(coolerFan, pwmValue);
      } else {
        rampingDown = false; // done ramping
      }
    }
    digitalWrite(coolerPump, HIGH);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////  BALANCE PH LEVEL   /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void regulatePH(float ph) {
  unsigned long currentMillis = millis();

  // Handle pH UP pump control
  if (ph > phMax) {
    switch (phUpState) {
      case IDLE:
        digitalWrite(phUpPump, LOW);  // Turn pump ON (active LOW)
        pumpStartTimeUp = currentMillis;
        phUpState = PUMPING;
        break;
      case PUMPING:
        if (currentMillis - pumpStartTimeUp >= pumpDuration) {
          digitalWrite(phUpPump, HIGH);  // Turn pump OFF
          waitStartTimeUp = currentMillis;
          phUpState = WAITING;
        }
        break;
      case WAITING:
        if (currentMillis - waitStartTimeUp >= waitDuration) {
          if (ph > phMax) {
            digitalWrite(phUpPump, LOW);  // Pump again
            pumpStartTimeUp = currentMillis;
            phUpState = PUMPING;
          } else {
            phUpState = IDLE;  // pH in range, stop pumping
          }
        }
        break;
    }
  } else {
    // If pH not high, ensure pump is off and reset state
    digitalWrite(phUpPump, HIGH);
    phUpState = IDLE;
  }

  // Handle pH DOWN pump control
  if (ph < phMin) {
    switch (phDownState) {
      case IDLE:
        digitalWrite(phDownPump, LOW);  // Turn pump ON (active LOW)
        pumpStartTimeDown = currentMillis;
        phDownState = PUMPING;
        break;
      case PUMPING:
        if (currentMillis - pumpStartTimeDown >= pumpDuration) {
          digitalWrite(phDownPump, HIGH);  // Turn pump OFF
          waitStartTimeDown = currentMillis;
          phDownState = WAITING;
        }
        break;
      case WAITING:
        if (currentMillis - waitStartTimeDown >= waitDuration) {
          if (ph < phMin) {
            digitalWrite(phDownPump, LOW);  // Pump again
            pumpStartTimeDown = currentMillis;
            phDownState = PUMPING;
          } else {
            phDownState = IDLE;  // pH in range, stop pumping
          }
        }
        break;
    }
  } else {
    // If pH not low, ensure pump is off and reset state
    digitalWrite(phDownPump, HIGH);
    phDownState = IDLE;
  }

  // If pH in range, ensure both pumps are off
  if (ph >= phMin && ph <= phMax) {
    digitalWrite(phUpPump, HIGH);
    digitalWrite(phDownPump, HIGH);
    phUpState = IDLE;
    phDownState = IDLE;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////  CONSISTENT FERTIGATION   ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// void fertigation() {
//   //code for nutrient dosing here
// }

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////  SEND SENSOR READINGS TO NEXTION DISPLAY   //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void displayReadings(NexText &obj, float sensor) {
  char buffer[16];
  dtostrf(sensor, 4, 2, buffer);
  obj.setText(buffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////  DFROBOT SENSOR CALIBRATION   ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// calibration process by Serail CMD, will change soon for calibration through nextion display
void calibrateSensor() {
  static unsigned long timepoint = millis();

  if(millis()-timepoint>1000U) {
    if(Serial.available() > 0) {
      cmd = Serial.readStringUntil('\n');
      cmd.trim();

      if (strstr(cmd, "PH")){
        phVoltage = analogRead(phSensorPin) / 1024.0 * 5000;
        float temperature = readTemp();
        phSensor.calibration(phVoltage, temperature, cmd);
        Serial.println("pH cmd passed");
      } 
      
      else if (strstr(cmd, "EC")) {
        ecVoltage = analogRead(ecSensorPin) / 1024.0 * 5000;
        float temperature = readTemp();
        ecSensor.calibration(ecVoltage, temperature, cmd);
        Serial.println("EC cmd passed");
      }
    }
  }
}

void calibratePH(float temperature, float phVoltage, const char* cmd) {
  static unsigned long timepoint = millis();

  if(millis()-timepoint>1000U) {
    phSensor.calibration(phVoltage, temperature, cmd);
  }
}

void calibrateEC(float temperature, float ecVoltage, const char* cmd) {
  static unsigned long timepoint = millis();

  if(millis()-timepoint>1000U) {
    ecSensor.calibration(ecVoltage, temperature, cmd);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////  FOR SETUP CODE   ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void programinit() {
  // nexInit();

  // initialize sensors
  tempSensor.begin();
  phSensor.begin();
  ecSensor.begin();
  tdsSensor.setPin(tdsSensorPin);   // set tds sensor pin
  tdsSensor.setAref(5.0);           // reference voltage on ADC, default 5.0V on Arduino UNO
  tdsSensor.setAdcRange(1024);      // 1024 for 10bit ADC; 4096 for 12bit ADC
  tdsSensor.begin();

  // initialize actuators as output pin
  pinMode(waterCooler, OUTPUT);
  pinMode(coolerFan, OUTPUT);
  pinMode(coolerPump, OUTPUT);
  pinMode(phUpPump, OUTPUT);
  pinMode(phDownPump, OUTPUT);
  pinMode(nutrientAPump, OUTPUT);
  pinMode(nutrientBPump, OUTPUT);

  // initialize peltier and fan PWM to 0 (off)
  analogWrite(waterCooler, 0);
  analogWrite(coolerFan, 0);

  // initialize pumps off (HIGH = OFF)
  digitalWrite(coolerPump, HIGH);
  digitalWrite(phUpPump, HIGH);
  digitalWrite(phDownPump, HIGH);
  digitalWrite(nutrientAPump, LOW);
  digitalWrite(nutrientBPump, HIGH);
}