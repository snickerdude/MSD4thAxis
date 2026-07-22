#include <keypad.h>
#include <encoder.h>

// pins used for the handwheel encoder
const uint8_t HAND_A = 4;
const uint8_t HAND_B = 5;

uint8_t state = 0;
int32_t pos = 0;

keypad Keypad;
encoder Handwheel(HAND_A, HAND_B);

void setup(){
  Serial.begin(9600);
  Keypad.begin();
  Handwheel.begin();
  //pinMode(HAND_A, INPUT);
  //pinMode(HAND_B, INPUT);
}

void loop(){
  delay(1000);
  Serial.println(Handwheel.getPosition());
  //Keypad.scan();
  //digitalWrite(12, HIGH);
  for (int i = 0; i < 8; i ++) {
    //Serial.println(0xFFE3FFFF + (i << 18), BIN);

    //if (digitalRead(i) == HIGH){
    //  Serial.print(i);
    //  Serial.print(' ');
    //}
  }
  //Serial.println("Its an esp32 bitch");
  }