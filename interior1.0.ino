/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik Ekblad
 * Contribution by: Derek Macias
 * 
 * DESCRIPTION
 * Example showing how to create an atuator for a servo.
 * Connect red to +5V, Black or brown to GND and the last cable to Digital pin 3.
 * The servo consumes much power and should probably have its own powersource.'
 * The arduino might spontanally restart if too much power is used (happend
 * to me when servo tried to pass the extreme positions = full load).
 * http://www.mysensors.org/build/servo
 */

// Enable debug prints to serial monitor
#define MY_DEBUG 

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>

#define CHILD_IDB1 3
#define BUTTON_PIN 3  
#define CHILD_IDB2 4
#define BUTTON_PIN2 4

#define RELAY_1  5  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

Bounce debouncer = Bounce(); 
Bounce debouncer2=Bounce();
int oldValue=-1;
int oldValue2=-1;
int relvalue=-1;

MyMessage msgb1(CHILD_IDB1,V_TRIPPED);
MyMessage msgb2(CHILD_IDB2,V_TRIPPED);

void before()
{
    for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
        // Then set relay pins in output mode
        pinMode(pin, OUTPUT);
        // Set relay to last known state (using eeprom storage)
        digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
        relvalue=loadState(sensor)?RELAY_ON:RELAY_OFF;
    }
}



unsigned long timeOfLastChange = 0;
bool attachedServo = false;
 
int pin2=6;   //Entrada 2 del L293D
int pin7=5;  //Entrada 7 del L293D

 
void setup() 
{ 
  // Request last servo state at startup
  pinMode(pin2,OUTPUT);
  pinMode(pin7, OUTPUT);
  Serial.begin(9600);
  pinMode(BUTTON_PIN,INPUT);
  pinMode(BUTTON_PIN2,INPUT);
  digitalWrite(BUTTON_PIN, HIGH);
  digitalWrite(BUTTON_PIN2, HIGH);
  debouncer.attach(BUTTON_PIN);
  debouncer2.attach(BUTTON_PIN2);
  debouncer.interval(5);
  debouncer2.interval(5);
} 

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("bombillas_int", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_IDB1, S_DOOR);  
  present(CHILD_IDB2, S_DOOR); 
    sendSketchInfo("Relay", "1.0");

    for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
        // Register all sensors to gw (they will be created as child devices)
        present(sensor, S_BINARY);
    } 

}

void loop() 
{ 
  Serial.println("looped");
  //if (attachedServo && millis() - timeOfLastChange > DETACH_DELAY) {
  //   myservo.detach();
    // attachedServo = false;
  //}
  debouncer.update();
  debouncer2.update();
  int value =debouncer.read();
  int value2=debouncer2.read();
  
  Serial.println(value);
  Serial.println(value2);
  
  if(value!=oldValue){
    send(msgb1.set(value==HIGH ? 1:0));
    oldValue=value;
  }
  
    if(value2!=oldValue2){
    send(msgb2.set(value2==HIGH ? 1:0));
    oldValue2=value2;
  }
  
} 
void updateVal(int val){
  Serial.println("updating");
  int pwm1 = map(val, 0, 1023, 0, 255);
  int pwm2 = map(val, 0, 1023, 255, 0);
  Serial.print("pwm1");
  Serial.println(pwm1);
  Serial.print("pwm2");
  Serial.println(pwm2);
  analogWrite(pin2,pwm1);
  analogWrite(pin7,pwm2); 
  }

void receive(const MyMessage &message)
{
    // We only expect one type of message from controller. But we better check anyway.
    if (message.type==V_STATUS) {
        // Change relay state
        relvalue=relvalue==RELAY_ON?RELAY_OFF:RELAY_ON;
        digitalWrite(message.sensor-1+RELAY_1, relvalue);
        
        // Store state in eeprom
        saveState(message.sensor, message.getBool());
        // Write some debug info
        Serial.print("Incoming change for sensor:");
        Serial.print(message.sensor);
        Serial.print(", New status: ");
        Serial.println(message.getBool());
    }
}

