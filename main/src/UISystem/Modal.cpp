#include "Modal.h"
#include "Screen.h"
#include "Element.h"

Modal::Modal(Screen* screen) : screen(screen){
	if(screen == nullptr){
		return;
	}

	screen->modal = this;
}

Modal::~Modal(){
	if(screen == nullptr){
		return;
	}

	screen->modal = nullptr;
}

void Modal::loop(){
	onElements([](Element* el){
		if(el == nullptr){
			return;
		}

		el->loop();
	});
}

void Modal::draw(){
	onElements([this](Element* el){
		if(el == nullptr){
			return;
		}

		el->draw(&screen->canvas);
	});
}
