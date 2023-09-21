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
#include "LV_Interface/LVGL.h"
#include "LV_Interface/InputLVGL.h"
#include "LV_Interface/FSLVGL.h"
#include "Services/PairService.h"
#include "Devices/Joystick.h"
#include "Services/Comm.h"
#include "Devices/Input.h"
#include "Screens/DriveScreen.h"



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

	auto settings = new Settings();
	Services.set(Service::Settings, settings);

	auto display = new Display();
	display->drawTest();

	auto bl = new Backlight(LEDC_CHANNEL_0);
	Services.set(Service::Backlight, bl);

	auto lvgl = new LVGL(*display);
	auto lvglInput = new InputLVGL();
	auto fs = new FSLVGL('S');

	printf("Init done.\n");
	auto wifi = new WiFiSTA();
	Services.set(Service::WiFi, wifi);
	auto tcp = new TCPClient();
	Services.set(Service::TCP, tcp);

	auto comm = new Comm();
	Services.set(Service::Comm, comm);
	bl->fadeIn();

	auto input = new Input();
	auto joystick = new Joystick((gpio_num_t) JOY_H, (gpio_num_t) JOY_V);

	lvgl->startScreen([&joystick](){ return std::make_unique<DriveScreen>(joystick); });

	lvgl->start();

}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
