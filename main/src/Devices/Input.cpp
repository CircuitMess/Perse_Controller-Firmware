#include "Input.h"
#include "Util/Events.h"
#include <Util/stdafx.h>
#include <Pins.hpp>
#include <driver/gpio.h>

// button index -> GPIO port
const std::unordered_map<Input::Button, gpio_num_t> Input::PinMap{
		{ Pair, (gpio_num_t) BTN_PAIR },
#ifdef CTRL_TYPE_MISSIONCTRL
		{ Panic,    (gpio_num_t) BTN_PANIC },
		{ Joy,      (gpio_num_t) BTN_JOY },
		{ EncArm,   (gpio_num_t) BTN_ENC_ARM },
		{ EncPinch, (gpio_num_t) BTN_ENC_PINCH },
		{ EncCam,   (gpio_num_t) BTN_ENC_CAM },
		{ SwArm,    (gpio_num_t) SW_ARM },
		{ SwLight,  (gpio_num_t) SW_LIGHT },
#elifdef CTRL_TYPE_BASIC
		{ Up, (gpio_num_t) BTN_UP },
		{ Down, (gpio_num_t) BTN_DOWN },
		{ Left, (gpio_num_t) BTN_LEFT },
		{ Right, (gpio_num_t) BTN_RIGHT },
		{ Mode, (gpio_num_t) BTN_MODE },
#endif
};

const std::unordered_map<Input::Button, const char*> Input::PinLabels{
		{ Pair, "Pair" },
#ifdef CTRL_TYPE_MISSIONCTRL
		{ Panic,    "Panic" },
		{ Joy,      "Joy" },
		{ EncArm,   "EncArm" },
		{ EncPinch, "EncPinch" },
		{ EncCam,   "EncCam" },
		{ SwArm,    "SwArm" },
		{ SwLight,  "SwLight" },
#elifdef CTRL_TYPE_BASIC
		{ Up, "Up" },
		{ Down, "Down" },
		{ Left, "Left" },
		{ Right, "Right" },
		{ Mode, "Mode" },
#endif
};

Input::Input() : Threaded("Input", 2048, 6){
	auto mask = 0ULL;
	for(const auto& pair : PinMap){
		const auto port = pair.first;
		const auto pin = pair.second;

		btnState[port] = false;
		dbTime[port] = 0;

		mask |= (1ULL << pin);
	}

	gpio_config_t io_conf = {
			.pin_bit_mask = mask,
			.mode = GPIO_MODE_INPUT,
			.pull_up_en = GPIO_PULLUP_ENABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&io_conf);

	start();
}

Input::~Input(){
	stop();
}

bool Input::getState(Input::Button btn){
	if(!btnState.contains(btn)) return false;
	return btnState.at(btn);
}

void Input::loop(){
	scan();
	vTaskDelay(SleepTime);
}

void Input::scan(){
	for(const auto& pair : PinMap){
		const auto port = pair.first;
		const auto pin = pair.second;

		bool state = gpio_get_level(pin);

		if(state){
			released(port);
		}else{
			pressed(port);
		}
	}
}

void Input::pressed(Input::Button btn){
	if(btnState[btn]){
		dbTime[btn] = 0;
		return;
	}

	auto t = millis();

	if(dbTime[btn] == 0){
		dbTime[btn] = t;
		return;
	}else if(t - dbTime[btn] < DebounceTime){
		return;
	}

	btnState[btn] = true;
	dbTime[btn] = 0;

	Data data = {
			.btn = btn,
			.action = Data::Press
	};
	Events::post(Facility::Input, data);
}

void Input::released(Input::Button btn){
	if(!btnState[btn]){
		dbTime[btn] = 0;
		return;
	}

	auto t = millis();

	if(dbTime[btn] == 0){
		dbTime[btn] = t;
		return;
	}else if(t - dbTime[btn] < DebounceTime){
		return;
	}

	btnState[btn] = false;
	dbTime[btn] = 0;

	Data data = {
			.btn = btn,
			.action = Data::Release
	};
	Events::post(Facility::Input, data);
}
