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

#include <SPI.h>
#include <MySensors.h>
#include <Bounce2.h>


#define CHILD_IDB1 3
#define BUTTON_PIN 3
#define CHILD_IDB3 7
#define BUTTON_PIN3 7    
#define CHILD_IDB2 4
#define BUTTON_PIN2 4

#define MOTOR_MIN 0 // Fine tune your servos min. 0-180
#define MOTOR_MAX 1023  // Fine tune your servos max. 0-180
#define DETACH_DELAY 900 // Tune this to let your movement finish before detaching the servo
#define CHILD_ID 10   // Id of the sensor child

Bounce debouncer = Bounce(); 
Bounce debouncer2=Bounce();
Bounce debouncer3=Bounce();

int oldValue=-1;
int oldValue2=-1;
int oldValue3=-1;





MyMessage msg(CHILD_ID, V_DIMMER);
MyMessage msgb1(CHILD_IDB1,V_TRIPPED);
MyMessage msgb2(CHILD_IDB2,V_TRIPPED);
MyMessage msgb3(CHILD_IDB3,V_TRIPPED);

unsigned long timeOfLastChange = 0;
bool attachedServo = false;
 
int pin2=6;   //Entrada 2 del L293D
int pin7=5;  //Entrada 7 del L293D




 
void setup() 
{ 
  // Request last servo state at startup
  request(CHILD_ID, V_DIMMER);
  pinMode(pin2,OUTPUT);
  pinMode(pin7, OUTPUT);
  Serial.begin(9600);
  pinMode(BUTTON_PIN,INPUT);
  pinMode(BUTTON_PIN2,INPUT);
  pinMode(BUTTON_PIN3,INPUT);
  digitalWrite(BUTTON_PIN3, HIGH);
  digitalWrite(BUTTON_PIN, HIGH);
  digitalWrite(BUTTON_PIN2, HIGH);
  debouncer.attach(BUTTON_PIN);
  debouncer2.attach(BUTTON_PIN2);
  debouncer3.attach(BUTTON_PIN3);
  debouncer.interval(5);
  debouncer2.interval(5);
  debouncer3.interval(5);

} 

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("motor", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID, S_COVER);
  present(CHILD_IDB1, S_DOOR);  
  present(CHILD_IDB2, S_DOOR);  
  present(CHILD_IDB3, S_DOOR);  


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
  debouncer3.update();
  int value =debouncer.read();
  int value2=debouncer2.read();
  int value3=debouncer3.read();
  
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
  if(value3!=oldValue3){
    send(msgb3.set(value3==HIGH ? 1:0));
    oldValue3=value3;
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

void receive(const MyMessage &message) {
  Serial.println("mesage recived");
  //myservo.attach(SERVO_DIGITAL_OUT_PIN);   
  //attachedServo = true;
  if (message.type==V_DIMMER) { // This could be M_ACK_VARIABLE or M_SET_VARIABLE
     int val = message.getInt();
     //myservo.write(SERVO_MAX + (SERVO_MIN-SERVO_MAX)/100 * val); // sets the servo position 0-180
     int potenc= MOTOR_MAX + (MOTOR_MIN-MOTOR_MAX)/100 * val;
     updateVal(potenc);
     // Write some debug info
     Serial.print("Motor changed. new state: ");
     Serial.println(val);
   } else if (message.type==V_UP) {
     Serial.println("motor UP command");
     //myservo.write(SERVO_MIN);
     int potenc=MOTOR_MIN;
     updateVal(potenc);
     send(msg.set(100));
   } else if (message.type==V_DOWN) {
     Serial.println("motor DOWN command");
     //myservo.write(SERVO_MAX); 
     int potenc=MOTOR_MAX;
     updateVal(potenc);
     send(msg.set(0));
   } else if (message.type==V_STOP) {
     Serial.println("motor STOP command");
     //myservo.detach();
     //attachedServo = false;

   }
}


