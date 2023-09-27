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
	for(auto& element : elements){
		element->loop();
	}

	for(auto& element : elements){
		element->draw(screen->canvas);
	}
}
