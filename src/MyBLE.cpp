/*
 * MyBLE.cpp
 *
 *  Created on: 28.01.2025
 *      Author: ian
 */

#include <MyBLE.h>


const BLEUUID MyBLE::serviceUUIDExposure = BLEUUID((uint16_t) 0xFD6F);

MyBLE::MyBLE() {
	// Achtung! Der Konstruktur wird vor dem Arduino setup() aufgerufen. Und vielleicht auch vor dem Konstruktor von "typischen" Arduino-Objekten.
	// 			Deswegen ist es gefährlich hier schon andere Funktionen oder Methoden aufzurufen.
	// --> besser eine init() oder setup() Funktion verwenden, die vom main- setup() aufgerufen wird
}

void MyBLE::init() {
	BLEDevice::init("FLNTest");

	// Init Scan
	pScan = BLEDevice::getScan();	// Singleton: Gibt Pointer auf das einzige und dauerhaft (spätestens ab ersten Aufruf) existierende Scan-Objet zurück.
									// --> Löschen nicht nötig und nicht sinnvoll (potentiell gefährlich, da man es auch für "andere" löscht.)
									// --> Speichern des Pointers nicht nötig, kann aber hilfreich sein. Man kann sich den Pointer aber jederzeit mit BLEScan::getScan() "wiederholen".

	pScan->setAdvertisedDeviceCallbacks(this);		// setze das Objekt, welches über gefundene Devices informiert wird. Dies sind wir selbst, also "this".

	pScan->setActiveScan(true); //active scan uses more power, but get results faster
	pScan->setInterval(100);
	pScan->setWindow(99);  // less or equal setInterval value

	// Async Scan starten
	pScan->start(30, nullptr, false);	// Ein Aufruf mit result-callback wird asychron ausgeführt. Auch wenn das callback ein nullptr ist.
										// Ihr werdet über results einzeln informiert und könnt ich (grob) auf die Zeit verlassen.
										// Es ist also nicht unbedingt nötig auch das Callback abzuwarten, sondern könnt den Scan auch so zyklisch neu starten

}

void MyBLE::onResult(BLEAdvertisedDevice advertisedDevice) {	// Call by value: advertisedDevice gehört Euch (liegt aber eh auf dem Stack)
//	if (advertisedDevice.getServiceUUID().equals(serviceUUIDExposure)) {
//		Serial.print("Ignore device - exposure notification");
//		return;
//	}


	// Gefundene Devices ausgeben
	Serial.printf("🔵 Neues Device: %s\n", advertisedDevice.getName().c_str());
	Serial.printf("           RSSI: %ddb\n", advertisedDevice.getRSSI());
	Serial.printf("        Address: %s\n", advertisedDevice.getAddress().toString().c_str());
	Serial.printf("   ServiceCount: %d\n", advertisedDevice.getServiceUUIDCount());

	for (uint8_t c = 0; c < advertisedDevice.getServiceUUIDCount(); c++) {
		BLEUUID uuid = advertisedDevice.getServiceUUID(c);
		Serial.printf("        ID: %s\n", uuid.toString().c_str());
	}
}
