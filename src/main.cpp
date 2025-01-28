#include <Arduino.h>
#include "MyBLE.h"

MyBLE ble;

void setup() {
	Serial.begin(115200);
	Serial.println("FLN BLE Workshop");

	ble.init();
}


void loop() {

}

