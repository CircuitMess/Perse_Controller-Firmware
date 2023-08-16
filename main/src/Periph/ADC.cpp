#include "ADC.h"
#include <driver/adc.h>
#include <esp_log.h>
#include <algorithm>

static const char* TAG = "ADC";

ADC::ADC(gpio_num_t pin, float ema_a, int min, int max, int readingOffset) : pin(pin), ema_a(ema_a), min(min), max(max), offset(readingOffset){
	if(pin != GPIO_NUM_1 && pin != GPIO_NUM_2 && pin != GPIO_NUM_7 && pin != GPIO_NUM_8){
		ESP_LOGE(TAG, "Only GPIOs 1, 2, 7, 8 are supported for ADC");
		valid = false;
		return;
	}

	adc1_config_width(ADC_WIDTH_BIT_12);
	if(pin == GPIO_NUM_1){
		chan = ADC1_CHANNEL_0;
	}else if(pin == GPIO_NUM_2){
		chan = ADC1_CHANNEL_1;
	}else if(pin == GPIO_NUM_7){
		chan = ADC1_CHANNEL_6;
	}else if(pin == GPIO_NUM_8){
		chan = ADC1_CHANNEL_7;
	}
	adc1_config_channel_atten(chan, ADC_ATTEN_DB_11);

	sample();
}

void ADC::setEmaA(float emaA){
	ema_a = emaA;
}

void ADC::resetEma(){
	val = -1;
	sample();
}

float ADC::sample(){
	if(!valid){
		return 0;
	}

	float reading = adc1_get_raw(chan);

	if(val == -1 || ema_a == 1){
		val = reading;
	}else{
		val = val * (1.0f - ema_a) + ema_a * reading;
	}

	return getVal();
}

float ADC::getVal() const{
	if(!valid){
		return 0;
	}

	if(max == 0 && min == 0){
		return val + offset;
	}

	float min = this->min;
	float max = this->max;
	bool inverted = min > max;
	if(inverted){
		std::swap(min, max);
	}

	float val = std::clamp(this->val + offset, min, max);
	val = (val - min) / (max - min);
	val = std::clamp(val*100.0f, 0.0f, 100.0f);

	if(inverted){
		val = 100.0f - val;
	}

	return val;
}

