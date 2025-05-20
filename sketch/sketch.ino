/*
  NOT YET INCLUDED: (will write later)
    NUTRIENT DOSING
    CALIBRATION

  RELAY: NORMALLY OPEN
    HIGH = OFF
    LOW = ON
*/

// -- LIBRARIES -- //
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DFRobot_PH.h>
#include <DFRobot_EC10.h>
#include <GravityTDS.h>
#include <Nextion.h>
#include <EEPROM.h>

// -- PINS -- //
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

// -- INSTANCES -- //
OneWire oneWire(tempSensorPin);
DallasTemperature tempSensor(&oneWire);
DFRobot_PH phSensor;
DFRobot_EC10 ecSensor;
GravityTDS tdsSensor; 

// -- VARIABLES -- //
float tempValue = 0;
float phValue = 0;
float phVoltage = 0;
float ecValue = 0;
float ecVoltage = 0;
float tdsValue = 0;

float tempMin = 18;
float tempMax = 26;
float phMin = 5.5;
float phMax = 6.5;
//ec and tds

// -- Nextion Objects : (page id, obj id, "objname") -- //
NexText tempDisplay = NexText(0, 2, "tempDisplay");
NexText phDisplay = NexText(0, 3, "phDisplay");
NexText ecDisplay = NexText(0, 4, "ecDisplay");
NexText tdsDisplay = NexText(0, 5, "tdsDisplay");


// -- MAIN PROGRAM -- //

void setup() {
  Serial.begin(115200);
  nexInit();

  tempSensor.begin();
  phSensor.begin();
  ecSensor.begin();
  tdsSensor.setPin(tdsSensorPin);
  tdsSensor.setAref(5.0);       // reference voltage on ADC, default 5.0V on Arduino UNO
  tdsSensor.setAdcRange(1024);  // 1024 for 10bit ADC; 4096 for 12bit ADC
  tdsSensor.begin();

  pinMode(waterCooler, OUTPUT);
  pinMode(coolerFan, OUTPUT);
  pinMode(coolerPump, OUTPUT);
  pinMode(phUpPump, OUTPUT);
  pinMode(phDownPump, OUTPUT);
  pinMode(nutrientAPump, OUTPUT);
  pinMode(nutrientBPump, OUTPUT);

  // initialize PWM pins to 0 (off)
  analogWrite(waterCooler, 0);
  analogWrite(coolerFan, 0);

  // pumps off (HIGH = OFF)
  digitalWrite(coolerPump, HIGH);
  digitalWrite(phUpPump, HIGH);
  digitalWrite(phDownPump, HIGH);
  digitalWrite(nutrientAPump, HIGH);
  digitalWrite(nutrientBPump, HIGH);
}

void loop() {
  tempValue = readTemp();
  phValue = readPh(tempValue);
  ecValue = readEc(tempValue);
  tdsValue = readTds(tempValue);

  displayReadings(tempDisplay, tempValue);
  displayReadings(phDisplay, phValue);
  displayReadings(ecDisplay, ecValue);
  displayReadings(tdsDisplay, tdsValue);

  regulateTemp(tempValue);
  regulatePH(phValue);
  // fertigation(ecValue, tdsValue);

  delay(1000);
}

float readTemp() {
  tempSensor.requestTemperatures();
  tempValue = tempSensor.getTempCByIndex(0);

  return tempValue;
}

float readPh(float temperature) {
  int adcValue = analogRead(phSensorPin);
  phVoltage = adcValue / 1024.0 * 5000;               // mV -- read the pH voltage
  phValue = phSensor.readPH(phVoltage, temperature);  // convert voltage to pH with temp compensation

  return phValue;
}

float readEc(float temperature) {
  int adcValue = analogRead(ecSensorPin);
  ecVoltage = adcValue / 1024.0 * 5000;               // mV -- read the ec voltage
  ecValue = ecSensor.readEC(ecVoltage, temperature);  // convert voltage to EC with temp compensation

  return ecValue;
}

float readTds(float temperature) {
  tdsSensor.setTemperature(temperature);  // set and execute temp compensation
  tdsSensor.update();                     // sample and calculate
  tdsValue = tdsSensor.getTdsValue();     // get the value

  return tdsValue;
}

void regulateTemp(float temperature) {
  if (temperature > tempMax) {
    for (int pwm = 0; pwm < 255; pwm++) {
      analogWrite(waterCooler, pwm);
      analogWrite(coolerFan, pwm);
      delay(10);
    }
    digitalWrite(coolerPump, LOW);
  } else if (temperature < tempMin) {
    for (int pwm = 255; pwm > 0; pwm--) {
      analogWrite(waterCooler, pwm);
      analogWrite(coolerFan, pwm);
      delay(10);
    }
    digitalWrite(coolerPump, HIGH);
  }
}

void regulatePH(float ph) {
  if (ph > phMax) {
    digitalWrite(phDownPump, LOW);
    digitalWrite(phUpPump, HIGH);
  } else if (ph < phMin) {
    digitalWrite(phUpPump, LOW);
    digitalWrite(phDownPump, HIGH);
  } else {
    digitalWrite(phUpPump, HIGH);
    digitalWrite(phDownPump, HIGH);
  }
}

// void fertigation() {
//   //code for nutrient dosing here
// }

void displayReadings(NexText &obj, float sensor) {
  char buffer[10];
  dtostrf(sensor, 4, 2, buffer);
  obj.setText(buffer);
}

// -- FOR CALIBRATION ONLY -- //
// code here