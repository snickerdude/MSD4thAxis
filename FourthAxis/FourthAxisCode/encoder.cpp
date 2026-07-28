#include "encoder.h"

encoder::encoder(uint8_t aPin, uint8_t bPin, uint8_t zPin, uint16_t debounce) 
	: _aPin(aPin),
		_bPin(bPin),
		_zPin(zPin),
		_debounce(debounce)
{
	
}

void encoder::begin(){
	pinMode(_aPin, INPUT);
	pinMode(_bPin, INPUT);
	pinMode(_zPin, INPUT);
	attachInterruptArg(digitalPinToInterrupt(_aPin), encoder::staticAisr, this, RISING);
	if (_zPin != 100){
		attachInterruptArg(digitalPinToInterrupt(_zPin), encoder::staticZisr, this, RISING);
	}
	_timer = esp_timer_get_time();
	Serial.println(_debounce);
	Serial.println("Handwheel setup success");
}

int32_t encoder::getPosition(){
	return _position;
}

void ARDUINO_ISR_ATTR encoder::staticAisr(void* arg) {
	encoder* instance = static_cast<encoder*>(arg);
  instance->handleA();
}

void ARDUINO_ISR_ATTR encoder::staticZisr(void* arg) {
	encoder* instance = static_cast<encoder*>(arg);
  instance->handleZ();
}

void ARDUINO_ISR_ATTR encoder::handleA() {
	uint64_t now = esp_timer_get_time();
	if (now - _timer > _debounce) {
		if (digitalRead(_bPin) ? _position -- : _position ++);
		_timer = now;
	}
}

void ARDUINO_ISR_ATTR encoder::handleZ() {
	Serial.println("zero!!");
}