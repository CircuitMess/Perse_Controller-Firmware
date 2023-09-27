#include "UIThread.h"
#include "Util/stdafx.h"

UIThread::UIThread(Display& display) : Threaded("UIThread", 4 * 1024, 6, 1), display(display){

}

UIThread::~UIThread(){
	stop();
}

void UIThread::startScreen(std::function<std::unique_ptr<Screen>()> create){
	currentScreen.reset();
	currentScreen = create();
}

void UIThread::loop(){
	if(!currentScreen){
		delayMillis(FrameTime);
		return;
	}

	const auto currMicros = esp_timer_get_time();

	currentScreen->loop();
	if(!currentScreen) return; // in case the screen exited in its loop

	display.getCanvas().pushSprite(0, 0);
	display.getLGFX().display();

	const auto loopTime = (micros() - currMicros) / 1000;
	if(loopTime > FrameTime){
		delayMillis(1);
	}else{
		delayMillis(FrameTime - loopTime);
	}
}
