#include "Encoders.h"
#include "Util/Events.h"
#include <Pins.hpp>
#include <driver/gpio.h>

const std::unordered_map<Encoders::Enc, Encoders::EncPins> Encoders::PinMap = {
		{ Cam, { .pinA = ENC_CAM_A, .pinB = ENC_CAM_B }},
		{ Arm, { .pinA = ENC_ARM_A, .pinB = ENC_ARM_B }},
		{ Pinch, { .pinA = ENC_PINCH_A, .pinB = ENC_PINCH_B }}
};

const std::unordered_map<Encoders::Enc, const char*> Encoders::Labels{
		{ Cam,   "Com" },
		{ Arm,   "Arm" },
		{ Pinch, "Pinch" }
};


Encoders::Encoders() : Threaded("Encoders", 2048, 8){
	auto mask = 0ULL;
	for(const auto& pair : PinMap){
		const auto pins = pair.second;
		mask |= (1ULL << pins.pinA) | (1ULL << pins.pinB);
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

Encoders::~Encoders(){
	stop();
}

void Encoders::loop(){
	scan();
	vTaskDelay(SleepTime);
}

void Encoders::scan(){
	for(int i = 0; i < Enc::COUNT; i++){
		scanOne((Enc) i);
	}
}

void Encoders::scanOne(Encoders::Enc enc){
	const auto& pins = PinMap.at(enc);
	int8_t val = 0;

	bool stateA = gpio_get_level((gpio_num_t) pins.pinA);
	if(states[enc].has_value() && stateA != states[enc]){
		bool stateB = gpio_get_level((gpio_num_t) pins.pinB);
		if(stateB != stateA){
			val = 1;
		}else{
			val = -1;
		}
	}
	states[enc] = stateA;

	if(val != 0){
		Events::post(Facility::Encoders, Data { .enc = enc, .dir = val });
	}
}
