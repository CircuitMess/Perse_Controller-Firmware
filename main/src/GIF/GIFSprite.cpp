#include "GIFSprite.h"
#include <esp_log.h>

static const char* TAG = "GIFSprite";

GIFSprite::GIFSprite(FILE* file) : gif(file){
	if(!gif){
		ESP_LOGE(TAG, "File not open");
		return;
	}
}

GIFSprite::~GIFSprite(){
	stop();
}

void GIFSprite::loop(uint micros){
	if(!gif) return;
	if(!running) return;

	frameCounter += micros;

	while(frameCounter / 1000 >= gif.frameDuration()){
		frameCounter -= gif.frameDuration() * 1000;
		bool done = false;
		if(!gif.nextFrame()){
			stop();
			done = true;
		}

		if(gif.getLoopCount() > loopCount){
			loopCount = gif.getLoopCount();

			if(loopDoneCallback){
				loopDoneCallback(loopCount);
			}
			return;
		}

		if(done) return;
	}
}

void GIFSprite::start(){
	if(!gif) return;
	if(running) return;
	running = true;
	gif.nextFrame();
}

void GIFSprite::stop(){
	if(!running) return;
	running = false;
}

void GIFSprite::reset(){
	if(!gif) return;

	frameCounter = 0;
	loopCount = 0;

	gif.reset();
	gif.nextFrame();
}

uint16_t GIFSprite::getWidth() const{
	if(!gif) return 0;
	return gif.getWidth();
}

uint16_t GIFSprite::getHeight() const{
	if(!gif) return 0;
	return gif.getHeight();
}

GIF::LoopMode GIFSprite::getLoopMode() const{
	if(!gif) return GIF::LoopMode::Auto;
	return gif.getLoopMode();
}

void GIFSprite::setLoopMode(GIF::LoopMode loopMode){
	if(!gif) return;
	gif.setLoopMode(loopMode);
}

uint32_t GIFSprite::getLoopCount() const{
	if(!gif) return 0;
	return gif.getLoopCount();
}

void GIFSprite::setLoopDoneCallback(std::function<void(uint32_t loopCount)> callback){
	loopDoneCallback = callback;
}

void GIFSprite::setScale(uint8_t scale){
	GIFSprite::scale = scale;
}

Sprite GIFSprite::getSprite() const{
	auto frame = gif.getFrame();
	Sprite sprite;
	sprite.setBuffer((void*) frame.getData(), frame.getWidth(), frame.getHeight(), 24);
	return sprite;
}
