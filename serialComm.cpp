
#include "Arduino.h"
#include "serialComm.h"
#include "globals.h"


// how much serial data we expect before a newline
const unsigned int MAX_INPUT = 50;

void processIncomingByte (const byte inByte){
  static char input_line [MAX_INPUT];
  static unsigned int input_pos = 0;
  //Serial.print("inByte: ");
  //Serial.println(inByte);
  switch (inByte) {
    case '\n':   // end of text
      /*input_line [input_pos] = '\r';
      input_pos++;
      input_line [input_pos] = '\n';
      input_pos++;
      */
      for (int i=input_pos; i<MAX_INPUT; i++) {
        input_line [i] = 0;  // terminating null byte

      }
      
      // terminator reached! process input_line here ...
      processCommand (input_line);
      //Serial.println(input_line);
      // reset buffer for next time
      input_pos = 0;  
      break;
  
    case '\r':   // discard carriage return
      break;
  
    default:
      // keep adding if not full ... allow for terminating null byte
      if (input_pos < (MAX_INPUT - 1))
        input_line [input_pos++] = inByte;
      break;

  }  // end of switch
  
}

void processCommand(char* command){
  Serial.print(String(command)+'\13'+'\10');
  
  char* cmd_code = strtok(command, ":");
  //Serial.print("CMD code: ");
  //Serial.println(cmd_code);
  
  if (strcmp(cmd_code,"SW_PINS") == 0) {
    //Serial.println("Set switch pins...");
    cmd_code = strtok(NULL, ":");
    int i = 0;
    while( cmd_code != NULL ){
      
      //Serial.print("pin: ");
      //Serial.println(cmd_code);
      int pin = atoi(cmd_code);
      pinMode(pin,INPUT_PULLUP);
      SWITCH_ARRAY[i].attach(pin);
      SWITCH_ARRAY[i].update();
      int value = SWITCH_ARRAY[i].read();
      sendInputValue("SW:", SWITCH_ARRAY[i].getPin(), value);
      cmd_code = strtok(NULL, ":");
      i++;
    }
    for (int j=i; j<32; j++){
      SWITCH_ARRAY[j].attach(-1);
    }
    /* for(int i = 0; i<16; i++){
      Serial.println("Switch nr "+ (String)i+ " pin: "+ (String)SWITCH_ARRAY[i].getPin());
    }*/
  }
  
  if (strcmp(cmd_code,"POT_PINS") == 0) {
    //Serial.println("");
    //Serial.println("Set pot pins...");
    cmd_code = strtok(NULL, ":");
    int i = 0;
    while( cmd_code != NULL ){
      //Serial.print("cmd chunk, pin: ");
      //Serial.println(cmd_code);
      int pin = atoi(cmd_code);
      POT_ARRAY[i].attach(pin);
      POT_ARRAY[i].update();
      int value = POT_ARRAY[i].read();
      sendInputValue("POT:",POT_ARRAY[i].getPin(), value);
      cmd_code = strtok(NULL, ":");
      
      i++;
    }
    for (int j=i; j<16; j++){
      POT_ARRAY[j].attach(-1);
    }
    /*for(int i = 0; i<16; i++){
      Serial.println("Pot nr "+ (String)i+ " pin: "+ (String)POT_ARRAY[i].getPin());
    }*/
  }
  
}

void sendInputValue(String inputType, uint8_t pin,int value){
  Serial.print(inputType+(String)pin+":"+(String)value+';');
  //Serial.println(inputType+(String)pin+":"+(String)value);
  //Serial.println(value);
  delay(1);
}

void sendXP_Cmd(String command){
  Serial.print("CMND0"+command+'\13'+'\10');
  delay(1);
}

void sendXP_Dref(float value, String dataref){
  union{ byte bv[4]; float fv;} bf;
  bf.fv = value; 

  /*char DataRef[500]; 
  int n = 0;    
  while (dataref[n]) { DataRef[n] = dataref[n]; n++;}   
  for (int i = n; i < 499; i++) DataRef[i] = char(32); DataRef[500] = char(0);
  */
  Serial.print("DREF0");
  for (int i=0; i<4; i++)  Serial.print((char)bf.bv[i]); 
  Serial.print(dataref+'\13'+'\10');
    
}