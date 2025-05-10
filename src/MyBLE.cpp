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

const BLEUUID MyBLE::serviceUUIDCSC = BLEUUID((uint16_t)0x1816);
const BLEUUID MyBLE::charUUIDCSCMeasurement = BLEUUID((uint16_t)0x2A5B);

const BLEUUID MyBLE::serviceUUIDBat = BLEUUID((uint16_t) 0x180F);
const BLEUUID MyBLE::charUUIDBat = BLEUUID((uint16_t) 0x2A19);

MyBLE::MyBLE() {
	// Achtung! Der Konstruktur wird vor dem Arduino setup() aufgerufen. Und vielleicht auch vor dem Konstruktor von "typischen" Arduino-Objekten.
	// 			Deswegen ist es gefährlich hier schon andere Funktionen oder Methoden aufzurufen.
	// --> besser eine init() oder setup() Funktion verwenden, die vom main- setup() aufgerufen wird
}

void MyBLE::init() {
	BLEDevice::init("FLNTest");
	xTaskCreate(+[](void* thisInstance){((MyBLE*)thisInstance)->scanLoop();}, "BLEScanUndConnectTask", 3072, this, 5, &scanTaskHandle);
	pinMode(8, OUTPUT);
	digitalWrite(8, HIGH);
}

void MyBLE::scanLoop() {
	const int scan_duration = 8;
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

		bool reScan = false;
		// Nach dem Scan prüfen, ob (neue) Geräte zum verbinde da sind:
		if (connectDevices.size() > 0) {
			for (size_t i = 0; i < connectDevices.size(); i++) {
				connectToAdvDev(i);
			}
		} else {
			Serial.println("🙈 kein Gerät zum Verbinden gefunden");
			reScan = true;
		}
		while (!reScan) {  // Reconnect - Schleife. Innerhalb der Schleife wird geprüft, ob mindestens 1 Gerät neu verbunden werden muss.

			vTaskDelay(7 * 1000 / portTICK_PERIOD_MS);	// Warte 15 Sec
			if (clients.size() == 0) reScan = true;
			for (size_t i = 0; i < clients.size(); i++) {
				Serial.printf("Prüfe Client %x\n", i);
				// Zugriff auf den BLEClient
				BLEClient *client = clients[i].get();
				if (client->isConnected()) {
					Serial.printf("🆗 Gerät %x weiterhin verbunden\n", i);
					// !!!! getRssi blockiert unendlich, wenn das Gerät es nicht unterstützt, die rssi über die Verbindung abzufragen.
					//Serial.printf("🆗 Verbundenes Gerät %x mit %d dBm\n", i, client->getRssi());

				} else {
					Serial.printf("🛑 Verbindung zu Gerät %i verloren!\n", i);
					reScan = true;
				}
			}
		}  // ----------------- Ende der Reconnect-Schleife
		Serial.println(" Verbindung zu (mindestens) 1 Gerät verloren");
		Serial.println("DEBUG: ScanLoopEnde (vor Delay)");
		vTaskDelay((scan_duration*2) * 1000 / portTICK_PERIOD_MS);	// Warte die doppelte Zeit
																	// Einige Geräte, zum Beispiel manche "Cycling Speed & Cadence" sind nach der Aktivierung (durch Bewegung) nur recht kurz (z. B. 30sec)
																	// aktiv. Deswegen sollte die Zeit nicht zu lange sein. Dabei muss auch die Zeit berücksichtigt werden, die es braucht, um ggf. andere
																	// gefundene Geräte zu verbinden.
		Serial.println("DEBUG: ScanLoopEnde (nach Delay)");
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
	}
	if (filterDevice(advertisedDevice)) {
		Serial.println("✅ HRM Service gefunden - Verbindungsaufbau");
		connect = true;
		//connectDevices.push_back(std::make_unique<BLEAdvertisedDevice>(advertisedDevice));  // geht erst mit C++14, aber die Toolchain nutzt C++11 (auch -std=C++14 als Buildflag reicht nicht)
		connectDevices.push_back(std::unique_ptr<BLEAdvertisedDevice>(new BLEAdvertisedDevice(advertisedDevice)));// da advertisedDevice auf dem Stack liegt, müssen wir eine Kopie anlegen.
	}
//	if (connect) {
//		pScan->stop();	// Scan stoppen und sofort connecten
//	}
}

bool MyBLE::connectToAdvDev(size_t idx) {
	bool rc = false;
	Serial.printf("DEBUG -- %d device in adv-Liste\n", connectDevices.size());

	for (const auto& devicePtr : connectDevices) {
		Serial.println("DEBUG -- ITERATOR START");
	    BLEAdvertisedDevice* device = devicePtr.get();
	    //BLEAdvertisedDevice* device = it->get();
		Serial.printf("DEBUG -- Got device at 0x%x\n", device);

		// Neuen Client anlegen
		auto client = std::unique_ptr<BLEClient>(new BLEClient());		// unique_ptr hier bedeutet, dass der BLEClient automatisch wieder gelöscht wird, wenn client aus dem Scope geht
    																// (außer wenn vorher die Pointer-Ownership verschoben wird)
		BLEAddress addr = device->getAddress();
		Serial.printf("🚀 Verbindungsaufbau zu %s\n", addr.toString().c_str());
		client->setClientCallbacks(this);
		//bool connected = pClient->connect(addr, BLE_ADDR_TYPE_PUBLIC);		// Manche devices benötigen auch BLE_ADDR_TYPE_RANDOM - warum auch immer?!

		// Der connect()-Aufruf ist blockierend - der aktuelle Task steht hier also bis die Verbindung erfolgreich ist (oder abgebrochen wird).
		bool connected = client->connect(device);		// Device aus der gespeicherten Liste

		//connectDevices.erase(it);
		if (!connected) {
			Serial.printf("❌ Can't connect to device %s.\n", addr.toString().c_str());
			continue;
		}
		// Prüfen auf BatteryService
		BLERemoteService *pRemoteServiceBat = client->getService(serviceUUIDBat);
		if (pRemoteServiceBat == nullptr) {
			Serial.println("⚠️ Kein Batterieservice gefunden");
		} else {
			BLERemoteCharacteristic *pRemoteCharacteristicBat = pRemoteServiceBat->getCharacteristic(charUUIDBat);
			if (pRemoteCharacteristicBat == nullptr) {
				Serial.println("😵‍💫 Batterieservice gefunden, aber keine Characteristics ?!?!");
			} else {
				Serial.println("🔋 Service für Batterieladung gefunden");
				// Batterie-Characteristic wollen wir per READ, also direkt auslesen.
				uint8_t batLevel = pRemoteCharacteristicBat->readUInt8();
				Serial.printf("🔋 Ladezustand der Batterie %d %%\n", batLevel);
			}
		}
		if(addNotify(client.get())) rc = true;
		clients.push_back(std::move(client));

	}
	Serial.println("DEBUG - Delete advertised list");
	connectDevices.clear();
	return rc;
	}


bool MyBLE::addNotify(BLEClient* client) {
	BLEUUID uuidTemp = serviceUUIDHRM;		// Non-const Kopie der aktuell verwendenten UUID
	BLERemoteService* pRemoteService = client->getService(uuidTemp);
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

	return true;
}

void MyBLE::notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
	Serial.println("🛎️ Notify Callback!");
	if (length < 2)	return;
	uint8_t flags = pData[0];	// Erstes Byte sind die Flags
	uint16_t hr = pData[1];		// Zweites Byte ist die Heart Rate (Kommt als 1 Byte oder 2 Byte-Wert. Zunächst ertes Byte einlesen
	Serial.printf("Debug: HR Data mit Länge %ul. Ersten 8bit: 0x%x FLAG: %X\n", length,	hr, flags);
	if ((pData[0] & 1)) {		// Flag-Bit 0 ist Flag, ob 16 bit or 8bit
		hr |= (pData[2] << 8);
	}
	Serial.printf("💓: %d ❤ pro minute\n", hr);
	isHigh = (hr >= 147) && (hr < 152);
}


void MyBLE::onConnect(BLEClient *pClient) {
	Serial.printf("☎️ Connect %s!\n", pClient->getPeerAddress().toString().c_str());
	connect = false;
    digitalWrite(8, LOW);
}

void MyBLE::onDisconnect(BLEClient *pClient) {
	Serial.println("☎️ Disconnect!");
    digitalWrite(8, HIGH);
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it->get() == pClient) {
            Serial.printf("Removing disconnected device: %s\n", pClient->getPeerAddress().toString().c_str());
            clients.erase(it);
            return;
        }
    }
    Serial.println("Disconnected client not found in the list.");
}

bool MyBLE::filterDevice(BLEAdvertisedDevice& dev) {
	if (isAlreadyConnected(dev)) {
		Serial.println("Bereits verbunden");
		return false;
	}
	// Filter nach Service
	for (size_t i = 0 ; i < dev.getServiceUUIDCount(); i++) {
		Serial.printf("Prüfen auf Service %x\n", i);
		Serial.flush();
		if (dev.getServiceUUID(i).equals(serviceUUIDHRM)) {
			Serial.println("Filter: ❤️ HRM gefunden");
			return true;
		}
//		if (dev.getServiceUUID(i).equals(serviceUUIDCSC)) {
//			Serial.println("Filter: 🚴 CSC gefunden");
//			return true;
//		}
	}
	Serial.println("Filter: 💤 Gerät uninteressant");
	return false;
}


bool MyBLE::isAlreadyConnected(BLEAdvertisedDevice& newDevice) {
    auto it = std::find_if(clients.begin(), clients.end(),
        [&newDevice](const std::unique_ptr<BLEClient>& client) {
            return client->getPeerAddress().equals(newDevice.getAddress());
        });

    return it != clients.end();  // true, wenn Gerät bereits verbunden ist
}


