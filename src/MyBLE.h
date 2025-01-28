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

class MyBLE: public BLEAdvertisedDeviceCallbacks, BLEExtAdvertisingCallbacks {

public:
	MyBLE();
	void init();
	virtual void onResult(BLEAdvertisedDevice advertisedDevice);
	virtual void onResult(esp_ble_gap_ext_adv_reprot_t report);


private:
	BLEScan* pScan = 0;

};

#endif /* SRC_MYBLE_H_ */
