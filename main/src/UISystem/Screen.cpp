#include "Screen.h"
#include "Modal.h"
#include "Element.h"
#include "Util/Services.h"

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

void Screen::transition(ScreenCreateFunc create){
	auto ui = (UIThread*) Services.get(Service::UI);
	if(ui == nullptr) return;

	ui->startScreen(create);
}

int32_t Screen::getWidth() const {
	return canvas.width();
}

int32_t Screen::getHeight() const {
	return canvas.height();
}