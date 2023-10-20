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
#include "Services/RoverState.h"
#include "Periph/SPIFFS.h"
#include "UISystem/IntroScreen/IntroScreen.h"
#include "UISystem/UIThread.h"

void init(){
	gpio_config_t cfg = {
			.pin_bit_mask = 0,
			.mode = GPIO_MODE_INPUT
	};
	gpio_config(&cfg);

	auto ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	const auto spiffs = new SPIFFS();

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

	auto rover = new RoverState();
	Services.set(Service::RoverState, rover);

	auto uiThread = new UIThread(*display);
	uiThread->startScreen(&IntroScreen::createScreen);
	uiThread->start();
}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
