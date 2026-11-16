#include <keypad.h>
#include <encoder.h>
#include <display.h>
#include "A4988.h"
#include <math.h>

// using a 200-step motor (most common)
// using a 100 to 1 worm gear reduction
#define MOTOR_STEPS 200
#define WORM_REDUC 100
//1:100 worm gear ratio so 20000
#define STEPS_PER_REV (MOTOR_STEPS*WORM_REDUC)
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

//angle mode variables
enum AngleState : uint8_t { ANG_IDLE, ANG_ABS, ANG_INC }; //angle mode states
static AngleState angState = ANG_IDLE;
static char    angBuf[10]; //typed characters
static uint8_t angLen = 0; //number of characters in buffer
static bool    angNeg = false; //sign +-

//zero offset
volatile int32_t ZERO_OFFSET = 0;

//spline mode
#define MAX_SPLINES 999
enum SplineState : uint8_t { SPL_IDLE, SPL_ENTRY, SPL_ACTIVE };
static SplineState splState = SPL_IDLE;
static char    splBuf[4];          // up to 3 digits
static uint8_t splLen = 0;
static int32_t splCount = 0;       // N = splines per revolution
static int32_t splIndex = 0;       // 0..N-1, which spline we're on
static int32_t splBase  = 0;       // raw position of the current lap's first spline

void setup(){
  Serial.begin(115200);

  Serial.printf("Reset reason: %d\n", (int)esp_reset_reason());
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

  //mode change
  static uint8_t lastMode = 0xFF;
  if (params.mode != lastMode) {
    angleReset();
    splineReset();
    kp.getKey();                       // flush stale key
    lastMode = params.mode;
  }
  uint8_t key = kp.getKey();           // read once per pass
  if (xSemaphoreTake(POS_mutex, MUTEX_TIMEOUT)){
    if (handleZeroKeys(key)) key = K_NONE;   // zero keys handled here, in every mode
    Display.DC.actPos = ACTUAL_POS; // this is here to mutex protect it
    Display.DC.angle  = positionToAngle(ACTUAL_POS); // this is here to mutex protect it
    xSemaphoreGive(POS_mutex);
  }
  
  Display.DC.hwPos = Handwheel._direction;
  Display.DC.encPos = Encoder._position;
  Display.DC.mode = params.mode;
  Display.DC.mult = params.mult;
  Display.DC.angState = (params.mode == 2) ? angState : 0;
  snprintf((char*)Display.DC.entry, sizeof(Display.DC.entry), "%s%s", angNeg ? "-" : "", angBuf);
  Display.updateDisplay();
  Display.DC.splState = (params.mode == 3) ? splState : 0;
if (splState == SPL_ENTRY)
  snprintf((char*)Display.DC.entrySpline, sizeof(Display.DC.entrySpline), "%s", splBuf);
else if (splState == SPL_ACTIVE)
  snprintf((char*)Display.DC.entrySpline, sizeof(Display.DC.entrySpline), "%ld/%ld",
           (long)(splIndex + 1), (long)splCount);


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
    if (xSemaphoreTake(POS_mutex, MUTEX_TIMEOUT)){
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
    angleModeUpdate(key);
    /*
      check estop
      prompt for angle
      check for (go to angle) or (increment by angle)
      move by angle while monitoring estop
    */
  } else if (params.mode == 3) {
    // spline logic
    splineModeUpdate(key);
    /*
      check estop
      prompt for number of splines
      wait for next spline
      rotate by calculated # of steps/degrees
    */
  lastMode = params.mode;
  } else {
    // error out. invalid mode
  }

  kp.update();
}

void updateParams(){
  uint16_t mult = analogRead(KNOB_MULT);
  uint16_t mode = analogRead(KNOB_MODE);
  uint8_t estop = !digitalRead(ESTOP);
  //KNOBS
  if (mode < 1000)      params.mode = 0;   // Handwheel
  else if (mode < 2200) params.mode = 1;   // Jog
  else if (mode < 3200) params.mode = 2;   // Angle
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
  volatile int32_t steps_remain;
  for(;;) {
    if (xSemaphoreTake(POS_mutex, MUTEX_TIMEOUT)){
      steps_remain = TARGET_POS - ACTUAL_POS;
      if (steps_remain != 0){
        int32_t chunk = constrain(steps_remain, -200, 200);
        Stepper.move(chunk);
        ACTUAL_POS += chunk;
      }
      xSemaphoreGive(POS_mutex);
    }
    vTaskDelay(1);
  }
}

float positionToAngle(int32_t pos) {
  int32_t wrapped = pos % STEPS_PER_REV; //20000 steps per revolution
  if (wrapped < 0) wrapped += STEPS_PER_REV;
  return wrapped * 0.018; //0.018 degree per step = 20000/360
}

//angle to position for angle mode
int32_t angleToPosition(float degrees) {
  return (int32_t)lroundf(degrees / 0.018);
}

//angle mode functions

void angleReset() { //clear all
  angState = ANG_IDLE;
  angLen = 0;
  angBuf[0] = '\0';
  angNeg = false;
}

static bool angHasDecimal() { return strchr(angBuf, '.') != nullptr; }

void angleCommit() {
  if (angLen == 0) return; // nothing typed yet
  float deg = atof(angBuf); //convert string into float
  if (angNeg) deg = -deg; //apply the sign

  int32_t delta;
  if (angState == ANG_INC) {
    // relative to where the spindle is (or is heading)
    delta = angleToPosition(deg);
  } else {
    // ABS: go to this angle relative to zero
    int32_t want = angleToPosition(deg) % STEPS_PER_REV; //desired position
    if (want < 0) want += STEPS_PER_REV;
    //int32_t cur = TARGET_POS % STEPS_PER_REV; //current position
    //if (cur < 0) cur += STEPS_PER_REV;
    if (xSemaphoreTake(POS_mutex, MUTEX_TIMEOUT)){
      delta = want - ACTUAL_POS;
      xSemaphoreGive(POS_mutex);
    }
    //shortest path
    if (delta >  STEPS_PER_REV / 2) delta -= STEPS_PER_REV;
    if (delta <= -STEPS_PER_REV / 2) delta += STEPS_PER_REV;
  }
  if (xSemaphoreTake(POS_mutex, MUTEX_TIMEOUT)){
    TARGET_POS += delta;
    xSemaphoreGive(POS_mutex);
  }
  angleReset();                               // back to idle, ready for next entry
}

void angleModeUpdate(uint8_t k) {
  if (params.estop) { angleReset(); return; } // e-stop cancels any entry
  if (k == K_NONE) return;
  //ABS OR INC MODE
  if (k == K_SET_ANGLE_ABS) { angState = ANG_ABS; return; }
  if (k == K_ADD_ANGLE_INC) { angState = ANG_INC; return; }
  if (angState == ANG_IDLE) return;           // ignore everything else until started
  //key inputs
  if ((k >= 1 && k <= 9) || k == '0') {
    if (angLen < sizeof(angBuf) - 1) {
      angBuf[angLen++] = (k == '0') ? '0' : ('0' + k);
      angBuf[angLen] = '\0';
    }
  } else if (k == K_DECIMAL) {
    if (!angHasDecimal() && angLen < sizeof(angBuf) - 2) {
      if (angLen == 0) angBuf[angLen++] = '0';   // ".5" -> "0.5"
      angBuf[angLen++] = '.';
      angBuf[angLen] = '\0';
    }
  } else if (k == K_PLUSMINUS) {
    angNeg = !angNeg;
  } else if (k == K_DEL) {
    if (angLen > 0)  angBuf[--angLen] = '\0';    // backspace
    else if (angNeg) angNeg = false;
    else             angleReset();               // DEL on empty entry cancels
  } else if (k == K_ENTER) {
    angleCommit();
  }
}
//spline mode
void splineReset() {
  splState = SPL_IDLE;
  splLen = 0;
  splBuf[0] = '\0';
  splCount = 0;
  splIndex = 0;
}
static char keyToDigitChar(uint8_t k) {
  if (k >= 1 && k <= 9) return '0' + k;
  if (k == '0') return '0';
  return 0;
}
// Raw position of the current spline, measured from the lap's base
static int32_t splineTarget() {
  return splBase + (splIndex * STEPS_PER_REV + splCount / 2) / splCount;
}

void splineModeUpdate(uint8_t k) {
  if (params.estop) { splineReset(); return; }   // e-stop ends the sequence
  if (k == K_NONE) return;

  // Set Splines starts (or restarts) a sequence from any state
  if (k == K_SET_SPLINES) { splineReset(); splState = SPL_ENTRY; return; }

  if (splState == SPL_ENTRY) {
    char d = keyToDigitChar(k);
    if (d) {
      if (splLen < sizeof(splBuf) - 1) {
        splBuf[splLen++] = d;
        splBuf[splLen] = '\0';
      }
    } else if (k == K_DEL) {
      if (splLen > 0) splBuf[--splLen] = '\0';
      else            splineReset();             // DEL on empty entry cancels
    } else if (k == K_ENTER) {
      int32_t n = atoi(splBuf);
      if (n >= 2 && n <= MAX_SPLINES) {
        splCount = n;
        splIndex = 0;
        splBase  = TARGET_POS;                   // spline 1 is wherever the spindle is now
        splState = SPL_ACTIVE;
      } else {
        splLen = 0;                              // invalid count: clear, let them retype
        splBuf[0] = '\0';
      }
    }
  } else if (splState == SPL_ACTIVE) {
    // Ignore Next Angle while still moving so a double press can't skip a spline
    if (k == K_NEXT_ANGLE && TARGET_POS == ACTUAL_POS) {
      splIndex++;
      if (splIndex >= splCount) {                // finished a lap: keep turning the same way
        splBase += STEPS_PER_REV;
        splIndex = 0;
      }
      TARGET_POS = splineTarget();
    }
  }
}



//zeroing
bool handleZeroKeys(uint8_t k) {
  if (k == K_SET_ZERO) {
    if (TARGET_POS == ACTUAL_POS) {ACTUAL_POS = 0; TARGET_POS = 0;}   // only when stopped
    //angleReset();               // harmless outside angle mode
    return true;
  }
  if (k == K_RETURN_ZERO) {
    if (!params.estop) {
      int32_t rel = ACTUAL_POS;//(TARGET_POS - ZERO_OFFSET) % STEPS_PER_REV;
      if (rel >  STEPS_PER_REV / 2) rel -= STEPS_PER_REV;
      if (rel < -STEPS_PER_REV / 2) rel += STEPS_PER_REV;
      TARGET_POS -= rel;
    }
    //angleReset();
    return true;
  }
  return false;
}
