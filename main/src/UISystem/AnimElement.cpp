#include "AnimElement.h"
#include "esp_log.h"
#include "Util/stdafx.h"

static const char* TAG = "AnimElement";

AnimElement::AnimElement(ElementContainer* parent, const char* path, uint8_t layer) : Element(parent, layer), gif(std::make_unique<GIFSprite>(fopen(path, "r"))){

	lastMicros = micros();
}

void AnimElement::setPath(const char* path){
	auto file = fopen(path, "r");

	if(ferror(file) != 0){
		ESP_LOGE(TAG, "Couldn't open file %s", path);
		return;
	}

	gif.reset();
	gif = std::make_unique<GIFSprite>(file);
	lastMicros = micros();
}

void AnimElement::setLoopMode(GIF::LoopMode loopMode){
	if(gif == nullptr){
		return;
	}

	gif->setLoopMode(loopMode);
}

void AnimElement::start(){
	if(gif == nullptr){
		return;
	}

	gif->start();
	lastMicros = micros();
}

void AnimElement::stop(){
	if(gif == nullptr){
		return;
	}

	gif->stop();
}

void AnimElement::reset(){
	if(gif == nullptr){
		return;
	}

	gif->reset();
	lastMicros = micros();
}

void AnimElement::draw(Sprite* canvas){
	if(gif == nullptr || canvas == nullptr){
		return;
	}

	auto sprite = gif->getSprite();
	sprite.pushRotateZoom(canvas, std::round(x + (float) sprite.width() / 2.0), std::round(y + (float) sprite.height() / 2.0), 0, 1, 1, TFT_TRANSPARENT);
}

void AnimElement::loop(){
	auto current = micros();
	auto diff = current - lastMicros;
	lastMicros = current;

	if(gif == nullptr){
		return;
	}
	
	gif->loop(diff);
}
