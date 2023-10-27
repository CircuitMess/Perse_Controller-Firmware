#include "LED.h"
#include "Util/stdafx.h"

LED::LED(AW9523& aw9523) : Threaded("LED", 4096), expander(aw9523), actionQueue(12){
	start();

	for(int i = 0; i < 16; i++){
		expander.pinMode(i, AW9523::LED);
		expander.dim(i, 0);

		actions[i] = Off;
		state[i] = false;
		times[i] = 0;
	}
}

void LED::on(uint8_t i){
	actionQueue.post({ i, On });
}

void LED::off(uint8_t i){
	actionQueue.post({ i, Off });
}

void LED::blink(uint8_t i){
	actionQueue.post({ i, Blink });
}

void LED::blinkCont(uint8_t i){
	actionQueue.post({ i, BlinkCont });
}

void LED::loop(){
	const auto now = millis();

	Action act{};
	while(actionQueue.get(act, 0)){
		const auto i = act.pin;

		if(act.action == Off){
			actions[i] = Off;
			expander.dim(i, 0);
		}else if(act.action == On){
			actions[i] = On;
			expander.dim(i, 5);
		}else if(act.action == Blink){
			actions[i] = Blink;
			times[i] = now;
			expander.dim(i, 5);
		}else if(act.action == BlinkCont){
			actions[i] = BlinkCont;
			times[i] = now;
			state[i] = true;
			expander.dim(i, 5);
		}
	}

	for(int i = 0; i < 16; i++){
		if(actions[i] == Blink && now - times[i] >= 50){
			actions[i] = Off;
			expander.dim(i, 0);
		}else if(actions[i] == BlinkCont && now - times[i] >= 500){
			state[i] = !state[i];
			times[i] = now;
			expander.dim(i, state[i] ? 5 : 0);
		}
	}

	delayMillis(10);
}
