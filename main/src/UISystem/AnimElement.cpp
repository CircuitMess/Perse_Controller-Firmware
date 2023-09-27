#include "AnimElement.h"
#include "esp_log.h"
#include "Util/stdafx.h"

static const char* TAG = "AnimElement";

AnimElement::AnimElement(ElementContainer* parent, const char* path) : Element(parent), gif(fopen(path, "r")){

	lastMicros = micros();
}

void AnimElement::setPath(const char* path){
	auto file = fopen(path, "r");
	if(ferror(file) != 0){
		ESP_LOGE(TAG, "Couldn't open file %s", path);
	}
}

void AnimElement::setLoopMode(GIF::LoopMode loopMode){
	gif.setLoopMode(loopMode);
}

void AnimElement::start(){
	gif.start();
}

void AnimElement::stop(){
	gif.stop();
}

void AnimElement::reset(){
	gif.reset();
}

void AnimElement::draw(Sprite* canvas){
	auto sprite = gif.getSprite();
	sprite.pushRotateZoom(canvas, std::round(x + (float) sprite.width() / 2.0), std::round(y + (float) sprite.height() / 2.0), 0, 1, 1, TFT_TRANSPARENT);
}

void AnimElement::loop(){
	auto current = micros();
	auto diff = current - lastMicros;
	lastMicros = current;

	gif.loop(diff);
}
