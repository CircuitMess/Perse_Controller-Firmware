#ifndef PERSE_ROVER_WIFIAP_H
#define PERSE_ROVER_WIFIAP_H

#include <esp_event.h>
#include <esp_netif_types.h>
#include <semaphore>
#include <esp_wifi_types.h>
#include "Util/Threaded.h"
#include "Util/Hysteresis.h"

class WiFiSTA {
public:
	WiFiSTA();

	struct Event {
		enum { Connect, Disconnect } action;
		union {
			struct {
				uint8_t bssid[6];
				bool success;
			} connect;

			struct {
				uint8_t bssid[6];
			} disconnect;
		};
	};

	void connect();
	void disconnect();

	enum State { Connected, Connecting, Disconnected, Scanning, ConnAbort };
	State getState();

	enum ConnectionStrength {
		None = 4,
		VeryLow = 3,
		Low = 2,
		Medium = 1,
		High = 0
	};

	ConnectionStrength getConnectionStrength();

private:
	int getConnectionRSSI() const;

	esp_event_handler_instance_t evtHandler;
	void event(int32_t id, void* data);

	State state = Disconnected;
	std::binary_semaphore initSem{ 0 };

	Hysteresis hysteresis;

	static constexpr int ConnectRetries = 2;
	int connectTries;

	static esp_netif_t* createNetif();

	static constexpr uint16_t ScanListSize = 24;

	static wifi_ap_record_t* findNetwork(wifi_ap_record_t* ap_info, uint32_t numRecords);
};


#endif //PERSE_ROVER_WIFIAP_H
