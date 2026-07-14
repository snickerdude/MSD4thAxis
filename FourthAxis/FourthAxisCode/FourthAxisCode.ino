#include <keypad.h>

keypad Keypad;

void setup(){
  Serial.begin(9600);
  Keypad.begin();
}

void loop(){
  delay(1000);
  Keypad.scan();
  //digitalWrite(12, HIGH);
  for (int i = 0; i < 8; i ++) {
    //Serial.println(0xFFE3FFFF + (i << 18), BIN);

    //if (digitalRead(i) == HIGH){
    //  Serial.print(i);
    //  Serial.print(' ');
    //}
  }
  Serial.println("Its an esp32 bitch");
  //key
  }