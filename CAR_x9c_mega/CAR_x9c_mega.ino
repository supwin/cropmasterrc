#include <SD.h>         // Include the SD library
#include <TMRpcm.h>     // Include the TMRpcm library
#include <IBusBM.h>   
IBusBM ibus;
TMRpcm audio;           // Create an object of TMRpcm class

#define rev_L 5
#define rev_R 6
#define pin_L 3
#define pin_R 9
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
bool startEngine=true;

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

  pinMode(rev_L, OUTPUT);
  pinMode(rev_R, OUTPUT);
  pinMode(pin_L, OUTPUT);
  pinMode(pin_R, OUTPUT);
  //pinMode(keyStartPIN,OUTPUT);
  pinMode(startBladePIN,OUTPUT);
  //pinMode(gearGND,OUTPUT);
  //pinMode(gearSW,OUTPUT);

  digitalWrite(rev_L,HIGH);
  digitalWrite(rev_R,HIGH);
  digitalWrite(pin_L,HIGH);
  digitalWrite(pin_R,HIGH);
  analogWrite(pin_L,75);
  analogWrite(pin_R,75);
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
  wavPlay("greeting.wav",true,"Crop Master RC รถบังคับเพื่องานการเกษตร สวัสดีครับ");
  readSubVersion("0p020000p1");
}

void readSubVersion(char* txt){
  wavPlay("alfavrsn.wav",true,"อัลฟ่าเวอร์ชั่น");
  for(int i = 0; i < strlen(txt); i++) {
    char txtchar = txt[i];
    String filename = String(txtchar) + ".wav";
    wavPlay(filename.c_str(), true, &txtchar);
  }
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

void wavPlay(char* wavfile, bool wait, char* txt){
  audio.play(wavfile); // Play the audio file from the SD card

  Serial.print(txt);
  while (audio.isPlaying() && wait){
    Serial.print(".");
    delay(250);
  };// Wait for the audio to finish playing
  Serial.println(".");
}

void emergencyStop(){
  startMotorControllerBox=false; //เก็บไว้ยังไม่ได้ใช้ อาจจะเอาออก
  //digitalWrite(keyStartPIN,HIGH);
  digitalWrite(startBladePIN,HIGH);

  startEngine=false;  //สถานะเครื่องหยุดทำงาน 
  bladeStatusON = false;

  wavPlay("stpeng.wav",true,"ดับเครื่อง");
  wavPlay("stpbld.wav",true,"หยุดใบตัด");
  int zTime = 0;
  while(readSwitch(6,false)){
    zTime++;
    if(zTime>9){
      zTime=0;
      Serial.println("Z");
    }else{
      Serial.print("z");
    }
    delay(1000);
  }
}

void cancelBladStart(){
    wavPlay("ccblstat.wav",true,"ยกเลิก ใบตัด");
}


void loop(){
  startEngine=true;
  if(!startMotorControllerBox){
    if(!readSwitch(5, false) && !readSwitch(6, false)){  //vrB ถ้าไม่เปิดใบตัดไว้(flase) หรือ swA ถ้าไม่ปิดฉุกเฉิน(flase)ไว้
      if(startOrder()){
        startEngine=false;  
        wavPlay("3.wav",false,"3");
        delay(1000);
        if(startOrder()){
          wavPlay("2.wav",false,"2");
          delay(1000);
          if(startOrder()){
            wavPlay("1.wav",false,"1");
            delay(1000);
            wavPlay("carready.wav",true,"รถพร้อมทำงาน และกำลังเคลื่อนตัว กรุณาถอยออกห่าง");
            Serial.println("Ready....");
            startMotorControllerBox=true; //เก็บไว้ยังไม่ได้ใช้ อาจจะเอาออก
            startEngine=true;  //สถานะพร้อมรับคำสั่ง อันตรายถ้าอยู่ใกล้
            //digitalWrite(keyStartPIN,LOW);
          }
        }
      }
      if(!startEngine){
        wavPlay("ccelst.wav",true,"ยกเลิกการสต๊าส");
      }
    }else{
      if(readSwitch(5, false)) wavPlay("blondstr.wav",true,"สวิทช์ใบตัดเปิดค้างอยู่ ห้ามสต๊าส");
      if(readSwitch(6, false)) wavPlay("enofdstr.wav",true,"สวิทช์เครื่องปิดอยู่ ไม่สามารถสต๊าสได้");
    }
  }else{
    // หยุดเครื่องฉุกเฉินจะต้อง upgrade เป็นเคส interup ภายหลัง
    if(readSwitch(6,false)){ //ถ้าสถานะ sw7 เป็น true 
      emergencyStop();
      return;  // ออกไปเริ่ม loop ใหม่ ไม่ทำคำสั่งที่เหลือต่อไป
    }

    if(readSwitch(5, false) && !bladeStatusON && !readSwitch(6,false)){  //เปิดสวิทช์ และใบตัดไม่ได้ทำงานอยู่ และ swA ต้อง true ด้วย
      wavPlay("blstrt.wav",true,"ใบตัดกำลังเริ่มทำงานใน"); 
      wavPlay("3.wav",false,"3");
      delay(1000);
      if(!readSwitch(6, false)){
        if(readSwitch(5,false)){
          wavPlay("2.wav",false,"2");
          delay(1000);
        }else{
          cancelBladStart();
          return; // ออกไปเริ่ม loop ใหม่ ไม่ทำคำสั่งที่เหลือต่อไป
        }
      }else{
        emergencyStop();
        return; // ออกไปเริ่ม loop ใหม่ ไม่ทำคำสั่งที่เหลือต่อไป
      }
      
      if(!readSwitch(6, false)){
        if(readSwitch(5,false)){
          wavPlay("1.wav",false,"1");
          delay(1000);
        }else{
          cancelBladStart();
          return; // ออกไปเริ่ม loop ใหม่ ไม่ทำคำสั่งที่เหลือต่อไป
        }
      }else{
        emergencyStop();
        return; // ออกไปเริ่ม loop ใหม่ ไม่ทำคำสั่งที่เหลือต่อไป
      }
      
      digitalWrite(startBladePIN,LOW);
      Serial.println("bladeStared");
      bladeStatusON = true;
    }else if(!readSwitch(5, false) && bladeStatusON){  //ปิดสวิทช์ และใบตัดทำงานอยู่
      digitalWrite(startBladePIN,HIGH);
      Serial.println("bladeStoped");
      bladeStatusON = false;
      wavPlay("bldstop.wav",false,"ใบตัดหยุดทำงาน"); //ใบตัดหยุดทำงาน 
    }
        

    ch_L_Value = readChannel(0, -60, 60, 0);
    ch_R_Value = readChannel(1, -60, 60, 0);

    
    if(ch_L_Value<0){
      ch_L_Value = ch_L_Value*(-1);    
      digitalWrite(rev_L,LOW);
      Serial.print("L Backward "); 
    }else{
      digitalWrite(rev_L,HIGH);
      Serial.print("L Forward ");  
    }
    Serial.println(ch_L_Value);
    analogWrite(pin_L,map(ch_L_Value,0,60,75,130));
    
    if(ch_R_Value<0){
      ch_R_Value = ch_R_Value*(-1);  
      digitalWrite(rev_R,LOW);
      Serial.print("R Backward ");
    }else{
      digitalWrite(rev_R,HIGH);
      Serial.print("R Forward ");
    }
    Serial.println(ch_R_Value);
    analogWrite(pin_R,map(ch_R_Value,0,60,75,130));
    
    delay(250);
  }
}
