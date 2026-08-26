#include <keypad.h>
#include <encoder.h>
#include <display.h>

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
// enum used to assign strings to each key value
enum keymap{
  NUM_1, 	NUM_2,	NUM_3, 			NOKEY_1, 	SET_ZERO, 		RET_ZERO, 		UNASGN_1,
  NUM_4, 	NUM_5,	NUM_6, 			NOKEY_2, 	SET_SPLN, 		NXT_ANGL, 		UNASGN_2,
  NUM_7, 	NUM_8,	NUM_9, 			DEL,			SET_ANGL_ABS, ADD_ANGL_INC, UNASGN_3,
  DOT,		NUM_0,	PLUS_MINUS,	ENTER,		JOG_CCW,			JOG_CW,				UNASGN_4
};

//keymap Keymap == keymap::ENTER;

encoder Handwheel(HAND_A, HAND_B, 100, HAND_DEB);
encoder Encoder(ENC_A, ENC_B, ENC_Z, ENC_DEB);
display Display;


void setup(){
  Serial.begin(9600);
  Keypad.begin();
  Handwheel.begin();
  Encoder.begin();
  Display.begin();

  Display.DC.mode = 1;
  Display.DC.mult = 2;
  Display.DC.encPos = 0;
}

void loop(){
  delay(10);
  //Serial.println(Handwheel.getPosition());
  //Serial.println(Encoder.getPosition());
  //Serial.println(Handwheel._position);
  //Keypad.scan();
  //digitalWrite(12, HIGH);
  Display.DC.hwPos = Handwheel._position;
  Display.DC.encPos = Encoder._position;
  Display.updateDisplay();
  Serial.println(Keymap::ENTER);
  for (int i = 0; i < 8; i ++) {
    
    //Serial.println(0xFFE3FFFF + (i << 18), BIN);

    //if (digitalRead(i) == HIGH){
    //  Serial.print(i);
    //  Serial.print(' ');
    //}
  }
  //Serial.println("Its an esp32 bitch");
  }