#include <PWM.h>

int pwm_pin = 10;
int sw1 = 2;

void setup() {
  Serial.begin(9600);
  InitTimersSafe();
  pinMode(pwm_pin, OUTPUT);
  pinMode(sw1, INPUT_PULLUP);
  pwmWrite(pwm_pin, 0);
}

void loop() {
  if(digitalRead(sw1) == 0){
    SetPinFrequencySafe(pwm_pin, 1000);      //ความถี่ 1000Hz
    pwmWrite(pwm_pin, 125);                 //Duty Cycle 50%( 0 - 255 )
  }
  else{
    pwmWrite(pwm_pin, 0);
  }

}