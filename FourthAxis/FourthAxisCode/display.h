#ifndef DISPLAY_H
#define DISPLAY_H


#include <Wire.h> // used for i2c comms
#include <DIYables_OLED_SSD1309.h> // SSD1309 oled screen driver
#include <String.h>

struct displayContents {
	uint8_t mode;
	uint8_t mult;
	int32_t encPos;
	int32_t hwPos;
	uint8_t key;
	bool estop;
};


class display {
	public:
		display();
		volatile displayContents DC;
		
		void begin();
		
		void updateDisplay();
		
	private:
		const uint8_t SCREEN_WIDTH =		128;
		const uint8_t SCREEN_HEIGHT = 	64;
		const int8_t OLED_RESET = 			-1;
		static const uint8_t SCREEN_ADDRESS =	0x3C;
		
		DIYables_OLED_SSD1309 Display;
};
#endif // DISPLAY_H