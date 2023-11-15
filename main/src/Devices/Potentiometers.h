#ifndef PERSE_MISSIONCTRL_POTENTIOMETERS_H
#define PERSE_MISSIONCTRL_POTENTIOMETERS_H

#include <map>
#include <glm.hpp>
#include "Util/Threaded.h"
#include "Services/ADCReader.h"

class Potentiometers : SleepyThreaded {
public:
	enum Potentiometer {
		FeedQuality
	};

	struct Data {
		Potentiometer potentiometer;
		uint8_t value = 0; // Percent
	};

	explicit Potentiometers(ADC& adc);

	uint8_t getCurrentValue(Potentiometer potentiometer) const;

protected:
	virtual void sleepyLoop() override;

private:
	static constexpr uint64_t SleepTime = 15; // [ms]
	static const std::map<Potentiometer, gpio_num_t> PinMappings;
	static const std::map<Potentiometer, glm::vec<2, int>> LimitMappings;
	static constexpr float Step = 100.0f / 31.0f;
	std::map<Potentiometer, ADCReader> adcReaders;
	std::map<Potentiometer, float> adcValues;
};

#endif //PERSE_MISSIONCTRL_POTENTIOMETERS_H