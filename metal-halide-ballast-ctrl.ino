/*
* A controller for Schiederwerk PVG 12-12 AC Electronic Lamp Power Supply
* for metal halide lamps
* 
* © Albertas Mickėnas 2016 
* albertas@technarium.lt mic@wemakethings.net
* This code is Open Source distributed under Apache 2.0 licence.
*/

#include <Bounce2.h>

#include "thermistor.h"

#define LED_RED 5
#define LED_GREEN 2 
#define BUTTON_START 8
#define BUTTON_STOP 11

#define CTRL_ON 4
#define CTRL_DIMM 6

#define FAN 3

#define POT A7
#define SENSOR1 A0
#define SENSOR2 A2
#define SENSE_LAMP_ON A5

Bounce buttonStart= Bounce(); 
Bounce buttonStop= Bounce(); 

typedef enum {FAN_OFF, FAN_ON} fan_state_t;
fan_state_t fanState = FAN_OFF;

void fanStart() {
  digitalWrite(FAN, HIGH);
  delay(1000);
  fanState = FAN_ON;
}

void fanStop() {
  digitalWrite(FAN, LOW);
  fanState = FAN_OFF;
}

void blinkError() {
  digitalWrite(LED_RED, HIGH);
  delay(200);
  digitalWrite(LED_RED, LOW);
  delay(200);
  digitalWrite(LED_RED, HIGH);
  delay(200);
  digitalWrite(LED_RED, LOW);
  delay(200);
  digitalWrite(LED_RED, HIGH);
  delay(200);
  digitalWrite(LED_RED, LOW);
  delay(200);
}

void fanServo(uint16_t temp) {
  if(temp > 400) {
    if(FAN_OFF == fanState) {
      fanStart();
      analogWrite(FAN, map(temp - 400, 0, 300, 128, 255));
    }
  } else {
    fanStop();
  }
}

uint16_t getTemperature(int sensor){
  uint32_t adc = analogRead(sensor);
  return thermistorLsbToTemperature(adc);  
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  pinMode(CTRL_ON, OUTPUT);
  pinMode(CTRL_DIMM, OUTPUT);

  pinMode(FAN, OUTPUT);
  
  pinMode(BUTTON_STOP, INPUT);
  buttonStop.attach(BUTTON_STOP);
  buttonStop.interval(100);
  pinMode(BUTTON_START, INPUT);
  buttonStart.attach(BUTTON_START);
  buttonStart.interval(100);
  pinMode(SENSE_LAMP_ON, INPUT);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  fanStart();
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  fanStop();
  Serial.begin(9600);
  Serial.println("Hello");
}


typedef enum {STATE_IDLE, STATE_START_DELAY, STATE_RUN, STATE_FAULT} state_t;

state_t currState = STATE_IDLE;
uint16_t currLevel;
uint16_t temp;
uint32_t runTimestamp, startTimestamp;

uint8_t didNotStart() {
  return millis() - runTimestamp > 1000 && HIGH == digitalRead(SENSE_LAMP_ON);
}

void loop() {

  temp = (getTemperature(SENSOR1) + getTemperature(SENSOR2)) / 2;
  Serial.println(temp);
  fanServo(temp);

  switch(currState){
  case STATE_IDLE:
    buttonStart.update();
    if(buttonStart.fell()){
      currState = STATE_START_DELAY;
      startTimestamp = millis();
    }
    break;
  case STATE_START_DELAY:
    if(millis() - startTimestamp > 5000) {
      digitalWrite(CTRL_ON, HIGH);
      runTimestamp = millis();
      currState = STATE_RUN;
    } else {
      digitalWrite(LED_GREEN, HIGH);
      delay(100);
      digitalWrite(LED_GREEN, LOW);
      delay(100);
    }
    
    break;
  case STATE_RUN:
    if(didNotStart()) {
      blinkError();

      buttonStart.update();
      while(LOW == buttonStart.read()){
        blinkError();
        buttonStart.update();
      }
      digitalWrite(CTRL_ON, LOW);
      currState = STATE_IDLE;
    }

    currLevel = map(analogRead(POT), 0, 1023, 0, 255);
    analogWrite(CTRL_DIMM, currLevel);
    
    buttonStop.update();
    if(buttonStop.fell()) {
      digitalWrite(CTRL_ON, LOW);
      currState = STATE_IDLE;
    }
    if(temp > 800) {
      digitalWrite(CTRL_ON, LOW);
      currState = STATE_FAULT;
      digitalWrite(LED_RED, HIGH);      
    }
    break;
  case STATE_FAULT:
    break;
  }

  if(LOW == digitalRead(SENSE_LAMP_ON)) {
    digitalWrite(LED_GREEN, HIGH);
  } else {
    digitalWrite(LED_GREEN, LOW);
  }


}
