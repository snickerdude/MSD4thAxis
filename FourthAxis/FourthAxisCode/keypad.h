//#pragma once

#ifndef MCP3562_H
#define MCP3562_H

#include <Arduino.h>
#include <vector>

class keypad {
public:
	// Class constructor
    keypad();
	// Class destructor
	~keypad();
	
    /**
     * @brief Initialize the GPIO pins. Call this in setup().
     */
    void begin();

    /**
     * @brief Periodic scan of the matrix. Call this in loop().
     * This is non-blocking and handles debouncing internally.
     */
    void update();
	
	/**
	 * @brief Scan all the keys and serial print which ones are pressed
	 */
	void scan();

    /**
     * @brief Register a callback function for key events.
     */
    //void registerCallback(KeyCallback callback);

    /**
     * @brief Check if a specific key is currently pressed (active state).
     */
    bool isKeyPressed(char key) const;

private:
	
	// Pin definitions for the 8 MUXed cols and the 4 rows.
	static const uint8_t KEY_A = 	12;
	static const uint8_t KEY_B = 	13;
	static const uint8_t KEY_C = 	14;
	static const uint8_t KEY_1 = 	15;	
	static const uint8_t KEY_2 = 	16;
	static const uint8_t KEY_3 = 	17;
	static const uint8_t KEY_4 = 	18;
	
	// Number of rows and columns in the keyboard
	static const uint8_t NUM_ROWS =	4;
	static const uint8_t NUM_COLS =	7;
	
	// GPIO mask used to clear the MUX outputs
	static const uint32_t CLR_MASK =	0x00007000;
	// GPIO mask used to read the keeb inputs
	static const uint32_t READ_MASK =	0x00078000;
	
    
	const char* _keyMap;

    // Internal tracking structures
    bool* _keyStates;       // Current stable debounced state of each key
    bool* _rawStates;       // Raw physical state during scan
    unsigned long* _lastDebounceTime; // Timestamp of last state flip per key

    static constexpr unsigned long DEBOUNCE_DELAY_MS = 25; // Debounce window
    //KeyCallback _eventCallback = nullptr;

    // Helper to translate 2D coordinates to a flat index
    inline int getIndex(uint8_t row, uint8_t col) const {
        return (row * NUM_COLS) + col;
    }
};

#endif // MCP3562_H