#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
//Define CE and CSN pins for tranciver 
#define CE_PIN   48
#define CSN_PIN 49

//Pipe addresses for sending and recieivng
const byte remoteAddress[5] = {'S','B','S','R','X'};
const byte boatAddress[5] = {'S','B','S','T','X'};

// Initializing pins for radio
RF24 radio(CE_PIN, CSN_PIN); 

//Initializing temp and joy variables
float T1, T2, T3, T4, T5, T6, V1;
int joyL, joyR;
int joyData[2];

//============

void setup() {
  //Serial.begin(9600); //(CHANGE WHEN SERIAL INTERFACE is Arduino IDE)
  Serial.begin(115200, SERIAL_8E1); //baud rate (CHANGE WHEN SERIAL INTERFACE is STM32)
  radio.begin();
  radio.setDataRate( RF24_250KBPS ); // data rate
  radio.setPALevel(RF24_PA_LOW); // power level output
  radio.openWritingPipe(remoteAddress);//send to remote pipe
  radio.openReadingPipe(1, boatAddress);//recieve from boat pipe
  radio.setRetries(2,5); // time delay in ms, retry count (IMPORTANT)
  send();
}
//=============

void loop() {
  send();
  getData();
  delay(250); //Gives radios enough time to be in same state (IMPORTANT)
}

//====================

void send() {
  //Get all 6 temps
  T1 = getTemp(0);
  T2 = getTemp(1);
  T3 = getTemp(2);
  T4 = getTemp(3);
  T5 = getTemp(4);
  T6 = getTemp(5);
  V1 = getV(); 
   
  //Throw temps in an array for sending
  float temps[7] = {T1, T2, T3, T4, T5, T6, V1};

  // Serial.print("T1: ");
  // Serial.print(temps[0], 2);
  // Serial.print(" F, T2: ");
  // Serial.print(temps[1], 2);
  // Serial.print(" F, T3: ");
  // Serial.print(temps[2], 2);
  // Serial.print(" F, T4: ");
  // Serial.print(temps[3], 2);
  // Serial.print(" F, T5: ");
  // Serial.print(temps[4], 2);
  // Serial.print(" F, T6: ");
  // Serial.print(temps[5], 2);
  // Serial.println(" F");

  radio.stopListening(); //Turn off recieving mode
  radio.write( &temps, sizeof(temps) ); //send temps to remote control
  radio.startListening();//Turn on recieiving mode


  //send to STM32 (CHANGE WHEN SERIAL INTERFACE is STM32)
  Serial.write((int) joyL);
  Serial.write((int) joyR);
}

//================

void getData() {
  if ( radio.available() ) { //if there is data sent from remote
    radio.read( &joyData, sizeof(joyData) ); //read in joystick data
    joyL = joyData[0];
    joyR = joyData[1];
    //(CHANGE FOR TESTING in ARduino IDE Serial)
    // Serial.print("Joystick L: ");
    // Serial.print(joyL);
    // Serial.print(", Joystick R: ");
    // Serial.println(joyR);
  }

}

//================

//function for converting thermistor voltage to fahrenheight tempurature
float getTemp(int thermistorPin) {
  float temperature;
  int rawADC = analogRead(thermistorPin);
  temperature = log(10000.0*((1024.0/rawADC-1)));
  temperature = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temperature * temperature ))* temperature );
  temperature = temperature - 273.15;
  temperature = (temperature * 9.0) / 5.0 + 32.0; // convert to Fahrenheit
  return temperature;
}

float getV() {
  int raw = analogRead(A10);
  float voltage = raw * (5.0 / 1024.0);
  float realVoltage = (0.96*122.0*voltage)/(22.0);
  //((22000)/(100000+22000)));
  return realVoltage;
  //Serial.print("Battery Voltage = ");
  //Serial.println(realVoltage);
}