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
#include <glm.hpp>
#include <gtx/vector_angle.hpp>
#include "Devices/Input.h"

bool calibration = false;

EventQueue* q;
Joystick* joystick;

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

	bl->fadeIn();

	auto input = new Input();
	joystick = new Joystick((gpio_num_t) JOY_H, (gpio_num_t) JOY_V);


	q = new EventQueue(12);
	Events::listen(Facility::Input, q);
	auto calibThread = new ThreadedClosure([](){
		Event item{};
		if(q->get(item, portMAX_DELAY)){
			auto data = (Input::Data*) item.data;
			if(data->btn == Input::Joy && data->action == Input::Data::Press){
				calibration = !calibration;
				printf(calibration ? "calibration start\n" : "calibration end\n");
				calibration ? joystick->startRangeCalib() : joystick->stopRangeCalib();
			}else if(data->btn == Input::Panic && data->action == Input::Data::Press){
				joystick->centerCalib();
				printf("center calibration\n");
			}
		}
		free(item.data);
	}, "CalibThread", 4096);
	calibThread->start();

	printf("Init done.\n");
	auto wifi = new WiFiSTA();
	Services.set(Service::WiFi, wifi);
	auto tcp = new TCPClient();
	Services.set(Service::TCP, tcp);

	auto comm = new Comm();

	auto pair = new PairService();
	while(pair->getState() == PairService::State::Pairing){
		delayMillis(1000);
	}
	if(pair->getState() == PairService::State::Success){
		while(1){
			auto vec = joystick->getPos();
			float val = std::clamp(glm::length(vec), 0.f, 1.f);
			float angle = 0;
			if(val > 0){
				vec = glm::normalize(vec);
				angle = glm::degrees(glm::angle(vec, { 0.0, 1.0 }));
				if(vec.x < 0){
					angle = 360 - angle;
				}
			}
			comm->sendDriveDir({ static_cast<uint16_t>(angle), val });
			printf("angle: %.2f, val: %.2f\n", angle, val);
			delayMillis(100);
		}
	}


}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
