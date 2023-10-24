#include "Screen.h"
#include "Modal.h"
#include "Element.h"

Screen::Screen(Sprite& canvas) : canvas(canvas){
}

Screen::~Screen(){
	delete modal;
}

void Screen::setBgColor(Color color){
	bgColor = color;
}

void Screen::loop(){
	onLoop();

	for(auto& element : elements){
		element->loop();
	}
}

void Screen::draw(){
	canvas.clear(bgColor);

	preDraw();

	for(auto& element : elements){
		element->draw(&canvas);
	}

	if(modal){
		modal->loop();
	}

	postDraw();
}
