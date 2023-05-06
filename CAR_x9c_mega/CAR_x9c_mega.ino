#include <SD.h>         // Include the SD library
#include <TMRpcm.h>     // Include the TMRpcm library
#include <LapX9C10X.h>
#include <IBusBM.h>   
IBusBM ibus;
TMRpcm audio;           // Create an object of TMRpcm class

#define CSPIN_L 34 //2
#define INCPIN_L 32 //3
#define UDPIN_L 30 //4
#define rev_L 5


#define CSPIN_R 44 //10
#define INCPIN_R 42 //9
#define UDPIN_R 40 //8
#define rev_R 6
/*
#define gearGND 50
#define gearSW 52

#define keyStartPIN 31*/
#define startBladePIN 7

int ch_L_Value;
int ch_L_Val;
uint8_t r_L_Value;
int ch_R_Value;
int ch_R_Val;
uint8_t r_R_Value;
bool startMotorControllerBox=false; //เก็บไว้ยังไม่ได้ใช้ อาจจะเอาออก
int str0,str1,str2,str3;
bool bladeStatusON=false;
bool greetingPlaied=false; 


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
  //pinMode(keyStartPIN,OUTPUT);
  pinMode(startBladePIN,OUTPUT);
  //pinMode(gearGND,OUTPUT);
  //pinMode(gearSW,OUTPUT);

  digitalWrite(rev_L,HIGH);
  digitalWrite(rev_R,HIGH);
  //digitalWrite(keyStartPIN,HIGH);
  digitalWrite(startBladePIN,HIGH);
  //digitalWrite(gearGND,HIGH);
  //digitalWrite(gearSW,HIGH);

  
  audio.speakerPin = 46; // Set the output pin for audio  สำหรับ mega ต้องขา 46 จึงจะมีเสียง
  audio.setVolume(5);   // Set the volume to a value between 0 and 7  ต้องกำหนดที่ 5 เท่านั้นจึงจะมีเสียงอาจจะเป็นที่สเปคลำโพง
  if (!SD.begin(53)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");  
    return;   // don't do anything more if not
  }
  wavPlay("greeting.wav",true);
}

bool startOrder(){
    str0 = readChannel(0, -60, 60, 0);
    str1 = readChannel(1, -60, 60, 0);
    str2 = readChannel(2, -60, 60, 0);
    str3 = readChannel(3, -60, 60, 0);
    if(str0<=-58 && str1<=2 && str2<=-58 && str3>=58){
      return true;
    }else{
      return false;
    }
}

void wavPlay(char* wavfile, bool wait){
  audio.play(wavfile); // Play the audio file from the SD card

  int playTime=0;
  while (audio.isPlaying() && wait){
    if(playTime==0){
      Serial.print("playing..");
      delay(250);
    }
    playTime=playTime+1;
    Serial.print(".");
    delay(250);
  };// Wait for the audio to finish playing
  Serial.println(" ");
}

void loop() {
  
  bool startEngine=true;
  if(!startMotorControllerBox){
    if(!readSwitch(5, false) && !readSwitch(6, false)){  //vrB ถ้าไม่เปิดใบตัดไว้(flase) หรือ swA ถ้าไม่ปิดฉุกเฉิน(flase)ไว้
      if(startOrder()){
        startEngine=false;  
        //wavPlay("3.wav",false);
        Serial.println("3");
        if(startOrder()){
          //wavPlay("2.wav",false);
          Serial.println("2");
          if(startOrder()){
            //wavPlay("1.wav",false);
            Serial.println("1");
            Serial.println("Wait for start 5 sec");
            //wavPlay("carready.wav",true); //รถพร้อมทำงาน และกำลังเคลื่อนตัว กรุณาถอยออกห่าง
            Serial.println("Ready....");
            startMotorControllerBox=true; //เก็บไว้ยังไม่ได้ใช้ อาจจะเอาออก
            startEngine=true;
            //digitalWrite(keyStartPIN,LOW);
          }
        }
      }
      if(!startEngine){
        Serial.println("Starting Cancelled");
        //wavPlay("cancelstarting.wav",true);
      }
    }else{
      Serial.println("Blade still ON or Emergency Sw OFF, don't start.");
      //wavPlay("bladeondontstart.wav",true);
    }
  }else{
    if(readSwitch(6,false)){
      //Serial.println("sw7 is true");
      startMotorControllerBox=false; //เก็บไว้ยังไม่ได้ใช้ อาจจะเอาออก
      //digitalWrite(keyStartPIN,HIGH);
      startEngine=false;
      Serial.println("Emergency Engine Stoped");
      //wavPlay("stopengine.wav",true);

      digitalWrite(startBladePIN,HIGH);
      bladeStatusON = false;
      Serial.println("Emergency Blade Stoped");
      //wavPlay("stopblade.wav",true);
    }

    if(readSwitch(5, false) && !bladeStatusON && !readSwitch(6,false)){  //เปิดสวิทช์ และใบตัดไม่ได้ทำงานอยู่ และ swA ต้อง true ด้วย
      digitalWrite(startBladePIN,LOW);
      Serial.println("bladeStared");
      bladeStatusON = true;
      //wavPlay("bladestarting.wav",false); //ใบตัดกำลังเริ่มทำงานใน 
    }else if(!readSwitch(5, false) && bladeStatusON){  //ปิดสวิทช์ และใบตัดทำงานอยู่
      digitalWrite(startBladePIN,HIGH);
      Serial.println("bladeStoped");
      bladeStatusON = false;
      //wavPlay("bladestop.wav",false); //ใบตัดหยุดทำงาน 
    }
        

    ch_L_Value = readChannel(0, -60, 60, 0);
    Serial.print("L = "); Serial.println(ch_L_Value);

    ch_R_Value = readChannel(1, -60, 60, 0);
    Serial.print("R = "); Serial.println(ch_R_Value);

    
    if(ch_L_Value<0){
      r_L_Value = 60+ch_L_Value;      
      digitalWrite(rev_L,LOW);
      Serial.println("L Backward");
    }else{
      r_L_Value = 60-ch_L_Value;
      digitalWrite(rev_L,HIGH);
      Serial.println("L Forward");
    }

    vr_L.set(r_L_Value);
    
    if(ch_R_Value<0){
      r_R_Value = 60+ch_R_Value;
      digitalWrite(rev_R,LOW);
      Serial.println("R Backward");
    }else{
      r_R_Value = 60-ch_R_Value;
      digitalWrite(rev_R,HIGH);
      Serial.println("R Forward");
    }

    vr_R.set(r_R_Value);
    
    delay(100);
  }
}
