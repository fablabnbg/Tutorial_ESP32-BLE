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

#include <vector>
#include <memory>


class MyBLE: public BLEAdvertisedDeviceCallbacks, BLEClientCallbacks {

public:
	MyBLE();
	void init();
	virtual void onResult(BLEAdvertisedDevice advertisedDevice);

	bool connectToAdvDev(size_t idx);

	virtual void onConnect(BLEClient *pClient);
	virtual void onDisconnect(BLEClient *pClient);

	bool isHigh = false;

private:
	BLEScan* pScan = 0;

	static const BLEUUID serviceUUIDExposure;
	static const BLEUUID serviceUUID;

	static const BLEUUID serviceUUIDHRM;
	static const BLEUUID charUUIDHRMPulse;

	static const BLEUUID serviceUUIDCSC;
	static const BLEUUID charUUIDCSCMeasurement;

	static const BLEUUID serviceUUIDBat;
	static const BLEUUID charUUIDBat;

	TaskHandle_t scanTaskHandle = nullptr;

	void scanLoop();

	void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

	bool addNotify(BLEClient* client);

	bool connect = false;
	//BLEAddress connectAddr = BLEAddress("0");
	std::vector<std::unique_ptr<BLEAdvertisedDevice>> connectDevices;		// Hier legen wir einen Vector an, um gefundene Devices zwischenzuspeichern. Um die Speicherverwaltung zu vereinfachen,
																			// verwenden wir einen Unique-Pointer. Das bedeutet, der Pointer darf nicht "weitergegeben" werden. Dafür kann der Speicherbereich
																			// dann automatisch freigegeben werden, wenn der unique_ptr gelöscht wird, z. B. beim Entfernen aus dem Vector.
	std::vector<std::unique_ptr<BLEClient>> clients;						// Und einen Vector für unsere (mit einem Device) verbundenen Clients


	bool filterDevice(BLEAdvertisedDevice& dev);
	bool isAlreadyConnected(BLEAdvertisedDevice& newDevice);
};

#endif /* SRC_MYBLE_H_ */
