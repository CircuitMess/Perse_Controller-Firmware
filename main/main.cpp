#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include "Pins.hpp"
#include "Devices/Display.h"
#include "Devices/Backlight.h"
#include "Util/stdafx.h"
#include "Util/Services.h"
#include "Periph/WiFiSTA.h"
#include "Util/Events.h"
#include "Services/TCPClient.h"
#include "Services/PairService.h"
#include "Services/Comm.h"

void init(){
	gpio_config_t cfg = {
			.pin_bit_mask = 0,
			.mode = GPIO_MODE_INPUT
	};
	gpio_config(&cfg);
	esp_log_level_set("WiFi_STA", ESP_LOG_VERBOSE);
	auto ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	auto settings = new Settings();
	Services.set(Service::Settings, settings);

	auto display = new Display();
	display->drawTest();

	auto bl = new Backlight(LEDC_CHANNEL_0);
	Services.set(Service::Backlight, bl);

	bl->fadeIn();

	printf("Init done.\n");
	auto wifi = new WiFiSTA();
	Services.set(Service::WiFi, wifi);
	auto tcp = new TCPClient();
	Services.set(Service::TCP, tcp);
	auto comm = new Comm();

	auto pair = new PairService();
	while(pair->getState() == PairService::State::Pairing){
		vTaskDelay(1000);
		printf("pairService state check\n");
	}
	if(pair->getState() == PairService::State::Success){
		printf("pair ok\n");
		vTaskDelay(1000);
		while(1){
			printf("send drive dir\n");
			comm->sendDriveDir(0b1010, 0b1111);
			vTaskDelay(1000/ portTICK_PERIOD_MS);
		}
	}else{
		printf("pair fail\n");
	}
}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
