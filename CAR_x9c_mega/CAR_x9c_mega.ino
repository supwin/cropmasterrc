#include <SD.h>         // Include the SD library
#include <TMRpcm.h>     // Include the TMRpcm library
#include <IBusBM.h>   
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

/*
version explaination
0 : untest version
1 : tested version 
2 : alpha version
3 : beta version
4 : candidate version
5 : release version
*/
int versionSerial="0";
char* versionNo = "0p0700";

int slowest=55;
int middle=78;
int fastest=100;
int ch_1;
int ch_2;
int str0,str1,str2,str3;
bool bladeStatusON=false;
bool startEngine=false;
bool bladeStopTemp=false;
float lat;
float lng;

// Struct to represent a waypoint
struct Waypoint {
  double latitude;
  double longitude;
};
Waypoint waypoints[10];

double currentLatitude;
double currentLongitude;
double currentHeading;
double currentDistance; 
double currentBearing;

// Initialize compass
//Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified();
Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345);

IBusBM ibus;
TMRpcm audio; // Create an object of TMRpcm class
TinyGPSPlus gps;  // The TinyGPS++ object

// Function to convert degrees to radians
double toRadians(double degrees) {
  return degrees * 3.14159 / 180.0;
}

#define pin_Amp 2
static const int gpsRXPin = 4, gpsTXPin = 3;
SoftwareSerial gpsSerial(gpsRXPin, gpsTXPin); // The serial connection to the GPS device
static const uint32_t GPSBaud = 9600;
#define rev_L 6
#define rev_R 7
#define pin_L 9
#define pin_R 10

/*
#define gearGND 50
#define gearSW 52
#define keyStartPIN 31*/
#define startBladePIN 7

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(GPSBaud);
  ibus.begin(Serial1);
  Serial.println("Starting...");  
  TCCR2B = TCCR2B & B11111000 | B00000001;
  //InitTimersSafe();
  
  pinMode(rev_L, OUTPUT);
  pinMode(rev_R, OUTPUT);
  pinMode(pin_L, OUTPUT);
  pinMode(pin_R, OUTPUT);
  //pinMode(keyStartPIN,OUTPUT);
  pinMode(startBladePIN,OUTPUT);
  //pinMode(gearGND,OUTPUT);
  //pinMode(gearSW,OUTPUT);
  pinMode(pin_Amp,OUTPUT);

  digitalWrite(rev_L,HIGH);
  digitalWrite(rev_R,HIGH);
  digitalWrite(pin_L,HIGH);
  digitalWrite(pin_R,HIGH);
  analogWrite(pin_L,slowest);
  analogWrite(pin_R,slowest);
  // pwmWrite(pin_L, 0);
  // pwmWrite(pin_R, 0);
  //digitalWrite(keyStartPIN,HIGH);
  digitalWrite(startBladePIN,HIGH);
  //digitalWrite(gearGND,HIGH);
  //digitalWrite(gearSW,HIGH);
  digitalWrite(pin_L,HIGH);

  audio.speakerPin = 46; // Set the output pin for audio  สำหรับ mega ต้องขา 46 จึงจะมีเสียง
  audio.setVolume(5);   // Set the volume to a value between 0 and 7  ต้องกำหนดที่ 5 เท่านั้นจึงจะมีเสียงอาจจะเป็นที่สเปคลำโพง
  if (!SD.begin(53)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");  
    return;   // don't do anything more if not
  }
  wavPlay("greeting.wav",true,"Crop Master RC รถบังคับเพื่องานการเกษตร สวัสดีครับ");
  readSubVersion(versionSerial,versionNo);
}

void readSubVersion(int versionSerial,char* versionNo){
      // version explaination
      // 0 : untest version
      // 1 : tested version 
      // 2 : alpha version
      // 3 : beta version
      // 4 : candidate version
      // 5 : release version
  char* versionfilename;
  char* versiontxt;
  switch (versionSerial){
    case 0:
      versionfilename = "untestversion.wav";
      versiontxt = "เวอร์ชั่นยังไม่ทดสอบ";
      break;
    case 1:
      versionfilename = "testedversion.wav";
      versiontxt = "เวอร์ชั่นทดสอบแล้ว";
      break;
    case 2:
      versionfilename = "alphaversion.wav";
      versiontxt = "อัลฟ่าเวอร์ชั่น";
      break;
    case 3:
      versionfilename = "";
      break;
    case 4:
      versionfilename = "";
      break;
    case 5:
      versionfilename = "";
  }
      
  wavPlay(versionfilename,true,versiontxt);
  for(int i = 0; i < strlen(versionNo); i++) {
    char txtchar = versionNo[i];
    String filename = String(txtchar) + ".wav";
    wavPlay(filename.c_str(), true, &txtchar);
  }
}

bool startOrder(){
    str0 = readChannel(0, -5, 5, 0);
    str1 = readChannel(1, -5, 5, 0);
    str2 = readChannel(2, -5, 5, 0);
    str3 = readChannel(3, -5, 5, 0);
    if(str0<=-4 && str1<=2 && str2<=-4 && str3>=4){
      return true;
    }else{
      return false;
    }
}

void wavPlay(char* wavfile, bool wait, char* txt){
  digitalWrite(pin_Amp,LOW);
  audio.play(wavfile); // Play the audio file from the SD card

  Serial.print(txt);
  while (audio.isPlaying() && wait){
    Serial.print(".");
    delay(250);
  };// Wait for the audio to finish playing
  digitalWrite(pin_Amp,HIGH);
  Serial.println(".");
}

void emergencyStop(){
  // สั่งหยุดเคลื่อนตัวและ reset relay เข้าเกียร์เดินหน้าไว้
  analogWrite(pin_L,slowest); 
  analogWrite(pin_R,slowest);
  digitalWrite(rev_L,HIGH);
  digitalWrite(rev_R,HIGH);
  //startMotorControllerBox=false; //เก็บไว้ยังไม่ได้ใช้ อาจจะเอาออก
  //digitalWrite(keyStartPIN,HIGH);
  digitalWrite(startBladePIN,HIGH);

  startEngine=false;  //สถานะเครื่องหยุดทำงาน 
  bladeStatusON = false;

  wavPlay("stpeng.wav",true,"ดับเครื่อง");
  wavPlay("stpbld.wav",true,"หยุดใบตัด");
  int zTime = 0;
  while(readSwitch(7,false)){
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

void cancelBladeStart(){
    wavPlay("ccblstat.wav",true,"ยกเลิก ใบตัด");
}

bool startSystem(){
  if(!readSwitch(5, false) && !readSwitch(7, false)){  //vrB ถ้าไม่เปิดใบตัดไว้(flase) หรือ swA ถ้าไม่ปิดฉุกเฉิน(flase)ไว้
    if(startOrder()){ 
      wavPlay("count3.wav",false,"3");
      delay(1000);
      if(startOrder()){
        wavPlay("count2.wav",false,"2");
        delay(1000);
        if(startOrder()){
          wavPlay("count1.wav",false,"1");
          delay(1000);
          wavPlay("carready.wav",true,"รถพร้อมทำงาน และกำลังเคลื่อนตัว กรุณาถอยออกห่าง");
          Serial.println("Ready....");
          //startMotorControllerBox=true; //เก็บไว้ยังไม่ได้ใช้ อาจจะเอาออก
          return true;  //สถานะพร้อมรับคำสั่ง อันตรายถ้าอยู่ใกล้
          //digitalWrite(keyStartPIN,LOW);
        }
      }
      wavPlay("ccelst.wav",true,"ยกเลิกการสต๊าส");
    }
  }else{
    if(readSwitch(5, false)) wavPlay("blondstr.wav",true,"สวิทช์ใบตัดเปิดค้างอยู่ ห้ามสต๊าส");
    if(readSwitch(7, false)) wavPlay("enofdstr.wav",true,"สวิทช์เครื่องปิดอยู่ ไม่สามารถสต๊าสได้");
  }
  return false;
}

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

void UpdateCurrentGPS(){
    while (gpsSerial.available() > 0) {  // หลังจากโครงเสร็จต้องเพิ่มให้มี err ด้วย
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        currentLatitude = gps.location.lat();
        currentLongitude = gps.location.lng();
      }
      currentDistance = getDistance(currentLatitude, currentLongitude, lat, lng);
      currentBearing = getBearing(currentLatitude, currentLongitude, lat, lng);
    }
  }
}

// Function to compute distance between two GPS coordinates using the haversine formula
double getDistance(double lat1, double lon1, double lat2, double lon2) {
  double dLat = toRadians(lat2 - lat1);
  double dLon = toRadians(lon2 - lon1);
  double a = pow(sin(dLat / 2), 2) + cos(toRadians(lat1)) * cos(toRadians(lat2)) * pow(sin(dLon / 2), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return 6371000 * c;  // Return distance in meters
}

// Function to compute bearing between two GPS coordinates
double getBearing(double lat1, double lon1, double lat2, double lon2) {
  double y = sin(toRadians(lon2 - lon1)) * cos(toRadians(lat2));
  double x = cos(toRadians(lat1)) * sin(toRadians(lat2)) - sin(toRadians(lat1)) * cos(toRadians(lat2)) * cos(toRadians(lon2 - lon1));
  double bearing = atan2(y, x);
  return bearing >= 0 ? bearing : 2 * 3.14159 + bearing;  // Return bearing in radians between 0 and 2*pi
}

// Function to turn the car in the right direction before going to the destination
void turnToCorrectDirection() {
  double heading_error = currentBearing - currentHeading;
  while (abs(heading_error) > 0.1) {
    sensors_event_t event; 
    compass.getEvent(&event);
    currentHeading = atan2(event.magnetic.y, event.magnetic.x);
    heading_error = currentBearing - currentHeading;
    if (heading_error > 3.14159) {
      heading_error -= 2 * 3.14159;
    } else if (heading_error < -3.14159) {
      heading_error += 2 * 3.14159;
    }
    if (heading_error > 0) {
      directionDriveMotor("LRotate",middle,middle,0,1);
    } else {
      directionDriveMotor("RRotate",middle,middle,1,0);
    }
  }
}

void directionDriveMotor(char* type,int L, int R,bool FBL,bool FBR){
  Serial.println(type);
  if(!FBL && !FBR && bladeStatusON){  //ถอยหลังให้ปิดใบมีด
    bladeStopFunc();
    bladeStopTemp=true;
  }
  if((FBL || FBR) && bladeStopTemp){  //ไม่ถอยหลัง ให้กลับมาเปิดใบมีด หากพึ่งปิดใบมีดเพราะก่อนหน้ามีการถอย
    bladeStartFunc();
    bladeStopTemp=false;
  }
      
  if(digitalRead(rev_L)!=FBL){
    analogWrite(pin_L,slowest);
    digitalWrite(rev_L,FBL);
  }
  if(digitalRead(rev_R)!=FBR){
    analogWrite(pin_R,slowest);
    digitalWrite(rev_R,FBR);
  }
  int L_Value = map(L,0,100,slowest,fastest);
  int R_Value = map(R,0,100,slowest,fastest);
  analogWrite(pin_L,L_Value);
  analogWrite(pin_R,R_Value);
}

void gotoDestinationPoint(int i){
  UpdateCurrentGPS();
  turnToCorrectDirection();
  
  while(currentDistance > 1){
    UpdateCurrentGPS();
    directionDriveMotor("forward",5,5,1,1);
    if(currentDistance < 3){
      bladeStartFunc();
      Serial.println("bladeStared");
    }
  }
}

void autoplanMode(){
  //loadPlan();  // เก็บข้อมูลพิกัดทุกจุดไว้ในตัวแปร point
  File dataFile = SD.open("GPS/planGPS.txt", FILE_WRITE);
  if (!dataFile) {
    wavPlay("unread.wav",true,"ไม่สามารถเปิดไฟล์ได้");
    Serial.println("Failed to open file.");
    return;
  }
  int i=0;
  while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n');
    int comma_pos = line.indexOf(',');
    waypoints[i].latitude = line.substring(0, comma_pos).toDouble();
    waypoints[i].longitude = line.substring(comma_pos + 1).toDouble();
    gotoDestinationPoint(i); 
    i++;
  }
   
}

void bladeStopFunc(){
    digitalWrite(startBladePIN,HIGH);
    bladeStatusON = false;
}

void bladeStartFunc(){
    digitalWrite(startBladePIN,LOW);
    bladeStatusON = true;
}

void logGPS() {
  lat = gps.location.lat();
  lng = gps.location.lng();
  File dataFile = SD.open("GPS/logGPS.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print(lat, 6);
    dataFile.print(", ");
    dataFile.println(lng, 6);
    dataFile.close();
    if(readChannel(2, -10, 10, 0) >= 4 && readSwitch(6, false)){
      File dataFile = SD.open("GPS/planGPS.txt", FILE_WRITE);
      if (dataFile) {
        dataFile.print(lat, 6);
        dataFile.print(", ");
        dataFile.println(lng, 6);
        dataFile.close();
        wavPlay("waypointadded.wav",true,"เพิ่ม waypoint แล้ว");        
      }
    }else{
      wavPlay("unseccesswaypoint.wav",true, "ไม่สามารถเพิ่ม waypoint ได้");              
    }
    Serial.println("Location written to SD card");
  } else {
    Serial.println("Error opening GPS/logGPS.txt");
  }
}

void loop(){
  while(!startEngine){  // รอการสต๊าส
    startEngine=startSystem();
    if(readChannel(3, -10, 10, 0)>=4 && readSwitch(6,false)){      
      autoplanMode();
    }
  }
  
  // หยุดเครื่องฉุกเฉินจะต้อง upgrade เป็นเคส interup ภายหลัง
  if(readSwitch(7,false)){ //ถ้าสถานะ sw8 เป็น true 
    emergencyStop();
    return;  // ออกไปเริ่ม loop ใหม่ ไม่ทำคำสั่งที่เหลือต่อไป
  }

  if(readSwitch(5, false) && !bladeStatusON && !readSwitch(7,false)){  //เปิดสวิทช์ และใบตัดไม่ได้ทำงานอยู่ และ swA ต้อง true ด้วย
    wavPlay("blstrt.wav",true,"ใบตัดกำลังเริ่มทำงานใน"); 
    wavPlay("count3.wav",false,"3");
    delay(1000);
    if(!readSwitch(7, false)){
      if(readSwitch(5,false)){
        wavPlay("count2.wav",false,"2");
        delay(1000);
      }else{
        cancelBladeStart();
        return; // ออกไปเริ่ม loop ใหม่ ไม่ทำคำสั่งที่เหลือต่อไป
      }
    }else{
      emergencyStop();
      return; // ออกไปเริ่ม loop ใหม่ ไม่ทำคำสั่งที่เหลือต่อไป
    }
    
    if(!readSwitch(7, false)){
      if(readSwitch(5,false)){
        wavPlay("count1.wav",false,"1");
        delay(1000);
      }else{
        cancelBladeStart();
        return; // ออกไปเริ่ม loop ใหม่ ไม่ทำคำสั่งที่เหลือต่อไป
      }
    }else{
      emergencyStop();
      return; // ออกไปเริ่ม loop ใหม่ ไม่ทำคำสั่งที่เหลือต่อไป
    }
    bladeStartFunc();
    Serial.println("bladeStared");
  }else if(!readSwitch(5, false) && bladeStatusON){  //ปิดสวิทช์ และใบตัดทำงานอยู่
    bladeStopFunc();
    Serial.println("bladeStoped");
    wavPlay("bldstop.wav",false,"ใบตัดหยุดทำงาน"); //ใบตัดหยุดทำงาน 
  }      

  ch_1 = readChannel(0, -10, 10, 0);
  ch_2 = readChannel(1, -10, 10, 0);

  if(ch_1 >= 0 && ch_2 >= 0) directionDriveMotor("forward",ch_1,ch_2,1,1); //เดินหน้า
  if(ch_1 < 0 && ch_1 >= -5 && ch_1*(-1) <= ch_2) directionDriveMotor("LRotate",ch_1*(-1),ch_2,0,1); //หมุนซ้าย
  if(ch_2 < 0 && ch_2 >= -5 && ch_2*(-1) <= ch_1) directionDriveMotor("RRotate",ch_1,ch_2*(-1),1,0); //หมุนขวา

  if(ch_1 < 0 && ch_2 < 0) directionDriveMotor("Backward",ch_2*(-1),ch_1*(-1),0,0); //ถอยหลัง
  if(ch_1 < 0 && ch_2 >=0 && ch_2 < 5 && ch_1*(-1) > ch_2) directionDriveMotor("LBackward",ch_2,ch_1*(-1),1,0); //ถอยหลังซ้าย  
  if(ch_2 < 0 && ch_1 >=0 && ch_1 < 5 && ch_2*(-1) > ch_1) directionDriveMotor("RBackward",ch_2*(-1),ch_1,0,1); //ถอยหลังขวา 
    
  // Check if there is new GPS data available
  while (gpsSerial.available()) {
    // Parse the NMEA data using the TinyGPS library
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid() && (lat != gps.location.lat() || lng != gps.location.lng())) logGPS();
}
