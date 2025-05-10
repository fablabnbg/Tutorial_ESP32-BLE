#include <Arduino.h>

#include "MyBLE.h"
#include "CommandHandler.h"

MyBLE ble;
extern CommandHandler cmdHandler;

//void switchVibration() {
//	pinMode
//
//};

void setup() {
	Serial.begin(115200);
	Serial.println("FLN BLE Workshop");
	ble.init();
	pinMode(7, OUTPUT);    // sets the digital pin 13 as output
	pinMode(3, OUTPUT);    // sets the digital pin 13 as output
//	digitalWrite(7, HIGH);
//	digitalWrite(3, HIGH);
//	delay(3000);
//	digitalWrite(7, LOW);
//	digitalWrite(3, LOW);
//	delay(3000);
	for (uint8_t i = 0; i<10; i++) {
		digitalWrite(7, HIGH);
		delay(50);
		digitalWrite(7, LOW);
		digitalWrite(3, HIGH);
		delay(50);
		digitalWrite(3, LOW);
		delay(100);
	}


	cmdHandler.setup();
}

void loop() {
	cmdHandler.loop();

	if (ble.isHigh) {
			digitalWrite(7, HIGH);
			digitalWrite(3, HIGH);
			delay(2000);
			digitalWrite(7, LOW);
			digitalWrite(3, LOW);
			delay(500);
			for (uint8_t i = 0; i<10; i++) {
				digitalWrite(7, HIGH);
				delay(50);
				digitalWrite(7, LOW);
				digitalWrite(3, HIGH);
				delay(50);
				digitalWrite(3, LOW);
				delay(100);
			}
	} else {
//		digitalWrite(7, HIGH);
//		digitalWrite(3, HIGH);
//		delay(50);
//		digitalWrite(7, LOW);
//		digitalWrite(3, LOW);
//		delay(50);
//		digitalWrite(7, HIGH);
//		digitalWrite(3, HIGH);
//		delay(50);
		digitalWrite(7, LOW);
		digitalWrite(3, LOW);
	}

}

