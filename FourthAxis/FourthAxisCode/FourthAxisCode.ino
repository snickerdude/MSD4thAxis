#include <keypad.h>
#include <encoder.h>
#include <display.h>
#include "A4988.h"
#include <math.h>

// using a 200-step motor (most common)
#define MOTOR_STEPS 200
// pins used for the stepper driver
#define MOTOR_STP 26
#define MOTOR_DIR 27
#define MOTOR_EN 32
// target position of the motor in steps
// signed integer so it can rotate backwards
int32_t TARGET_POS = 0;
int32_t ACTUAL_POS = 0;

// pins used for the handwheel encoder
const uint8_t HAND_A = 4;
const uint8_t HAND_B = 5;
const uint16_t HAND_DEB = 100; // debounce delay in us
uint8_t temp_HW = 0;

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

// function prototypes
void StepperOutTask(void* params);
void updateParams();

// define tasks
TaskHandle_t StepperOut;
// mutex for position
SemaphoreHandle_t POS_mutex = NULL;
#define MUTEX_TIMEOUT 5000

encoder Handwheel(HAND_A, HAND_B, 100, HAND_DEB);
encoder Encoder(ENC_A, ENC_B, ENC_Z, ENC_DEB);
display Display;
keypad kp;
BasicStepperDriver Stepper(MOTOR_STEPS, MOTOR_DIR, MOTOR_STP, MOTOR_EN);

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
  Serial.begin(115200);
  Handwheel.begin();
  Encoder.begin();
  Display.begin();
  kp.begin();
  
  pinMode(MOTOR_DIR, OUTPUT);
  pinMode(MOTOR_STP, OUTPUT);
  pinMode(MOTOR_EN, OUTPUT);

  xTaskCreatePinnedToCore(
    StepperOutTask, /* Function to implement the task */
    "stp and dir output", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &StepperOut,  /* Task handle. */
    0); /* Core where the task should run */

  POS_mutex = xSemaphoreCreateMutex();

  pinMode(ESTOP, INPUT);
  pinMode(KNOB_MULT, INPUT);
  pinMode(KNOB_MODE, INPUT);
  digitalWrite(MOTOR_DIR, HIGH);
  digitalWrite(MOTOR_EN, LOW);
  
  analogReadResolution(12);   // match ADC_BITS in the main code (0-4095)

  Display.DC.mode = 1;
  Display.DC.mult = 2;
  Display.DC.encPos = 0;
}

void loop(){
  updateParams();

  Display.DC.hwPos = Handwheel._direction;
  Display.DC.encPos = Encoder._position;
  Display.DC.mode = params.mode;
  Display.DC.mult = params.mult;
  Display.DC.actPos = ACTUAL_POS;
  Display.DC.angle  = positionToAngle(ACTUAL_POS);
  Display.updateDisplay();


  if (params.mode == 0) {
    // HW logic
    
    uint8_t steps = 10 * pow(2, (params.mult + 1));
    if (Handwheel._direction == 1 && xSemaphoreTake(POS_mutex, MUTEX_TIMEOUT)){
      if (TARGET_POS - ACTUAL_POS < steps*2) {
        TARGET_POS += steps;
      }
      xSemaphoreGive(POS_mutex);
    } else if (Handwheel._direction == 2 && xSemaphoreTake(POS_mutex, MUTEX_TIMEOUT)){
      if (ACTUAL_POS - TARGET_POS < steps*2) {
        TARGET_POS -= steps;
      }
      xSemaphoreGive(POS_mutex);
    }
    if (Handwheel._direction) Handwheel._direction = 0;
    
    /*
      check HW dir, check estop, send multiplier*dir number of pulses
    */
  } else if (params.mode == 1) {
    // Jog logic
    uint8_t steps = pow(2, (params.mult + 1));
    if (true && xSemaphoreTake(POS_mutex, MUTEX_TIMEOUT)){
      //Serial.println(kp.isKeyPressed(K_JOG_CW));
      if (kp.isKeyPressed(K_JOG_CW)) {
        Serial.println("JOG CW pressed");
        TARGET_POS += steps;
      }
      if (kp.isKeyPressed(K_JOG_CCW)) {
        Serial.println("JOG CCW pressed");
        TARGET_POS -= steps;
      }
      xSemaphoreGive(POS_mutex);
    }
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

//keypad testing
  kp.update();
  /*
    uint8_t k = kp.getKey();
    if (k) Serial.printf("pressed: 0x%02X\n", k);
    if (kp.isKeyPressed(K_JOG_CW)) Serial.println("jog CW held");
    */
    
}

void updateParams(){
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

void StepperOutTask(void* Params){
  Stepper.begin(360, 1);
  int32_t steps = 0;
  for(;;) {
  
    steps = TARGET_POS - ACTUAL_POS;
    //Serial.print(TARGET_POS);
    //Serial.print(' ');
    //Serial.println(ACTUAL_POS);
    if (xSemaphoreTake(POS_mutex, MUTEX_TIMEOUT)){
      if (steps != 0){
        Stepper.move(steps);
        ACTUAL_POS += steps;
      }
      xSemaphoreGive(POS_mutex);
    }
    
  }
}

float positionToAngle(int32_t pos) {
  int32_t wrapped = pos % 20000; //20000 steps per revolution
  if (wrapped < 0) wrapped += 20000;
  return wrapped * 0.018; //0.018 degree per step = 20000/360
}

//angle to position for angle mode
int32_t angleToPosition(float degrees) {
  return (int32_t)lroundf(degrees / 0.018);
}

  

