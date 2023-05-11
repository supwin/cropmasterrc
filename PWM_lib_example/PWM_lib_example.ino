#include <PWM.h>

int pwm_pin = 10;
int sw1 = 2;

void setup() {
  Serial.begin(9600);
  InitTimersSafe();
  pinMode(3, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(sw1, INPUT_PULLUP);
  pwmWrite(3, 0);
  pwmWrite(9, 0);
}

void loop() {
  ////if(digitalRead(sw1) == 0){
    SetPinFrequencySafe(3, 1000);  
    SetPinFrequencySafe(9, 1000);      //ความถี่ 1000Hz
    pwmWrite(3, 125);        //ความถี่ 1000Hz
    pwmWrite(9, 125);                 //Duty Cycle 50%( 0 - 255 )
  //}
 // else{
 //   pwmWrite(pwm_pin, 0);
 // }

}