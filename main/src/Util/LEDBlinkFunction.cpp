#include "LEDBlinkFunction.h"
#include "Util/stdafx.h"
#include <stdio.h>

LEDBlinkFunction::LEDBlinkFunction(SingleLED& led, uint32_t count, uint32_t period) : LEDFunction(led), count(count), period(period){
	startTime = millis();
	printf("Start: %llu\n", startTime);
}

LEDBlinkFunction::~LEDBlinkFunction(){
	led.setValue(0);
}

void LEDBlinkFunction::loop(){
	const uint64_t elapsedTime = millis() - startTime;

	bool ledState = false;
	if(elapsedTime % period <= period / 2){
		ledState = true;
	}

	if((led.getValue() > 0 && ledState) || (led.getValue() == 0 && !ledState)){
		return;
	}

	if(count != 0 && elapsedCount >= count){
		if(led.getValue() != 0){
			led.setValue(0);
		}

		return;
	}

	if(ledState){
		++elapsedCount;
		led.setValue(0xFF);
	}else{
		led.setValue(0);
	}
}