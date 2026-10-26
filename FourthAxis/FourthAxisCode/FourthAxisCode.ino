#include <keypad.h>
#include <encoder.h>
#include <display.h>
#include "A4988.h"

// using a 200-step motor (most common)
#define MOTOR_STEPS 200
// configure the pins connected
//#define DIR 27
//#define STEP 26
//#define EN 32

// pins used for the handwheel encoder
const uint8_t HAND_A = 4;
const uint8_t HAND_B = 5;
const uint16_t HAND_DEB = 100; //debounce delay

// pins used for the motor encoder
const uint8_t ENC_A = 34;
const uint8_t ENC_B = 35;
const uint8_t ENC_Z = 36;
const uint16_t ENC_DEB = 10; // 10us debounce delay

//pins for knobs
#define KNOB_MULT 2 //knob 1 USING IO 2
#define KNOB_MODE 25 //knob 2

// pins used for the stepper driver
#define MOTOR_STP 26
#define MOTOR_DIR 27
#define MOTOR_EN 32

//pin for ESTOP
#define ESTOP 39

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
//BasicStepperDriver Stepper(MOTOR_STEPS, MOTOR_DIR, MOTOR_STP, MOTOR_EN);

void updateParams();

struct {
  /*
  Operating mode
    0 = Handwheel
    1 = Jog
    2 = Angle
    3 = Spline
  */
  uint8_t mode; 
  /*
  Multiplier
    0 = 1x
    1 = 10x
    2 = 100x
  */
  uint8_t mult;
  /*
  Estop boolean
    True if e-stop is pressed
    False if e-stop is not pressed
  */
  bool estop;
  /* 
  Handwheel direction
    0 if no change since last check
    1 if CW change since last check
    2 if CCW change since last check
  */
  uint8_t HWdir;
} params;


void setup(){
  Serial.begin(9600);
  Keypad.begin();
  Handwheel.begin();
  Encoder.begin();
  Display.begin();
  //Stepper.begin(100, 1);
  //Stepper.setEnableActiveState(LOW);
  pinMode(MOTOR_DIR, OUTPUT);
  pinMode(MOTOR_STP, OUTPUT);
  pinMode(MOTOR_EN, OUTPUT);
  pinMode(ESTOP, INPUT);
  digitalWrite(MOTOR_DIR, HIGH);
  digitalWrite(MOTOR_EN, LOW);
  
  analogReadResolution(12);   // match ADC_BITS in the main code (0-4095)

  Display.DC.mode = 1;
  Display.DC.mult = 2;
  Display.DC.encPos = 0;
}

void loop(){
  //delay(1);
  //Serial.println(Handwheel.getPosition());
  //Serial.println(Encoder.getPosition());
  //Serial.println(Handwheel._position);
  //Keypad.scan();
  //digitalWrite(12, HIGH);

  updateParams();

  Display.DC.hwPos = Handwheel._position;
  Display.DC.encPos = Encoder._position;
  Display.DC.mode = params.mode;
  Display.DC.mult = params.mult;
  Display.updateDisplay();
  //Stepper.rotate(20);
  if (digitalRead(MOTOR_STP) == HIGH){
    digitalWrite(MOTOR_STP, LOW);
  } else {
    digitalWrite(MOTOR_STP, HIGH);
  }


  //Serial.println(Keymap::ENTER);
  if (params.mode == 0) {
    // HW logic
    /*
      check HW dir, check estop, send multiplier*dir number of pulses
    */
  } else if (params.mode == 1) {
    // Jog logic
    /*
      check estop, check jog button(s) pressed, send multiplier*dir number of pulses
    */
  } else if (params.mode == 2) {
    // angle logic
    /*
      check estop
      prompt for angle
      check for (go to angle) or (increment by angle)
      move by angle while monitoring estop
    */
  } else if (params.mode == 3) {
    // spline logic
    /*
      check estop
      prompt for number of splines
      wait for next spline
      rotate by calculated # of steps/degrees
    */
  } else {
    // error out. invalid mode
  }
}

void updateParams(){
  //params.mode = 0; // read analog pin and update mode accordingly
  //params.mult = 0; // read analog pin and update multiplier accordingly
  //params.HWdir = 0; // check HW direction, update variable, reset HW dir
  //params.estop = 0; // read digital pin and update status accordingly

  uint16_t mult = analogRead(KNOB_MULT);
  uint16_t mode = analogRead(KNOB_MODE);
  uint8_t estop = !digitalRead(ESTOP);

//KNOBS
if (mode < 1000)      params.mode = 0;   // Handwheel
  else if (mode < 2000) params.mode = 1;   // Jog
  else if (mode < 3000) params.mode = 2;   // Angle
  else                  params.mode = 3;   // Spline

  if (mult < 1500)      params.mult = 0;   // 1x
  else if (mult < 3000) params.mult = 1;   // 10x
  else                  params.mult = 2;   // 100x

  //ESTOP
  params.estop = estop;
  Display.DC.estop = estop;
}