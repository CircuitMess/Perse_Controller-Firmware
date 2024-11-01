#include "LEDService.h"
#include <esp_log.h>
#include "Devices/SingleExpanderLED.h"
#include "Devices/SinglePwmLED.h"
#include "Util/LEDBlinkFunction.h"
#include "Util/LEDBreatheFunction.h"
#include "Pins.hpp"

static const char* TAG = "LEDService";

const std::map<LED, LEDService::PwnMappingInfo> LEDService::PwmMappings = {
#ifdef CTRL_TYPE_MISSIONCTRL
		{ LED::Power, { (gpio_num_t) LED_POWER, LEDC_CHANNEL_1, 7 } },
		{ LED::Pair, { (gpio_num_t) LED_PAIR, LEDC_CHANNEL_2, 20 } },
		{ LED::PanicLeft,  { (gpio_num_t) LED_PANIC_L, LEDC_CHANNEL_3, 20 }},
		{ LED::PanicRight, { (gpio_num_t) LED_PANIC_R, LEDC_CHANNEL_4, 20 }},
#elifdef CTRL_TYPE_BASIC
		{ LED::Power, { (gpio_num_t) LED_POWER, LEDC_CHANNEL_1, 50 } },
		{ LED::Pair, { (gpio_num_t) LED_PAIR, LEDC_CHANNEL_2, 70 } },
		{ LED::Warning, { (gpio_num_t) LED_WARN, LEDC_CHANNEL_3, 70 } },
		{ LED::SoundLight, { (gpio_num_t) LED_SOUNDLIGHT, LEDC_CHANNEL_4, 70 } },
		{ LED::ArmPinch, { (gpio_num_t) LED_ARMPINCH, LEDC_CHANNEL_5, 70 } },
		{ LED::Navigation, { (gpio_num_t) LED_NAVIGATION, LEDC_CHANNEL_0, 70 } },

#endif
};

#ifdef CTRL_TYPE_MISSIONCTRL
const std::map<LED, LEDService::ExpanderMappingInfo> LEDService::ExpanderMappings = {
		//1 blok za svaki type
		{ LED::CamL4,      { EXTLED_CAM_L4,      0x20 } },
		{ LED::CamL3,      { EXTLED_CAM_L3,      0x20 } },
		{ LED::CamL2,      { EXTLED_CAM_L2,      0x20 } },
		{ LED::CamL1,      { EXTLED_CAM_L1,      0x20 } },
		{ LED::CamCenter,  { EXTLED_CAM_0,       0x20 } },
		{ LED::CamR1,      { EXTLED_CAM_R1,      0x20 } },
		{ LED::CamR2,      { EXTLED_CAM_R2,      0x20 } },
		{ LED::CamR3,      { EXTLED_CAM_R3,      0x20 } },
		{ LED::CamR4,      { EXTLED_CAM_R4,      0x20 } },
		{ LED::Warning,    { EXTLED_WARN,        0x20 } },
		{ LED::Arm,        { EXTLED_ARM,         0x06 } },
		{ LED::ArmUp,      { EXTLED_ARM_UP,      0x06 } },
		{ LED::ArmDown,    { EXTLED_ARM_DOWN,    0x06 } },
		{ LED::Light,      { EXTLED_LIGHT,       0x06 } },
		{ LED::PinchOpen,  { EXTLED_PINCH_OPEN,  0x06 } },
		{ LED::PinchClose, { EXTLED_PINCH_CLOSE, 0x06 } },
};
#endif


#ifdef CTRL_TYPE_MISSIONCTRL
LEDService::LEDService(AW9523& aw9523) : Threaded("LEDService"), instructionQueue(25){
	for(LED led = (LED) 0; (uint8_t) led < (uint8_t) LED::COUNT; led = (LED) ((uint8_t) led + 1)){
		const bool isExpander = ExpanderMappings.contains(led);
		const bool isPwm = PwmMappings.contains(led);

		if(isExpander && isPwm){
			ESP_LOGE(TAG, "LED %d is marked as both expander and PWM.", (uint8_t) led);
		}else if(isExpander){
			ExpanderMappingInfo ledData = ExpanderMappings.at(led);
			SingleLED* ledDevice = new SingleExpanderLED(aw9523, ledData.pin, ledData.limit);
			ledDevices[led] = ledDevice;
		}else{
			PwnMappingInfo ledData = PwmMappings.at(led);
			SingleLED* ledDevice = new SinglePwmLED(ledData.pin, ledData.channel, ledData.limit);
			ledDevices[led] = ledDevice;
		}
	}

	start();
}
#elifdef CTRL_TYPE_BASIC

LEDService::LEDService() : Threaded("LEDService"), instructionQueue(25){
	for(LED led = (LED) 0; (uint8_t) led < (uint8_t) LED::COUNT; led = (LED) ((uint8_t) led + 1)){
		if(!PwmMappings.contains(led)) continue;

		PwnMappingInfo ledData = PwmMappings.at(led);
		SingleLED* ledDevice = new SinglePwmLED(ledData.pin, ledData.channel, ledData.limit, led == LED::Power);
		ledDevices[led] = ledDevice;
	}

	start();
}

#endif

LEDService::~LEDService(){
	ledFunctions.clear();

	for(auto led : ledDevices){
		delete led.second;
	}
}

void LEDService::on(LED led){
	LEDInstructionInfo instruction{
			.led = led,
			.instruction = On
	};

	instructionQueue.post(instruction);
}

void LEDService::off(LED led){
	LEDInstructionInfo instruction{
			.led = led,
			.instruction = Off
	};

	instructionQueue.post(instruction);
}

void LEDService::blink(LED led, uint32_t count /*= 1*/, uint32_t period /*= 1000*/){
	LEDInstructionInfo instruction{
			.led = led,
			.instruction = Blink,
			.count = count,
			.period = period
	};

	instructionQueue.post(instruction);
}

void LEDService::breathe(LED led, uint32_t period /*= 1000*/){
	LEDInstructionInfo instruction{
			.led = led,
			.instruction = Breathe,
			.count = 0,
			.period = period
	};

	instructionQueue.post(instruction);
}

void LEDService::loop(){
	for(LEDInstructionInfo instructionInfo; instructionQueue.get(instructionInfo, 10);){
		if(instructionInfo.instruction == On){
			onInternal(instructionInfo.led);
		}else if(instructionInfo.instruction == Off){
			offInternal(instructionInfo.led);
		}else if(instructionInfo.instruction == Blink){
			blinkInternal(instructionInfo.led, instructionInfo.count, instructionInfo.period);
		}else if(instructionInfo.instruction == Breathe){
			breatheInternal(instructionInfo.led, instructionInfo.period);
		}
	}

	for(LED led = (LED) 0; (uint8_t) led < (uint8_t) LED::COUNT; led = (LED) ((uint8_t) led + 1)){
		if(!ledFunctions.contains(led)){
			continue;
		}

		ledFunctions[led]->loop();
	}
}

void LEDService::onInternal(LED led){
	if(ledFunctions.contains(led)){
		ledFunctions.erase(led);
	}

	if(!ledDevices.contains(led)){
		return;
	}

	if(ledDevices[led] == nullptr){
		return;
	}

	ledDevices[led]->setValue(0xFF);
}

void LEDService::offInternal(LED led){
	if(ledFunctions.contains(led)){
		ledFunctions.erase(led);
	}

	if(!ledDevices.contains(led)){
		return;
	}

	if(ledDevices[led] == nullptr){
		return;
	}

	ledDevices[led]->setValue(0);
}

void LEDService::blinkInternal(LED led, uint32_t count, uint32_t period){
	if(ledFunctions.contains(led)){
		ledFunctions.erase(led);
	}

	if(!ledDevices.contains(led)){
		ESP_LOGW(TAG, "LED %d is set to blink, but does not exist.", (uint8_t) led);
		return;
	}

	ledFunctions[led] = std::make_unique<LEDBlinkFunction>(*ledDevices[led], count, period);
}

void LEDService::breatheInternal(LED led, uint32_t period){
	if(ledFunctions.contains(led)){
		ledFunctions.erase(led);
	}

	if(!ledDevices.contains(led)){
		ESP_LOGW(TAG, "LED %d is set to breathe, but does not exist.", (uint8_t) led);
		return;
	}

	ledFunctions[led] = std::make_unique<LEDBreatheFunction>(*ledDevices[led], period);
}
