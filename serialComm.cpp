
#include "Arduino.h"
#include "serialComm.h"
#include "globals.h"
#include <EEPROM.h>

// how much serial data we expect before a newline
const unsigned int MAX_INPUT = 250;
char commands_buffer[512] = "";
int current_cmdsbuffer_length = 0;

// NOT USED?
void processIncomingByte (const byte inByte){
  char ser_buffer [MAX_INPUT];
  int buffer_length = 0;
  //static char ser_buffer [MAX_INPUT];
  static unsigned int input_pos = 0;
  //Serial.print("inByte: ");
  //Serial.println(inByte);
  if (input_pos < (MAX_INPUT - 1)){ // we fill in the buffer until it is full
    ser_buffer [input_pos] = inByte;
    input_pos++;
  } else { // we have filled the buffer lets analyse it
    ser_buffer [input_pos] = 0; // terminating null byte
    /*Serial.println("")
    char *command = strtok(ser_buffer, "\n");
      
      while (command != NULL){
        
        command = strtok(NULL, "\n"); 
        Serial.println("Processing command:"+(String)command);  
        processCommand(command);
      }
    */
    input_pos = 0; 
  
  }
     
  
  
}

// Called every loop to read serial port
void processIncomingByteOld (const byte inByte){
  static char input_line [MAX_INPUT];
  static unsigned int input_pos = 0;
  //Serial.print("inByte: ");
  //Serial.println(inByte);
  switch (inByte) {
    case ';':   // end of text
      /*input_line [input_pos] = '\r';
      input_pos++;
      input_line [input_pos] = '\n';
      input_pos++;
      */
      input_line [input_pos] = ';';
      input_line [input_pos+1] = 0;
      /*for (int i=input_pos; i<MAX_INPUT; i++) {
        input_line [i] = 0;  // terminating null byte

      }*/
      
      // terminator reached! process input_line here ...
      //Serial.println("");
      //Serial.println("input_line: "+(String)input_line);
      //Serial.println("");
      strcat(commands_buffer, input_line);
      //processCommand (input_line);
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
void processCommandsBuffer(){
  if( strlen(commands_buffer) > 0) { // if there is something in the buffer
    Serial.println(">>ARD DEBUG>processCommandsBuffer>commands buffer: "+(String)commands_buffer);
    char* command = strtok(commands_buffer, ";");
    char command_copy[250];
    while (command != NULL){
      strcpy(command_copy, command);
      processCommand(command_copy);
      command = strtok(NULL, ",");   
    }
    commands_buffer[0] = 0;
  }
}
void processCommand(char* command){
  Serial.print(">>ARD DEBUG>processCommand>command: "+String(command));
  
  char* cmd_code = strtok(command, ":");
  Serial.print(">>ARD DEBUG>processCommand>CMD code: ");
  Serial.print(cmd_code);
  
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
    for (int j=i; j<MAX_NR_SWITCHES; j++){
      SWITCH_ARRAY[j].attach(-1);
    }
    /* for(int i = 0; i<16; i++){
      Serial.println("Switch nr "+ (String)i+ " pin: "+ (String)SWITCH_ARRAY[i].getPin());
    }*/
  }
  
  // ROTENC_PINS:27,29,4:31,33,4:34,35,4
  if (strcmp(cmd_code,"ROTENC_PINS") == 0) {
    //Serial.println("Set rot encoder pins...");
    cmd_code = strtok(NULL, ":");
    int i = 0;
    char cmd_codes[MAX_NR_ENCODERS][30];
    while( cmd_code != NULL ){ // we store all rot enc pin commands in a table first
      //Serial.println(cmd_code);  
      strcpy(cmd_codes[i], cmd_code); 
      cmd_code = strtok(NULL, ":");
      i++;
    }
    for(int l = 0; l<i; l++){ // i is now the nr encoders, we go through the table of commands 
      //Serial.println("Encoder nr "+ (String)l+ " cmdcode: "+ (String)cmd_codes[l]);
      int pinA = -1;
      int pinB = -1;
      int stepsPerNotch = 1;
      int k = 0;
      char values[30];
      strcpy ( values, cmd_codes[l]);
      char *param_val = strtok(values, ",");
      
      while (param_val != NULL){
        if (k == 0){ // we have pin A
          pinA = atoi(param_val);
        } 
        if (k == 1){ // we have pin B
          pinB = atoi(param_val);
        } 
        if (k == 2){ // we have stepspernotch
          stepsPerNotch = atoi(param_val);
        }      
        param_val = strtok(NULL, ",");   
        k++;
      }
      /*Serial.print("pinA: ");
      Serial.print((String)pinA);
      Serial.print("; pinB: ");
      Serial.print((String)pinB);
      Serial.print("; steps per notch: ");
      Serial.println((String)stepsPerNotch);
      */
      ROTENC_ARRAY[l].attach(pinA, pinB, (uint8_t)stepsPerNotch);
    }
    for (int j=i; j<MAX_NR_ENCODERS; j++){
      ROTENC_ARRAY[j].attach(-1,-1);
    }

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
    for (int j=i; j<MAX_NR_POTS; j++){
      POT_ARRAY[j].attach(-1);
    }
    /*for(int i = 0; i<16; i++){
      Serial.println("Pot nr "+ (String)i+ " pin: "+ (String)POT_ARRAY[i].getPin());
    }*/
  }

  if (strcmp(cmd_code,"PWM_PINS") == 0) {
    //Serial.println("");
    //Serial.println("Set pwm pins...");
    cmd_code = strtok(NULL, ":");
    int i = 0;
    while( cmd_code != NULL ){
      //Serial.print("cmd chunk, pin: ");
      //Serial.println(cmd_code);
      int pin = atoi(cmd_code);
      PWM_ARRAY[i].attach(pin);
      cmd_code = strtok(NULL, ":");
      i++;
    }
    for (int j=i; j<MAX_NR_PWMS; j++){
      PWM_ARRAY[j].attach(-1);
    }
  }
  if (strcmp(cmd_code,"PWM") == 0) {
    //Serial.println("");
    //Serial.println("Set PWM pin...");
    cmd_code = strtok(NULL, ":");
    int i = 0;
    int pin = -2;
    int pwm_value = 0;
    while( cmd_code != NULL ){
      //Serial.print("cmd chunk, pin: ");
      //Serial.println(cmd_code);
      if(i == 0){
        pin = atoi(cmd_code);
      } if (i == 1){
        pwm_value = atoi(cmd_code);
      }
      cmd_code = strtok(NULL, ":");
      i++;
    }
    for (int i=0; i<MAX_NR_PWMS; i++){
      if(PWM_ARRAY[i].getPin() == pin){
        //Serial.println("PWM updating PWM value on pin: " + (String)pin + " value: " + pwm_value);
        PWM_ARRAY[i].updateValue(pwm_value);
      }
    }
  }

  if (strcmp(cmd_code,"SERVO_PINS") == 0) {
    //Serial.println("");
    //Serial.println("Set servo pins...");
    cmd_code = strtok(NULL, ":");
    int i = 0;
    while( cmd_code != NULL ){
      //Serial.print("cmd chunk, pin: ");
      //Serial.println(cmd_code);
      int pin = atoi(cmd_code);
      SERVO_ARRAY[i].attach(pin);
      cmd_code = strtok(NULL, ":");
      i++;
    }
    for (int j=i; j<MAX_NR_SERVOS; j++){
      SERVO_ARRAY[j].detach();
    }
  }

  if (strcmp(cmd_code,"SERVO") == 0) {
    //Serial.println("");
    //Serial.println("Set SERVO pin...");
    cmd_code = strtok(NULL, ":");
    int i = 0;
    int pin = -2;
    int servo_angle = 0;
    while( cmd_code != NULL ){
      //Serial.print("cmd chunk, pin: ");
      //Serial.println(cmd_code);
      if(i == 0){
        pin = atoi(cmd_code);
      } if (i == 1){
        servo_angle = atoi(cmd_code);
      }
      cmd_code = strtok(NULL, ":");
      i++;
    }
    for (int i=0; i<MAX_NR_SERVOS; i++){
      if(SERVO_ARRAY[i].getPin() == pin){
        //Serial.println("SERVO updating SERVO angle on pin: " + (String)pin + " angle: " + servo_angle);
        //SERVO_ARRAY[i].reAttach();
        //SERVO_ARRAY[i].updateValue(servo_angle);
        SERVO_ARRAY[i].write(servo_angle);
        
        //SERVO_ARRAY[i].detach();
      }
    }
  }

  if (strcmp(cmd_code,"DIGOUT_PINS") == 0) {
    //Serial.println("");
    //Serial.println("Set digital out pins...");
    cmd_code = strtok(NULL, ":");
    int i = 0;
    while( cmd_code != NULL ){
      //Serial.print("cmd chunk, pin: ");
      //Serial.println(cmd_code);
      int pin = atoi(cmd_code);
      DIGOUTPUTS_ARRAY[i].attach(pin);
      cmd_code = strtok(NULL, ":");
      i++;
    }
    for (int j=i; j<MAX_NR_DIGOUTPUTS; j++){
      DIGOUTPUTS_ARRAY[j].attach(-1);
    }
  }

  if (strcmp(cmd_code,"DIGOUT") == 0) {
    //Serial.println("");
    //Serial.println("Set digital out pin...");
    cmd_code = strtok(NULL, ":");
    int i = 0;
    int pin = -2;
    int value = 0;
    while( cmd_code != NULL ){
      //Serial.print("cmd chunk, pin: ");
      //Serial.println(cmd_code);
      if(i == 0){
        pin = atoi(cmd_code);
      } if (i == 1){
        value = atoi(cmd_code);
      }
      cmd_code = strtok(NULL, ":");
      i++;
    }
    for (int i=0; i<MAX_NR_DIGOUTPUTS; i++){
      if(DIGOUTPUTS_ARRAY[i].getPin() == pin){
        //Serial.println("DIG OUTPUT updating output value on pin: " + (String)pin + " value: " + value);
        DIGOUTPUTS_ARRAY[i].updateValue(value);
      }
    }
  }
  
  if (strcmp(cmd_code,"SET_ID") == 0) {
    //Serial.println("");
    //Serial.println("Set device ID...");
    cmd_code = strtok(NULL, ":");
    int i = 0;
    while( cmd_code != NULL ){
      //Serial.print("cmd chunk, device ID: ");
      //Serial.println(cmd_code);
      int devID = strtol(cmd_code,0,0);
      EEPROM.put(0x00, devID);
      cmd_code = strtok(NULL, ":");
      i++;
    }
    //deviceID = devID;
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
