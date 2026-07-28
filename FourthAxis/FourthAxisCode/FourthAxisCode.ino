#include <keypad.h>
#include <encoder.h>

// pins used for the handwheel encoder
const uint8_t HAND_A = 4;
const uint8_t HAND_B = 5;
const uint16_t HAND_DEB = 100; //debounce delay

const uint8_t ENC_A = 34;
const uint8_t ENC_B = 35;
const uint8_t ENC_Z = 36;
const uint16_t ENC_DEB = 10; // 10us debounce delay

uint8_t state = 0;
int32_t pos = 0;

keypad Keypad;
encoder Handwheel(HAND_A, HAND_B, 100, HAND_DEB);
encoder Encoder(ENC_A, ENC_B, ENC_Z, ENC_DEB);
void setup(){
  Serial.begin(9600);
  Keypad.begin();
  Handwheel.begin();
  Encoder.begin();
  //pinMode(HAND_A, INPUT);
  //pinMode(HAND_B, INPUT);
}

void loop(){
  delay(1000);
  //Serial.println(Handwheel.getPosition());
  //Serial.println(Encoder.getPosition());
  Serial.println(Encoder._position);
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