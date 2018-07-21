#include <Servo.h>

#include "Bounce2ST.h"
#include "ClickEncoderST.h"
#include "potentiometer.h"
#include "pwm.h"
#include "ServoST.h"
#include "PWMServo.h"
#include "serialComm.h"
#include "globals.h"
#include <TimerThree.h>
#include <EEPROM.h>

#define BAUD 57600
#define VERSION "1.3"

Bounce          SWITCH_ARRAY[MAX_NR_SWITCHES] ;
Potentiometer   POT_ARRAY[MAX_NR_POTS];
PWM             PWM_ARRAY[MAX_NR_PWMS];
//ServoST         SERVO_ARRAY[MAX_NR_SERVOS];
PWMServo         SERVO_ARRAY[MAX_NR_SERVOS];
DigOutput       DIGOUTPUTS_ARRAY[MAX_NR_DIGOUTPUTS];
ClickEncoder    ROTENC_ARRAY[MAX_NR_ENCODERS];
int16_t         lastEncoderValues[MAX_NR_ENCODERS];
int16_t         currentEncoderValues[MAX_NR_ENCODERS];

unsigned long current_time;
unsigned long last_time;

unsigned int deviceID;

void timerIsr() {
  /*for (int i = 0; i < MAX_NR_SERVOS; i++) {
    SERVO_ARRAY[i].reAttach();
  }*/
  
  //Serial.println("service encoders");
  for (int i = 0; i < MAX_NR_ENCODERS; i++) {
    ROTENC_ARRAY[i].service(); 
  }
  /*for (int i = 0; i < MAX_NR_SERVOS; i++) {
    SERVO_ARRAY[i].refresh();
    //SERVO_ARRAY[i].detach();
  }*/
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUD);
  Timer3.initialize(4000);
  Timer3.attachInterrupt(timerIsr); 
  for (int i = 0; i < MAX_NR_ENCODERS; i++) {
    lastEncoderValues[i] = 0; 
    currentEncoderValues[i] = 0; 
  }
  
  last_time = millis();

  // Get 2-byte device ID from address 0x00 in EEPROM
  // Some Arduinos do not have a USB serial number,
  // so we'll keep track of them with this.
  EEPROM.get( 0, deviceID);

}

int val = 0;
int angle = 0; 
bool firstloop = true;

const unsigned int MAX_INPUT = 500;
char ser_buffer [MAX_INPUT];
int buffer_length = 0;


void loop() {
  current_time = millis();
  if (current_time - last_time > 500) {
    //Serial.println("ARD_RUNNING;");
    last_time = current_time;
  }
  
  //Serial.println("looping");
  //noInterrupts();
  while (Serial.available () > 0){
    //Serial.print("Serial data available!");
    processIncomingByteOld (Serial.read ());
  }
  //interrupts();
  processCommandsBuffer();
  
  for (int i = 0; i < MAX_NR_SWITCHES; i++) {
    if (SWITCH_ARRAY[i].update() == 1) {
      int value = SWITCH_ARRAY[i].read();
      sendInputValue("SW:", SWITCH_ARRAY[i].getPin(), value);
    } 
  }
  for (int i = 0; i < MAX_NR_POTS; i++) {
    if (POT_ARRAY[i].update() == true) {
      //Serial.println("POT VALUE CHANGED: POT nr"+ (String)i + " pin: "+(String) POT_ARRAY[i].getPin());
      int value = POT_ARRAY[i].read();
      sendInputValue("POT:",POT_ARRAY[i].getPin(), value);
    } 
  }

  for (int i = 0; i < MAX_NR_ENCODERS; i++) {
    currentEncoderValues[i] += ROTENC_ARRAY[i].getValue();
    if (currentEncoderValues[i] != lastEncoderValues[i]) {
      //Serial.println("Encoder "+(String)i+" value: "+(String)currentEncoderValues[i]); 
      
      sendInputValue("ROTENC:",ROTENC_ARRAY[i].getPin(), currentEncoderValues[i]-lastEncoderValues[i]);
      lastEncoderValues[i] = currentEncoderValues[i];
    }
    
  }
  
  for (int i = 0; i < MAX_NR_PWMS; i++) {
    PWM_ARRAY[i].writeValue(); 
  }
  for (int i = 0; i < MAX_NR_DIGOUTPUTS; i++) {
    DIGOUTPUTS_ARRAY[i].writeValue(); 
  }/*
  for (int i = 0; i < MAX_NR_SERVOS; i++) {
    //SERVO_ARRAY[i].reAttach();
    SERVO_ARRAY[i].refresh();
    //SERVO_ARRAY[i].detach();
  }*/
  
  if (firstloop == true){ 
    Serial.print("READY;");
    Serial.print("VERSION:"+(String)VERSION+";");
    Serial.print("ID:0x"+(String(deviceID,HEX))+";");
    firstloop = false;
  }
  /**
  // scan from 0 to 180 degrees
  for(angle = 0; angle < 180; angle++)  
  {                                  
    SERVO_ARRAY[0].write(angle);               
    delay(15);                   
  } 
  // now scan back from 180 to 0 degrees
  for(angle = 180; angle > 0; angle--)    
  {                                
    SERVO_ARRAY[0].write(angle);           
    delay(15);       
  } */
  
}
