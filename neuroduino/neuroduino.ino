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

//
unsigned long msAtStart;
unsigned long msFromStart;

// DIGITAL PINS
int outputPins[] = {8,9,10,11};
int N_OUTPUT_PINS;
int photoresistorPin = 0;


// BUTTONS
const char* buttonColours[] = {"RED","BLUE","YELLOW","GREEN"};
const int buttonPins[] = {7, 6, 5, 4};
//ascii chars representing
const char buttonKeys[] = {'a','d','j','l'};

// PHOTORESISTOR
int photoresistorThresholdHigh = 500;
int photoresistorThresholdLow = 100;
int calibrationTime = 100;

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
    pinMode(buttonPins[i], OUTPUT);
  }
  
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
    if (connected) {
      ListenForOrders();
    }
  }
  if (connected){
    if (photoresistorUse) {
      PhotoresistorAction();
    }
    ButtonsAction();
  }
}

void LettingKnow() {
  float time = millis();
  while (true) {
    serialInput = Serial.readStringUntil(untilChar);
    if (serialInput == "DONE") {
      Connect();
      break;
    }
    Serial.println("NEURODUINO");
    if (millis() - time > 1000) {
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
  //Keyboard.end();
}

void Restart(){
  Disconnect();
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
    if (serialInput == "PHOTO+") {
      StartPhotoresistor();
      SendDone();
    }
    if (serialInput == "PHOTO-") {
      StopPhotoresistor();
      SendDone();
    }
    if (serialInput == "PHOTO-CAL-HIGH") {
      CalibratePhotoresistorHigh();
      SendDone();
    }
    if (serialInput == "PHOTO-CAL-LOW") {
      CalibratePhotoresistorLow();
      SendDone();
    }   
    if (serialInput == "RESTART") {
      Restart();
    }
  }
}

void SendDone(){
  unsigned long msSinceStart = millis() - msAtStart;
  char buf[14];
  sprintf(buf,"DONE%lu", msSinceStart);
  Serial.println(buf);
}

void Blink(){
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
}

void StartPhotoresistor(){
  photoresistorUse = true;
}

void StopPhotoresistor(){
  photoresistorUse = false;
}

void CalibratePhotoresistorHigh(){
  unsigned long endTime = millis() + calibrationTime;
  photoresistorThresholdHigh = analogRead(photoresistorPin);
  while(millis() > endTime){
    int newValue = analogRead(photoresistorPin);
    if(newValue > photoresistorThresholdHigh){
      photoresistorThresholdHigh = newValue;
    }
  }
}

void CalibratePhotoresistorLow(){
  unsigned long endTime = millis() + calibrationTime;
  photoresistorThresholdLow = analogRead(photoresistorPin);
  while(millis() > endTime){
    int newValue = analogRead(photoresistorPin);
    if(newValue < photoresistorThresholdHigh){
      photoresistorThresholdLow = newValue;
    }
  }
}

void PhotoresistorAction(){
  //what is this doing here???
  if(digitalRead(7) == LOW){
    Serial.println(analogRead(photoresistorPin));
    static bool alreadyReacted = false;
    if (analogRead(photoresistorPin) > photoresistorThresholdHigh) {
        if(!alreadyReacted){
          alreadyReacted = true;
        }
    } else {
      alreadyReacted = false;
    }
  }
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
