#include "WiFiSTA.h"
#include "Util/Events.h"
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_wifi_default.h>
#include <string>
#include <cstring>
#include <esp_log.h>

static const char* TAG = "WiFi_STA";

static std::string mac2str(uint8_t ar[]){
	std::string str;
	for(int i = 0; i < 6; ++i){
		char buf[3];
		sprintf(buf, "%02X", ar[i]);
		str += buf;
		if(i < 5) str += ':';
	}
	return str;
}

WiFiSTA::WiFiSTA(){
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, [](void* arg, esp_event_base_t base, int32_t id, void* data){
		if(base != WIFI_EVENT) return;
		auto wifi = static_cast<WiFiSTA*>(arg);
		wifi->event(id, data);
	}, this, &evtHandler);

	ESP_ERROR_CHECK(esp_netif_init());
	createNetif();

	wifi_init_config_t cfg_wifi = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg_wifi);

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());

	initSem.acquire();
}

void WiFiSTA::event(int32_t id, void* data){
	ESP_LOGD(TAG, "Evt %ld", id);

	if(id == WIFI_EVENT_STA_START){
		initSem.release();
	}else if(id == WIFI_EVENT_STA_CONNECTED){
		auto event = (wifi_event_sta_connected_t*) data;
		const auto mac = mac2str(event->bssid);
		ESP_LOGI(TAG, "connected bssid %s, chan=%d", mac.c_str(), event->channel);

		const esp_netif_ip_info_t ip = {
				.ip =		{ .addr = esp_ip4addr_aton("11.0.0.2") },
				.netmask =	{ .addr = esp_ip4addr_aton("255.255.255.0") },
				.gw =		{ .addr = esp_ip4addr_aton("11.0.0.1") },
		};
		const auto netif = esp_netif_get_default_netif();
		esp_netif_set_ip_info(netif, &ip);

		if(state == Connecting){
			state = Connected;

			Event evt { .action = Event::Connect, .connect = { .success = true } };
			memcpy(evt.connect.bssid, event->bssid, 6);
			Events::post(Facility::WiFiSTA, evt);
		}else if(state == ConnAbort){
			esp_wifi_disconnect();
		}

	}else if(id == WIFI_EVENT_STA_DISCONNECTED){
		auto event = (wifi_event_sta_disconnected_t*) data;
		const auto mac = mac2str(event->bssid);
		ESP_LOGI(TAG, "disconnected bssid %s, reason=%d", mac.c_str(), event->reason);

		if(state == Connecting){
			if(++connectTries <= ConnectRetries){
				esp_wifi_connect();
			}else{
				state = Disconnected;
				Event evt { .action = Event::Connect, .connect = { .success = false } };
				Events::post(Facility::WiFiSTA, evt);
			}

			return;
		}

		if(state == ConnAbort){
			state = Disconnected;
			Event evt{ .action = Event::Connect, .connect = { .success = false } };
			Events::post(Facility::WiFiSTA, evt);
			return;
		}

		state = Disconnected;

		Event evt { .action = Event::Disconnect };
		memcpy(evt.disconnect.bssid, event->bssid, 6);
		Events::post(Facility::WiFiSTA, evt);
	}else if(id == WIFI_EVENT_SCAN_DONE){
		if(state != Scanning) return;

		auto number = ScanListSize;
		ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
		ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&number));
		auto ret = findNetwork(ap_info, number);

		if(ret != nullptr){
			state = Connecting;
			connectTries = 0;

			wifi_config_t cfg_sta = {
					.sta = {
							.password = "RoverRover"
					}
			};
			strncpy((char*) cfg_sta.sta.ssid, (char*) ret->ssid, 32);
			ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg_sta));

			esp_wifi_connect();
		}else{
			state = Disconnected;
			Event evt{ .action = Event::Connect, .connect = { .success = false } };
			Events::post(Facility::WiFiSTA, evt);
		}
	}
}

esp_netif_t* WiFiSTA::createNetif(){
	esp_netif_inherent_config_t base{};
	memcpy(&base, ESP_NETIF_BASE_DEFAULT_WIFI_AP, sizeof(esp_netif_inherent_config_t));
	base.flags = (esp_netif_flags_t) ((base.flags & ~(ESP_NETIF_DHCP_SERVER | ESP_NETIF_DHCP_CLIENT | ESP_NETIF_FLAG_EVENT_IP_MODIFIED)) | ESP_NETIF_FLAG_GARP);

	const esp_netif_ip_info_t ip = {
			.ip =		{ .addr = esp_ip4addr_aton("11.0.0.2") },
			.netmask =	{ .addr = esp_ip4addr_aton("255.255.255.0") },
			.gw =		{ .addr = esp_ip4addr_aton("11.0.0.1") },
	};
	base.ip_info = &ip;

	esp_netif_config_t cfg = ESP_NETIF_DEFAULT_WIFI_AP();
	cfg.base = &base;

	esp_netif_t* netif = esp_netif_new(&cfg);
	assert(netif);
	esp_netif_set_default_netif(netif);

	esp_netif_attach_wifi_station(netif);
	esp_wifi_set_default_wifi_sta_handlers();

	return netif;
}

void WiFiSTA::connect(){
	if(state != Disconnected) return;

	state = Scanning;
	memset(ap_info, 0, sizeof(ap_info));
	esp_wifi_scan_start(&ScanConfig, false);
}

WiFiSTA::State WiFiSTA::getState(){
	return state;
}

void WiFiSTA::disconnect(){
	if(state == Disconnected) return;

	switch(state){
		case Connected:
			esp_wifi_disconnect();
			break;
		case Connecting:
			state = ConnAbort;
			esp_wifi_disconnect();
			break;
		case Disconnected:
			break;
		case Scanning:
			esp_wifi_scan_stop();
			break;
		case ConnAbort:
			break;
	}
}

wifi_ap_record_t* WiFiSTA::findNetwork(wifi_ap_record_t* ap_info, uint32_t numRecords){
	for(int i = 0; i < numRecords; ++i){
		if(strncmp((const char*) ap_info[i].ssid, "Perseverance Rover", 18) == 0){
			return &ap_info[i];
		}
	}
	return nullptr;
}
