#ifndef PERSE_ROVER_LEDSERVICE_H
#define PERSE_ROVER_LEDSERVICE_H

#include <cstdint>
#include <map>
#include <tuple>
#include <driver/ledc.h>
#include <memory>
#include <mutex>
#include "Util/Threaded.h"
#include "Util/Queue.h"

#ifdef CTRL_TYPE_MISSIONCTRL
#include "Devices/AW9523.h"
#endif


enum class LED : uint8_t {
	Power,
	Pair,
	Warning,
#ifdef CTRL_TYPE_MISSIONCTRL
	PanicLeft,
	PanicRight,
	CamL4,
	CamL3,
	CamL2,
	CamL1,
	CamCenter,
	CamR1,
	CamR2,
	CamR3,
	CamR4,
	Arm,
	ArmUp,
	ArmDown,
	Light,
	PinchOpen,
	PinchClose,
#elifdef CTRL_TYPE_BASIC
	SoundLight,
	ArmPinch,
	Navigation,
#endif
	COUNT
};

class LEDService : private Threaded {
public:
#ifdef CTRL_TYPE_MISSIONCTRL
	explicit LEDService(AW9523& aw9523);
#elifdef CTRL_TYPE_BASIC
	LEDService();
#endif

	virtual ~LEDService();

	void on(LED led);

	void off(LED led);

	void blink(LED led, uint32_t count = 1, uint32_t period = 1000);

	void breathe(LED led, uint32_t period = 1000);

protected:
	virtual void loop() override;

private:
#ifdef CTRL_TYPE_MISSIONCTRL
	struct ExpanderMappingInfo {
		uint8_t pin = 0;
		uint8_t limit = 0xFF;
	};
	static const std::map<LED, ExpanderMappingInfo> ExpanderMappings;
#endif

	struct PwnMappingInfo {
		gpio_num_t pin = GPIO_NUM_NC;
		ledc_channel_t channel = LEDC_CHANNEL_0;
		uint8_t limit = 100;
	};

	static const std::map<LED, PwnMappingInfo> PwmMappings;

private:
	enum LEDInstruction {
		On,
		Off,
		Blink,
		Breathe
	};

	struct LEDInstructionInfo {
		LED led;
		LEDInstruction instruction;
		uint32_t count;
		uint32_t period;
	};

	std::map<LED, class SingleLED*> ledDevices;
	std::map<LED, std::unique_ptr<class LEDFunction>> ledFunctions;
	Queue<LEDInstructionInfo> instructionQueue;

private:
	void onInternal(LED led);

	void offInternal(LED led);

	void blinkInternal(LED led, uint32_t count, uint32_t period);

	void breatheInternal(LED led, uint32_t period);
};

#endif //PERSE_ROVER_LEDSERVICE_H