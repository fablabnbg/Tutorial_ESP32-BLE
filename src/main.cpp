#include <Arduino.h>

#include "MyBLE.h"
#include "CommandHandler.h"

MyBLE ble;
extern CommandHandler cmdHandler;

void setup() {
	Serial.begin(115200);
	Serial.println("FLN BLE Workshop");
	ble.init();
	cmdHandler.setup();
}

void loop() {
	cmdHandler.loop();

}

