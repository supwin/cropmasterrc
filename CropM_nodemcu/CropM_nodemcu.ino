#include <ESP8266SD.h>      // Include the ESP8266SD library
#include <ESP8266Audio.h>   // Include the ESP8266Audio library

AudioGeneratorWAV audio;     // Create an object of AudioGeneratorWAV class
AudioFileSourceSD file;      // Create an object of AudioFileSourceSD class

#define CSPIN_L D2 //2
#define INCPIN_L D3 //3
#define UDPIN_L D5 //4
#define rev_L D6


#define CSPIN_R D7 //10
#define INCPIN_R D8 //9
#define UDPIN_R D0 //8
#define rev_R D1

#define startBladePIN D4

int ch_L_Value;
int ch_L_Val;
uint8_t r_L_Value;
int ch_R_Value;
int ch_R_Val;
uint8_t r_R_Value;
bool startMotorControllerBox=false;
int str0,str1,str2,str3;
bool bladeStatusON=false;
bool greetingPlaied=false; 
bool startEngine=true;


LapX9C10X vr_L(INCPIN_L, UDPIN_L, CSPIN_L, LAPX9C10X_X9C103);
LapX9C10X vr_R(INCPIN_R, UDPIN_R, CSPIN_R, LAPX9C10X_X9C103);

int readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue) {
  uint16_t ch = ibus.readChannel(channelInput);
  if (ch < 100) return defaultValue;
  return map(ch, 1000, 2000, minLimit, maxLimit);
}

bool readSwitch(byte channelInput, bool defaultValue) {
  int intDefaultValue = (defaultValue) ? 100 : 0;
  int ch = readChannel(channelInput, 0, 100, intDefaultValue);
  return (ch > 50);
}

void setup() {
  Serial.begin(9600);
  ibus.begin(Serial1);
  Serial.println("Starting...");  
  vr_L.begin(99);
  vr_R.begin(99);

  pinMode(rev_L, OUTPUT);
  pinMode(rev_R, OUTPUT);
  pinMode(startBladePIN,OUTPUT);

  digitalWrite(rev_L,HIGH);
  digitalWrite(rev_R,HIGH);
  digitalWrite(startBladePIN,HIGH);

  audio.speakerPin = D10; // Set the output pin for audio
  audio.setVolume(5);   // Set the volume to a value between 0 and 7
  if (!SD.begin(SS)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");  
    return;   // don't do anything more if not
  }
  wavPlay("greeting.wav",true,"Crop Master RC รถบังคับเพื่องานการเกษตร สวัสดีครับ");
}

void loop() {
  if(startOrder()){
    if(startEngine){
      startMotorControllerBox=true; //เก็บไว้ยังไม่ได้ใช้ อาจจะเอาออก
      digitalWrite(startBladePIN,LOW);
      wavPlay("startEngine.wav",false,"เริ่มเครื่องยนต์");
      delay(2000);
      //digitalWrite(keyStartPIN,LOW);
      bladeStatusON=true;
      startEngine=false;
    }else{
      if(bladeStatusON){
        wavPlay("bladeOn.wav",false,"เ




