#include "Potentiometers.h"
#include "Util/Events.h"
#include "Pins.hpp"

const std::map<Potentiometers::Potentiometer, gpio_num_t> Potentiometers::PinMappings = {
		{ FeedQuality, (gpio_num_t) PIN_QUAL }
};

const std::map<Potentiometers::Potentiometer, glm::vec<2, int>> Potentiometers::LimitMappings = {
		{ FeedQuality, { 0, 4096 }}
};

Potentiometers::Potentiometers(ADC& adc) : SleepyThreaded(SleepTime, "Potentiometers", 2 * 1024){
	for(const std::pair<Potentiometer, gpio_num_t> mapping: PinMappings){
		if(!LimitMappings.contains(mapping.first)){
			continue;
		}

		ADCReader adcReader(adc, mapping.second, 0.05, LimitMappings.at(mapping.first).x, LimitMappings.at(mapping.first).y, 0);

		adc_unit_t unit;
		adc_channel_t chan;
		ESP_ERROR_CHECK(adc_oneshot_io_to_channel(mapping.second, &unit, &chan));

		adc.config(chan, {
				.atten = ADC_ATTEN_DB_11,
				.bitwidth = ADC_BITWIDTH_12
		});

		adcReaders.insert({ mapping.first, adcReader });
	}

	start();
}

uint8_t Potentiometers::getCurrentValue(Potentiometers::Potentiometer potentiometer) const{
	if (!adcValues.contains(potentiometer)){
		return 0;
	}

	return (uint8_t) adcValues.at(potentiometer);
}

void Potentiometers::sleepyLoop(){
	for(std::pair<Potentiometer, ADCReader> adcPair: adcReaders){
		adcPair.second.sample();
		const float percentValue = adcPair.second.getValue();

		if(!adcValues.contains(adcPair.first) || std::abs(adcValues[adcPair.first] - percentValue) >= Step){
			adcValues[adcPair.first] = percentValue;

			Data data{
					.potentiometer = adcPair.first,
					.value = (uint8_t) percentValue
			};

			Events::post(Facility::Potentiometers, data);
		}
	}
}