#include "display.h"

display::display():
	// construct a display class
	Display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)
{
	
}

void display::begin(){
	if (!Display.begin(SSD1309_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1309 allocation failed"));
    for (;;)
      ; // halt
  }
	Display.clearDisplay();
	Display.setTextSize(1);               // Normal 1:1 pixel scale
  Display.setTextColor(SSD1309_PIXEL_ON);  // Pixels on
	Serial.println("Display setup success");
}

void display::updateDisplay(){
	const char* tempText;
	Display.clearDisplay();
	Display.setCursor(0, 0);
	switch (DC.mode) {
		case 0:
			tempText = "Handwheel";
			break;
		case 1:
			tempText = "Spline";
			break;
		default:
			tempText = "err";
	}
	Display.printf("Current mode: %s\n", tempText);
	
	switch (DC.mult) {
		case 0:
			tempText = "1x";
			break;
		case 1:
			tempText = "10x";
			break;
		case 2:
			tempText = "100x";
			break;
		default:
			tempText = "err";
	}
	Display.printf("Multiplier: %s\n", tempText);	
	
	double encPosDeg = ((double)DC.encPos / 57000) * 360;
	Display.printf("Encoder pos: %.4f\n", encPosDeg);
	Display.printf("Handwheel pos: %d\n", DC.hwPos);
	
	Display.display();
}
