#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>
#include <esp_timer.h>

class encoder {
  public:
		encoder(uint8_t aPin, uint8_t bPin, uint8_t zPin = 100, uint16_t debounce = 100);
		
		void begin();
		int32_t getPosition();
		
		// other variables
		volatile int32_t _position = 0;
	private:
		uint8_t _aPin;
		uint8_t _bPin;
		uint8_t _zPin;
		
		uint16_t _debounce; // debounce delay in microseconds
		volatile uint64_t _timer;
		// intertupt service routines
		static void ARDUINO_ISR_ATTR staticAisr(void* arg);
		static void ARDUINO_ISR_ATTR staticZisr(void* arg);
		
		void handleA();
		void handleZ();		
};

#endif // ENCODER_H