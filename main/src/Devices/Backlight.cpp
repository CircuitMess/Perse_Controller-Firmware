#include "Backlight.h"
#include <algorithm>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cmath>
#include "Util/Services.h"
#include "Util/stdafx.h"
#include "Pins.hpp"

Backlight::Backlight(ledc_channel_t channel) : pwm(PIN_BL, channel, true){
	pwm.detach();
}

void Backlight::setBrightness(uint8_t level){
	pwm.setDuty(mapDuty(level));
}

constexpr uint8_t Backlight::mapDuty(uint8_t level){
	level = std::clamp(level, (uint8_t) 0, (uint8_t) 100);

	float fVal = map((float) level, 0, 100, MinDuty, 100);

	fVal /= 100.0f;
	fVal = std::pow(fVal, 2.0f);
	fVal = std::round(fVal * 100.0f);

	return (uint8_t) std::clamp(fVal, 0.0f, 100.0f);
}

void Backlight::fadeIn(){
	if(state) return;
	state = true;

	const auto settings = (Settings*) Services.get(Service::Settings);
	if(settings == nullptr){
		return;
	}

	const auto brightness = settings->get().screenBrightness;
	const auto mapped = mapDuty(brightness);

	pwm.attach();

	for(int i = 0; i <= 100; i++){
		uint8_t val = map((double) i, 0, 100, 0, mapped);
		pwm.setDuty(val);
		vTaskDelay(FadeDelay / portTICK_PERIOD_MS);
	}
}

void Backlight::fadeOut(){
	if(!state) return;
	state = false;

	const auto settings = (Settings*) Services.get(Service::Settings);
	if(settings == nullptr){
		return;
	}

	const auto brightness = settings->get().screenBrightness;
	const auto mapped = mapDuty(brightness);

	for(int i = 100; i >= 0; i--){
		uint8_t val = map((double) i, 0, 100, 0, mapped);
		pwm.setDuty(val);
		delayMillis(FadeDelay);
	}

	pwm.detach();
}

bool Backlight::isOn(){
	return state;
}
