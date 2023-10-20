#include "Screen.h"
#include "Modal.h"
#include "Element.h"

Screen::Screen(Sprite& canvas) : canvas(canvas){
}

Screen::~Screen(){
	delete modal;
}

void Screen::loop(){
	onLoop();
	for(auto& element : elements){
		element->loop();
	}

	preDraw();
	for(auto& element : elements){
		element->draw(&canvas);
	}
	if(modal){
		modal->loop();
	}
	postDraw();
}

int32_t Screen::getWidth() const {
	return canvas.width();
}

int32_t Screen::getHeight() const {
	return canvas.height();
}