#include "Battery.h"
#include <cmath>
#include <utility>
#include "Pins.hpp"
#include "Util/Events.h"
#include "Util/stdafx.h"
#include "Services/Comm.h"
#include "Util/Services.h"

#ifdef CTRL_TYPE_MISSIONCTRL
#define MAX_READ 4600	// 4.6V
#define MIN_READ 3600	// 3.6V
#define FACTOR2 (-0.000215947f)
#define FACTOR1 2.4594f
#define FACTOR0 (-1742.26f)

#elifdef CTRL_TYPE_BASIC
#define MAX_READ 4200    // 4.2V
#define MIN_READ 3600    // 3.6V
#define FACTOR2 (-0.000163675f)
#define FACTOR1  2.13009f
#define FACTOR0 (-1678.34f)
#endif

#ifdef CTRL_TYPE_MISSIONCTRL
#define BATTERY_TASK_COREID 1
#elifdef CTRL_TYPE_BASIC
#define BATTERY_TASK_COREID 0
#endif

Battery::Battery(ADC& adc) : SleepyThreaded(MeasureIntverval, "Battery", 3 * 1024, 5, BATTERY_TASK_COREID),
					 adc(adc, (gpio_num_t)PIN_BATT, 0.05, MIN_READ, MAX_READ, (float) getVoltOffset() + FACTOR0, FACTOR1, FACTOR2),
					 hysteresis({ 0, 4, 15, 30, 70, 100 }, 3) {

	adc_unit_t unit;
	adc_channel_t chan;
	adc_oneshot_io_to_channel(PIN_BATT, &unit, &chan);

	adc.config(chan, {
		.atten = ADC_ATTEN_DB_11,
		.bitwidth = ADC_BITWIDTH_12
	});

	sample(true);
}

void Battery::begin() {
	start();
}

uint8_t Battery::getPerc() const {
	return adc.getValue();
}

Battery::Level Battery::getLevel() const {
	return (Level)hysteresis.get();
}

int16_t Battery::getVoltOffset() {
	int16_t upper = 0, lower = 0;
	ESP_ERROR_CHECK(esp_efuse_read_field_blob((const esp_efuse_desc_t**) EfuseAdcLow, &lower, 8));
	ESP_ERROR_CHECK(esp_efuse_read_field_blob((const esp_efuse_desc_t**) EfuseAdcHigh, &upper, 8));

	return (upper << 8) | lower;
}

uint16_t Battery::mapRawReading(uint16_t reading) {
	const float fval = reading;
	return std::round(FACTOR0 + fval * FACTOR1 + std::pow(fval, 2.0f) * FACTOR2);
}

bool Battery::isShutdown() const {
	return shutdown;
}

void Battery::setShutdownCallback(std::function<void()> callback){
	shutdownCallback = std::move(callback);
	if(!shutdownCallback || !shutdown) return;

	shutdownCallback();
}

void Battery::sleepyLoop() {
	if (shutdown) {
		return;
	}

	sample();
}

void Battery::sample(bool fresh/* = false*/) {
	if (shutdown) {
		return;
	}

	const Level oldLevel = getLevel();

	if (fresh) {
		adc.resetEma();
		hysteresis.reset(adc.getValue());
	}
	else {
		float val = adc.sample();
		hysteresis.update(val);
	}

	if (oldLevel != getLevel() || fresh) {
		Events::post(Facility::Battery, Battery::Event{.action = Event::LevelChange, .level = getLevel()});
	}

	if(fresh){
		if(Comm* comm = (Comm*) Services.get(Service::Comm)){
			comm->sendControllerBatteryCritical(getLevel() == Critical);
		}
	}

	if (getLevel() == Critical) {
		if(Comm* comm = (Comm*) Services.get(Service::Comm)){
			comm->sendControllerBatteryCritical(true);
		}

#ifdef CTRL_TYPE_MISSIONCTRL
        delayMillis(BattPopupTime); //wait for BattPopup to show
#endif
		if(running()){
			setPriority(10);
			stop(0);
		}
		shutdown = true;
		if(!shutdownCallback) return;
		shutdownCallback();
		return;
	}
}
