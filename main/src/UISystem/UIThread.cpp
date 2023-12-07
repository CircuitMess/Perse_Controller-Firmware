#include "UIThread.h"
#include "Screen.h"
#include "Util/stdafx.h"

UIThread::UIThread(Display& display) : Threaded("UIThread", 6 * 1024, 6, 1), display(display){

}

UIThread::~UIThread(){
	stop();
}

void UIThread::startScreen(const ScreenCreateFunc& create){
	creator = create;
}

void UIThread::loop(){
	if(!currentScreen){
		if(!creator){
			delayMillis(FrameTime);
			return;
		}

		currentScreen.reset();
		currentScreen = creator(display.getCanvas());
		creator = {};
	}

	const auto currMicros = esp_timer_get_time();

	currentScreen->loop();
	if(creator){
		currentScreen.reset();
		currentScreen = creator(display.getCanvas());
		creator = {};
		return;
	}

	currentScreen->draw();

	display.commit();

	const auto loopTime = (micros() - currMicros) / 1000;
	if(loopTime > FrameTime){
		delayMillis(1);
	}else{
		delayMillis(FrameTime - loopTime);
	}
}
