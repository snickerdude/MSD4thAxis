#include "keypad.h"


/*
//// Key map:
//// x means no key assigned
[1, 2, 3, x, set zero, return to zero, x
4, 5, 6, x, set splines, next angle, x
7, 8, 9, del, set angle abs, add angle inc, x
decimal, 0, +-, enter, jog CCW, jog CW, x]
*/
static const uint8_t KEYMAP[28] = {
    '1','2','3', K_NONE, K_SET_ZERO,      K_RETURN_ZERO,   K_NONE,
    '4','5','6', K_NONE, K_SET_SPLINES,   K_NEXT_ANGLE,    K_NONE,
    '7','8','9', K_DEL,  K_SET_ANGLE_ABS, K_ADD_ANGLE_INC, K_NONE,
    K_DECIMAL,'0',K_PLUSMINUS, K_ENTER, K_JOG_CCW, K_JOG_CW, K_NONE
};

keypad::keypad() {
    int totalKeys = NUM_ROWS * NUM_COLS;;
		_keyMap = KEYMAP;
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
    unsigned long now = millis();

    for (uint8_t col = 0; col < NUM_COLS; col++) {
        REG_WRITE(GPIO_OUT_W1TC_REG, CLR_MASK);
        REG_WRITE(GPIO_OUT_W1TS_REG, (uint32_t)col << 12);
        delayMicroseconds(5);
        uint8_t rows = (REG_READ(GPIO_IN_REG) & READ_MASK) >> 15;

        for (uint8_t row = 0; row < NUM_ROWS; row++) {
            int idx = getIndex(row, col);
            bool raw = rows & (1 << row);

            if (raw != _rawStates[idx]) {
                _rawStates[idx] = raw;
                _lastDebounceTime[idx] = now;          // reading changed, restart timer
            } else if (raw != _keyStates[idx] &&
                       (now - _lastDebounceTime[idx]) >= DEBOUNCE_DELAY_MS) {
                _keyStates[idx] = raw;                 // stable long enough, commit
                if (raw && _keyMap[idx] != K_NONE) _lastPressed = _keyMap[idx];
            }
        }
    }
}

uint8_t keypad::getKey() {
    uint8_t k = _lastPressed;
    _lastPressed = K_NONE;
    return k;
}

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

bool keypad::isKeyPressed(uint8_t key) const {
		if (key == K_NONE) return false;
    int totalKeys = NUM_ROWS * NUM_COLS;
    for (int i = 0; i < totalKeys; ++i) {
        if (_keyMap[i] == key) {
            return _keyStates[i];
        }
    }
    return false;
}

