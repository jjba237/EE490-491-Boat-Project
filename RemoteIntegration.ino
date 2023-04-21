#include <LiquidCrystal.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//Define CE and CSN pins for tranciver and joystick pins
#define CE_PIN   8
#define CSN_PIN 9
#define joyLvoltage A0
#define joyRvoltage A1

//Pipe addresses for sending and recieivng
const byte remoteAddress[5] = {'S','B','S','R','X'};
const byte boatAddress[5] = {'S','B','S','T','X'};

// Initializing pins for radio and pins for LCD
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
LiquidCrystal lcd(7, 6, 5, 4, 3, 2); // RS, E, D4, D5, D6, D7

//Initializing temp,joy, and button variables
float temps[7]; // must match dataToSend in master
int joyL, joyR, joyLMap, joyRMap;
int state = 0;
int button = 0;
int switchstate = 0;
int onOff = 0;

void setup() {
  Serial.begin(9600); //baud rate
  radio.begin();
  radio.setDataRate( RF24_250KBPS );//data rate
  radio.setPALevel(RF24_PA_LOW);//power level output
  radio.openWritingPipe(boatAddress);//send to boat pipe
  radio.openReadingPipe(1, remoteAddress);//receive from remote pipe
  radio.setRetries(3,5); // time delay in ms, retry count (IMPORTANT)
  radio.startListening();//turn on recieving mode
  lcd.begin(20, 4);//set LCD dimentions
  pinMode(A4, INPUT);//use analog pin for digital button input
  pinMode(A3, INPUT);//use analog pin for digital kill switch input
}

//====================

void loop() {
  getData();
  send();
  //delay(100);
}

//====================

void send() {
  //read in voltage from joystick potentiometer and map to 8-bit number for UART coms
  //if the kill switch is off, if on, send default values
  int joyData[2] = {joyLMap, joyRMap};
  onOff = digitalRead(A3); //read in switch
  if (onOff < 1){
    //Serial.println("On");
    joyL = analogRead(joyLvoltage);
    joyR = analogRead(joyRvoltage);
    joyLMap = map(joyL, 0,1023, -126, 127); //0-126
    joyRMap = map(joyR,0,1023,2,256); //128-254

    if (joyLMap == -1 || joyLMap == 1){
      joyLMap = 0;
    }
    if (joyLMap < 0){
      joyLMap = 0;
    }
    if (joyRMap == 127 || joyRMap == 129){
      joyRMap = 128;
    }
    if (joyRMap < 128){
      joyRMap = 128;
    }
  }
  else{
    //Serial.println("Off");
    joyLMap = 0;
    joyRMap =128;
  }
  
   

  Serial.print("Joystick L: ");
  Serial.print(joyLMap);
  Serial.print(", Joystick R: ");
  Serial.println(joyRMap);

  radio.stopListening();//turn off recieving mode
  radio.write( &joyData, sizeof(joyData) );//send joy data to boat
  radio.startListening();//turn on recieiving mode

  Serial.print("Joystick L: ");
  Serial.print(joyData[0]);
  Serial.print(", Joystick R: ");
  Serial.println(joyData[1]);
}

//================

void getData() {
  if ( radio.available() ) {  //if data is available from boat
    radio.read( &temps, sizeof(temps) ); //read in temp data from boat
    button = digitalRead(A4);//button voltage (0 or 1)  
    if (button < 1){ //if the button is pressed, progress to the next state.
      state = state +1; 
      //delay(400);
        if (state > 1){ //if the state is greater than 2, reset to 0
          state = 0;
        }
    }
    switch(state) {
      case 0: //state 1, display temps on LCD
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("   Temperatures");
        lcd.setCursor(0, 1);
        lcd.print("T1:");
        lcd.print((int)temps[0]);
        lcd.print(" F ");
        lcd.setCursor(9, 1);
        lcd.print("T2:");
        lcd.print((int)temps[1]);
        lcd.print(" F ");
        lcd.setCursor(0, 2);
        lcd.print("T3 ");
        lcd.print((int)temps[2]);
        lcd.print(" F ");
        lcd.setCursor(9, 2);
        lcd.print("T4:");
        lcd.print((int)temps[3]);
        lcd.print(" F ");
        lcd.setCursor(0, 3);
        lcd.print("T5:");
        lcd.print((int)temps[4]);
        lcd.print(" F ");
        lcd.setCursor(9, 3);
        lcd.print("T6:");
        lcd.print((int)temps[5]);
        lcd.print(" F ");
        break;
      case 1: //state 2, display voltages on LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Batteries n Parallel");
        lcd.setCursor(5, 2);
        lcd.print(temps[6]);
        lcd.setCursor(9, 2);
        lcd.print(" Volts");        
        break;
    }
  }
}