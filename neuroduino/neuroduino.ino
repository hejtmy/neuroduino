/*
  Name:		neuroduino.ino
  Created:	4/20/2002 6:43:19 PM
  Author:	Lukáš 'hejtmy' Hejtmánek
*/

//#include <Keyboard.h>
#define ARRAY_SIZE(array) ((sizeof(array))/(sizeof(array[0])))

// Class state
String serialInput;
int timeout = 25;
bool connected = false;
bool photoresistorUse = false;
bool pulsing = false;
int speed = 50; //speed for the delay factor
int lettingKnowTime = 1000; //duration of how long will we keep the connection trying

//
unsigned long msAtStart;
unsigned long msFromStart;

// DIGITAL PINS
int outputPins[] = {8,9,10,11};
int N_OUTPUT_PINS;


// BUTTONS
const char* buttonColours[] = {"RED","BLUE","YELLOW","GREEN"};
const int buttonPins[] = {7, 6, 5, 4};
//ascii chars representing
const char buttonKeys[] = {'a','d','j','l'};

// PHOTORESISTOR
int photoresistorPin = 0;
int photoresistorThreshold = 500;
int photoTresholdMargin = 100;
int photoCalibrationTime = 200;
bool photoActionSent = false;
bool photoSendData = false;
int photoTimeSent = 0;
int photoSendingDelay = 200;

char untilChar = '\!';

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  N_OUTPUT_PINS = ARRAY_SIZE(outputPins);
  Serial.setTimeout(timeout);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  //initialise buttons
  for(int i = 0; i < ARRAY_SIZE(buttonPins); i++){
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  for(int i = 0; i < ARRAY_SIZE(outputPins); i++){
    pinMode(outputPins[i], OUTPUT);
  }
  pinMode(photoresistorPin, INPUT);
}

// the loop function runs over and over again until power down or reset
void loop() {
  serialInput = Serial.readStringUntil(untilChar);
  if (serialInput != "") {
    if (serialInput == "WHO") {
      LettingKnow();
    }
    if (serialInput == "DISCONNECT") {
      Disconnect();
    }
    if (!connected){
      return;
    }
    ListenForOrders();
  }
  if (!connected){
    return;
  }
  if (photoresistorUse) {
    PhotoresistorAction();
  }
  if(photoSendData){
    int t = millis();
    if(t - photoTimeSent > photoSendingDelay){
      photoTimeSent = t;
      SendPhotoData();
    }
  }
  ButtonsAction();
}

void LettingKnow() {
  unsigned long t = millis();
  while (true) {
    serialInput = Serial.readStringUntil(untilChar);
    if (serialInput == "DONE") {
      Connect();
      break;
    }
    Serial.println("NEURODUINO");
    if (millis() - t > lettingKnowTime) {
      Serial.println("TIME IS UP");
      break;
    }
    delay(speed);
  }
}

void Connect(){
  //Keyboard.begin();
  msAtStart = millis();
  connected = true;
}

void Disconnect(){
  connected = false;
  Restart();
  //Keyboard.end();
}

void Restart(){
  photoSendData = false;
  photoresistorUse = false;
  photoTimeSent = 0;
  CancelPulse();
}

void ListenForOrders() {
  if (serialInput != "") {
    if(serialInput.substring(0,6) == "PULSE+"){ 
      StartPulse(serialInput);
      SendDone();
    }
    if (serialInput == "PULSE-") {
      CancelPulse();
      SendDone();
    }
    if (serialInput == "BLINK") {
      Blink();
      SendDone();
    }
    if (serialInput == "PHOTO-START") {
      photoresistorUse = true;
      SendDone();
    }
    if (serialInput == "PHOTO-END") {
      photoresistorUse = false;
      SendDone();
    }
    if (serialInput == "PHOTO-DATA+") {
      photoSendData = true;
    }
    if (serialInput == "PHOTO-DATA-") {
      photoSendData = false;
    }
    if (serialInput == "PHOTO-CALIBRATE") {
      CalibratePhotoresistor();
      SendDone();
    }
    if (serialInput == "RESTART") {
      Restart();
    }
  }
}

unsigned long GetTime(){
  unsigned long msSinceStart = millis() - msAtStart;
  return msSinceStart;
}

void SendDone(){
  unsigned long msSinceStart = GetTime();
  char buf[14];
  sprintf(buf,"DONE%lu", msSinceStart);
  Serial.println(buf);
}

void Blink(){
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
}

void CalibratePhotoresistor(){
  unsigned long endTime = millis() + photoCalibrationTime;
  photoresistorThreshold = analogRead(photoresistorPin);
  while(millis() > endTime){
    int newValue = analogRead(photoresistorPin);
    if(newValue > photoresistorThreshold){
      photoresistorThreshold = newValue;
    }
  }
}

void PhotoresistorAction(){
  static bool photoHigh = false;
  if (analogRead(photoresistorPin) > photoresistorThreshold && !photoHigh) {
      char buf[20];
      sprintf(buf,"PHOTO-HIGH-%lu", GetTime());
      Serial.println(buf);
      photoHigh = true;
  }
  if (analogRead(photoresistorPin) < (photoresistorThreshold - photoTresholdMargin) && photoHigh) {
      char buf[20];
      sprintf(buf,"PHOTO-LOW-%lu", GetTime());
      Serial.println(buf);
      photoHigh = false;
  }
}

void SendPhotoData(){
  char buf[20];
  sprintf(buf, "PHOTO-%d", analogRead(photoresistorPin));
  Serial.println(buf);
}

void ButtonsAction(){
  for(int i=0; i < ARRAY_SIZE(buttonPins); i++){
    if(digitalRead(buttonPins[i]) == LOW){
      //Keyboard.write(buttonKeys[i]);
      Serial.println(buttonColours[i]);
    }
  }
}

void StartPulse(String inputString){
  pulsing = true; // no fuctionality yet
  // The signal comes as PULSE+0101 where the 
  // 0 an 1 determine pins at given positions to be on or off
  String activePins = inputString.substring(7,11);
  char buf[N_OUTPUT_PINS];
  activePins.toCharArray(buf, N_OUTPUT_PINS);
  for(int i = 0; i < N_OUTPUT_PINS; i++){
    if(buf[i] == "1"){
      digitalWrite(outputPins[i], HIGH);
    } else {
      digitalWrite(outputPins[i], LOW);
    }
  }
}

void CancelPulse(){
  pulsing = false; // no fuctionality yet
  digitalWrite(outputPins[0], LOW);
}
