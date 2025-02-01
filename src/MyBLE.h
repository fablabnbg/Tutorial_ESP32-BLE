/*
 * MyBLE.h
 *
 *  Created on: 28.01.2025
 *      Author: ian
 */

#ifndef SRC_MYBLE_H_
#define SRC_MYBLE_H_

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
//#include <BLEUUID.h>

class MyBLE: public BLEAdvertisedDeviceCallbacks, BLEClientCallbacks {

public:
	MyBLE();
	void init();
	virtual void onResult(BLEAdvertisedDevice advertisedDevice);

	bool connectToAddr(BLEAddress& addr);
	virtual void onConnect(BLEClient *pClient);
	virtual void onDisconnect(BLEClient *pClient);

private:
	BLEScan* pScan = 0;

	static const BLEUUID serviceUUIDExposure;
	static const BLEUUID serviceUUID;

	static const BLEUUID serviceUUIDHRM;
	static const BLEUUID charUUIDHRMPulse;

	TaskHandle_t scanTaskHandle = nullptr;

	void scanLoop();

	void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

	bool connect = false;
	BLEAddress connectAddr = BLEAddress("0");
	BLEClient* pClient = 0;

};

#endif /* SRC_MYBLE_H_ */
