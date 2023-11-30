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

	if(transitioned) return;

	onElements([](Element* el){
		if(el == nullptr){
			return;
		}

		el->loop();
	});

	if(modal){
		modal->draw();
	}
}

void Screen::draw(){
	canvas.clear(bgColor);

	preDraw();

	onElements([this](Element* el){
		if(el == nullptr){
			return;
		}

		el->draw(&canvas);
	});

	postDraw();

	if(modal){
		modal->draw();
	}
}

void Screen::transition(const ScreenCreateFunc& create){
	auto ui = (UIThread*) Services.get(Service::UI);
	if(ui == nullptr) return;

	ui->startScreen(create);
	transitioned = true;
}

int32_t Screen::getWidth() const {
	return canvas.width();
}

int32_t Screen::getHeight() const {
	return canvas.height();
}