/*
 * MyBLE.cpp
 *
 *  Created on: 28.01.2025
 *      Author: ian
 */

#include <MyBLE.h>


const BLEUUID MyBLE::serviceUUIDExposure = BLEUUID((uint16_t) 0xFD6F);
//const BLEUUID MyBLE::serviceUUID[DEV_COUNT] = { BLEUUID((uint16_t)0x180D), BLEUUID((uint16_t)0x1816), BLEUUID((uint16_t)0x1816), BLEUUID("e62efa94-afa8-11ed-afa1-0242ac120002"), BLEUUID("71C1E128-D92F-4FA8-A2B2-0F171DB3436C")};
const BLEUUID MyBLE::serviceUUIDHRM =  BLEUUID((uint16_t)0x180D);
const BLEUUID MyBLE::charUUIDHRMPulse = BLEUUID((uint16_t)0x2A37);

MyBLE::MyBLE() {
	// Achtung! Der Konstruktur wird vor dem Arduino setup() aufgerufen. Und vielleicht auch vor dem Konstruktor von "typischen" Arduino-Objekten.
	// 			Deswegen ist es gefährlich hier schon andere Funktionen oder Methoden aufzurufen.
	// --> besser eine init() oder setup() Funktion verwenden, die vom main- setup() aufgerufen wird
}

void MyBLE::init() {
	BLEDevice::init("FLNTest");
	xTaskCreate(+[](void* thisInstance){((MyBLE*)thisInstance)->scanLoop();}, "BLEScanUndConnectTask", 3072, this, 5, &scanTaskHandle);
}

void MyBLE::scanLoop() {
	const int scan_duration = 10;
	Serial.println("👩‍🏭 New Task: BLEScanUndConnectTask");
	// Init Scan
	pScan = BLEDevice::getScan();	// Singleton: Gibt Pointer auf das einzige und dauerhaft (spätestens ab ersten Aufruf) existierende Scan-Objet zurück.
									// --> Löschen nicht nötig und nicht sinnvoll (potentiell gefährlich, da man es auch für "andere" löscht.)
									// --> Speichern des Pointers nicht nötig, kann aber hilfreich sein. Man kann sich den Pointer aber jederzeit mit BLEScan::getScan() "wiederholen".

	pScan->setAdvertisedDeviceCallbacks(this);		// setze das Objekt, welches über gefundene Devices informiert wird. Dies sind wir selbst, also "this".

	pScan->setActiveScan(true); //active scan uses more power, but get results faster
	pScan->setInterval(100);
	pScan->setWindow(99);  // less or equal setInterval value

	while (true) {
		Serial.println("📡 Scan started");
		//pScan->start(scan_duration, nullptr, false);	// da ein Callback festgelegt wird, wird der Scan asychron gestart. Da der Callback ein Null-Pointer ist, wird man nicht informiert.
		BLEScanResults results = pScan->start(scan_duration);					// start ohne Callback: Blocking. Da wir einen eigenen Task für das Scannen nutzen, stört uns das nicht.

		Serial.println("🏁 Scan finished");

		if (connect && connectToAddr(connectAddr)) {
			while(pClient->isConnected()) {
				Serial.println("🆗 Device ist noch verbunden");
				vTaskDelay(20 * 1000 / portTICK_PERIOD_MS);	// Warte 20 Sec
			}
			Serial.println("🛑 Verbindung verloren");


		}
		vTaskDelay((scan_duration*2) * 1000 / portTICK_PERIOD_MS);	// Warte die doppelte Zeit
																	// Einige Geräte, zum Beispiel manche "Cycling Speed & Cadence"
	}
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
		if (uuid.equals(serviceUUIDHRM)) {
			Serial.println("✅ HRM Service gefunden - Verbindungsaufbau");
			connect = true;
			connectAddr = advertisedDevice.getAddress();
		}
	}
	if (connect) {
		pScan->stop();	// Scan stoppen und sofort connecten
	}
}

bool MyBLE::connectToAddr(BLEAddress& addr) {

	pClient = new BLEClient();
	pClient->setMTU(256);

	Serial.printf("🚀 Verbindungsaufbau zu %s\n", addr.toString().c_str());

	pClient->setClientCallbacks(this);
	bool connected = pClient->connect(addr, BLE_ADDR_TYPE_PUBLIC);		// Manche devices benötigen auch BLE_ADDR_TYPE_RANDOM - warum auch immer?!
	// Der Aufruf ist blockierend - der aktuelle Task steht hier also bis die Verbindung erfolgreich ist (oder abgebrochen wird).


	if (!connected) {
		Serial.printf("❌ Can't connect to device %s.\n", addr.toString().c_str());
		return false;
	}

	BLEUUID uuidTemp = serviceUUIDHRM;		// Non-const Kopie der aktuell verwendenten UUID
	BLERemoteService* pRemoteService = pClient->getService(uuidTemp);
	if (pRemoteService == nullptr) {
		Serial.printf("⚠️ Cannot find HRM remote service %s\n", uuidTemp.toString().c_str());
		return false;
	}

	// Service gefunden - der besteht jetzt aus verschiendene "Characteristics".
	//   Der HRM-Service zum Beispiel aus dem aktuellen Puls, aber auch Meta-Informationen, wie ob es ein Brust- oder Armgurt ist bzw. am Handgelenk.


	uuidTemp = charUUIDHRMPulse;	// wieder non-const kopieren
	BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUIDHRMPulse);
	if (pRemoteCharacteristic == nullptr) {
		Serial.printf("⚠️ Cannot find HRM Pulse remote characteristic");
		return false;
	}

	// Ok, wir haben die interessante Characteristic gefunden. Jetzt melden wir uns an, regelmäßig informiert zu werden.

	pRemoteCharacteristic->registerForNotify([&](BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {notifyCallback(pBLERemoteCharacteristic, pData, length, isNotify);});

	//	bclog.logf(BCLogger::Log_Debug, BCLogger::TAG_BLE, "🔵%s Notify registered\n", DEV_EMOJI[ctype]);
//	storeAdress(ctype, *pServerAddress[ctype]);
//
//	if (hasBatService[ctype]) {
//		BLERemoteService *pRemoteServiceBat = pClient[ctype]->getService(serviceUUIDBat);
//		if (pRemoteServiceBat == nullptr) {
//			bclog.logf(BCLogger::Log_Warn, BCLogger::TAG_BLE, "🔵⚠️ Cannot find battery remote service %s", DEV_EMOJI[ctype]);
//			hasBatService[ctype] = false;
//			return false;
//		}
//		BLERemoteCharacteristic *pRemoteCharacteristicBat = pRemoteServiceBat->getCharacteristic(charUUIDBat);
//		if (pRemoteCharacteristicBat == nullptr) {
//			bclog.logf(BCLogger::Log_Warn, BCLogger::TAG_BLE, "🔵⚠️ Cannot find battery remote characteristics %s", DEV_EMOJI[ctype]);
//			hasBatService[ctype] = false;		//TODO set to high again for disconnect?
//			return false;
//		}
//		batLevel[ctype] = pRemoteCharacteristicBat->readUInt8();
//		bclog.logf(BCLogger::Log_Info, BCLogger::TAG_BLE, "%s battery level %d %%", DEV_EMOJI[ctype], batLevel[ctype]);
//	}
	return true;

}

void MyBLE::notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
	Serial.println("🛎️ Notify Callback!");
	if (length < 2)	return;
	uint8_t flags = pData[0];	// Erstes Byte sind die Flags
	uint16_t hr = pData[1];		// Zweites Byte ist die Heart Rate (Kommt als 1 Byte oder 2 Byte-Wert. Zunächst ertes Byte einlesen
	Serial.printf("Debug: HR Data mit Länge %ul. Ersten 8bit: %x FLAG: %X\n", length,	hr, flags);
	if ((pData[0] & 1)) {		// Flag-Bit 0 ist Flag, ob 16 bit or 8bit
		hr |= (pData[2] << 8);
	}
	Serial.printf("💓: %d ❤ pro minute\n", hr);
}

void MyBLE::onConnect(BLEClient *pClient) {
	Serial.println("☎️ Connect!");
	connect = false;
}

void MyBLE::onDisconnect(BLEClient *pClient) {
	Serial.println("☎️ Disconnect!");
	connectAddr = BLEAddress("0");

}
