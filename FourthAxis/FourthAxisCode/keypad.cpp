#include "keypad.h"


/*
//// Key map:
//// x means no key assigned
[1, 2, 3, x, set zero, return to zero, x
4, 5, 6, x, set splines, next angle, x
7, 8, 9, del, set angle abs, add angle inc, x
decimal, 0, +-, enter, jog CCW, jog CW, x]
*/

keypad::keypad() {
    int totalKeys = 26;
    _keyStates = new bool[totalKeys]();
    _rawStates = new bool[totalKeys]();
    _lastDebounceTime = new unsigned long[totalKeys]();
}

keypad::~keypad() {
    delete[] _keyStates;
    delete[] _rawStates;
    delete[] _lastDebounceTime;
}

void keypad::begin() {
	// Configure coloum pins as outputs and row pins as inputs. 
	for (uint8_t pin = KEY_A; pin <= KEY_C; pin++) {
	pinMode(pin, OUTPUT);
	}
	for (uint8_t pin = KEY_1; pin <= KEY_4; pin++) {
		pinMode(pin, INPUT_PULLDOWN);
	}
	Serial.println("Keypad setup success");
	
	
}

void keypad::update() {
    
}//11100000000000000000


void keypad::scan() {
	
	for (uint8_t col = 0; col < NUM_COLS; col ++) {
		// Bit masked used to set outputs
		uint32_t outMask = (col << 12); 

		REG_WRITE(GPIO_OUT_W1TC_REG, CLR_MASK); // Clear the MUX outputs
		REG_WRITE(GPIO_OUT_W1TS_REG, outMask); // Set the correct bits
		delayMicroseconds(1);
		Serial.println((REG_READ(GPIO_IN_REG) >> 15), BIN); // Read the results
	}// end for
} // end function 

bool keypad::isKeyPressed(char key) const {
    int totalKeys = NUM_ROWS * NUM_COLS;
    for (int i = 0; i < totalKeys; ++i) {
        if (_keyMap[i] == key) {
            return _keyStates[i];
        }
    }
    return false;
}