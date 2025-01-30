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

class MyBLE: public BLEAdvertisedDeviceCallbacks {

public:
	MyBLE();
	void init();
	virtual void onResult(BLEAdvertisedDevice advertisedDevice);

private:
	BLEScan* pScan = 0;

	static const BLEUUID serviceUUIDExposure;

};

#endif /* SRC_MYBLE_H_ */
