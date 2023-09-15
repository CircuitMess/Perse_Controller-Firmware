#ifndef PERSE_ROVER_WIFIAP_H
#define PERSE_ROVER_WIFIAP_H

#include <esp_event.h>
#include <esp_netif_types.h>
#include <semaphore>

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

	enum State { Connected, Connecting, Disconnected };
	State getState();

private:
	esp_event_handler_instance_t evtHandler;
	void event(int32_t id, void* data);

	State state = Disconnected;
	std::binary_semaphore initSem{ 0 };

	static constexpr int ConnectRetries = 2;
	int connectTries;

	static esp_netif_t* createNetif();

};


#endif //PERSE_ROVER_WIFIAP_H
