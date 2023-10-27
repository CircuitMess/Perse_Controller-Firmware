#include "Modal.h"
#include "Screen.h"
#include "Element.h"

Modal::Modal(Screen* screen) : screen(screen){
	screen->modal = this;
}

Modal::~Modal(){
	screen->modal = nullptr;
}

void Modal::loop(){
	onElements([](Element* el){
		el->loop();
	});
}

void Modal::draw(){
	onElements([this](Element* el){
		el->draw(&screen->canvas);
	});
}
