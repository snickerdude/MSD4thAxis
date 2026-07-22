#include "encoder.h"

encoder::encoder(uint8_t aPin, uint8_t bPin, uint8_t zPin) 
	: _aPin(aPin),
		_bPin(bPin),
		_zPin(zPin)
{
	
}

void encoder::begin(){
	pinMode(_aPin, INPUT);
	pinMode(_bPin, INPUT);
	pinMode(_zPin, INPUT);
	attachInterruptArg(digitalPinToInterrupt(_aPin), encoder::staticAisr, this, FALLING);
	if (_zPin != 100){
		attachInterruptArg(digitalPinToInterrupt(_zPin), encoder::staticZisr, this, RISING);
	}
	Serial.println("Handwheel setup success");
}

int32_t encoder::getPosition(){
	return _position;
}

void ARDUINO_ISR_ATTR encoder::staticAisr(void* arg) {
	encoder* instance = static_cast<encoder*>(arg);
  instance->handleA();
	//Serial.println(digitalRead(_bPin));
}

void ARDUINO_ISR_ATTR encoder::staticZisr(void* arg) {
	encoder* instance = static_cast<encoder*>(arg);
  instance->handleZ();
	//Serial.println(digitalRead(_bPin));
}

void ARDUINO_ISR_ATTR encoder::handleA() {
	if (digitalRead(_bPin) ? _position ++ : _position --);
}

void ARDUINO_ISR_ATTR encoder::handleZ() {
	//Serial.println(digitalRead(_bPin));
}