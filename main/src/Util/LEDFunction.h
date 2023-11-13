#ifndef PERSE_MISSIONCTRL_LEDFUNCTION_H
#define PERSE_MISSIONCTRL_LEDFUNCTION_H

#include "Devices/SingleLED.h"

class LEDFunction {
public:
	explicit LEDFunction(SingleLED& led);

	virtual ~LEDFunction() = default;

	virtual void loop() = 0;

protected:
	SingleLED& led;
};

#endif //PERSE_MISSIONCTRL_LEDFUNCTION_H