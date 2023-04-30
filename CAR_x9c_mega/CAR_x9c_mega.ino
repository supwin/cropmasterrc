  #include <LapX9C10X.h>
  #include <IBusBM.h>
  IBusBM ibus;
  #include <iBus.h>
  
  
  #define CH_L 2
  #define rev_L 4
  #define CSPIN_L 6
  #define INCPIN_L 7
  #define UDPIN_L 8
  #define CH_R 3
  #define rev_R 5
  #define CSPIN_R 9
  #define INCPIN_R 10
  #define UDPIN_R 11

  int ch_L_Value;
  int ch_L_Val;
  uint8_t r_L_Value;
  int ch_R_Value;
  int ch_R_Val;
  uint8_t r_R_Value;
  LapX9C10X vr_L(INCPIN_L, UDPIN_L, CSPIN_L, LAPX9C10X_X9C103);
  LapX9C10X vr_R(INCPIN_R, UDPIN_R, CSPIN_R, LAPX9C10X_X9C103);

  int readChannel(int channelInput, int minLimit, int maxLimit, int defaultValue){
    if (ibus.read()) {
      int ch = ibus.channel(channelInput);
      if (ch < 0) return defaultValue;
      return map(ch, 1000, 2000, minLimit, maxLimit);
    }
    return defaultValue;
  }


  void setup() {
    Serial.begin(9600);
    Serial1.begin(115200); // initialize Serial1 for iBUS communication
    IBus.begin(Serial1); 
    Serial.println("Starting");  
    vr_L.begin(99);
    vr_R.begin(99);
    //delay(5000);
    
    pinMode(CH_L, INPUT);
    pinMode(CH_R, INPUT);
    pinMode(rev_L, OUTPUT);
    pinMode(rev_R, OUTPUT);
    digitalWrite(rev_L,LOW);
    digitalWrite(rev_R,LOW);
  }

  void loop() {

    ch_L_Value = readChannel(CH_L, -60, 60, 0);
    ch_R_Value = readChannel(CH_R, -60, 60, 0);


    if(ch_L_Value<0){
      r_L_Value = 60+ch_L_Value;
      rev_L = HIGHT;
    }else{
      r_L_Value = 60-ch_L_Value;
      rev_L = LOW;
    }

    vr_L.set(rev_L);
    
    if(ch_R_Value<0){
      r_R_Value = 60+ch_R_Value;
      rev_R = HIGHT;
    }else{
      r_R_Value = 60-ch_R_Value;
      rev_R = LOW;
    }

    vr_R.set(rev_R);
    
    delay(1000);
  }
