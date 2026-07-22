#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

class encoder {
  public:
		encoder(uint8_t aPin, uint8_t bPin, uint8_t zPin = 100);
		
		void begin();
		int32_t getPosition();
	private:
		uint8_t _aPin;
		uint8_t _bPin;
		uint8_t _zPin;
		
		// intertupt service routines
		static void ARDUINO_ISR_ATTR staticAisr(void* arg);
		static void ARDUINO_ISR_ATTR staticZisr(void* arg);
		
		void handleA();
		void handleZ();
		
		// other variables
		volatile int32_t _position = 0;
		
};

#endif // ENCODER_H