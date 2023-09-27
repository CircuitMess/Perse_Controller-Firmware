#include "DriveScreen.h"
#include "Util/Services.h"
#include "Devices/Input.h"
#include "Devices/Encoders.h"
#include "Util/stdafx.h"
#include "LV_Interface/Theme/theme.h"
#include <glm.hpp>
#include <gtx/vector_angle.hpp>
#include "Pins.hpp"

DriveScreen::DriveScreen(Joystick* joystick, Display* display) : comm((Comm*) Services.get(Service::Comm)), queue(12), joystick(joystick), display(display),
											   calibThread([this](){ calibLoop(); }, "CalibThread", 4096), joySender([this](){ sendJoy(); }, "JoySender", 8000, 5, 0){
	printf("DriveScreen constructed\n");
	lv_obj_t* label = lv_label_create(obj);
	lv_label_set_text(label, "Test drive");
	lv_obj_set_style_text_color(label, lv_color_black(), 0);
	lv_obj_set_style_text_font(label, &devin, 0);
	lv_obj_align(label, LV_ALIGN_TOP_MID, 0,5);

	joystickLabel = lv_label_create(obj);
	lv_obj_set_style_text_color(joystickLabel, lv_color_black(), 0);
	lv_obj_set_style_text_font(joystickLabel, &devin, 0);
	lv_obj_center(joystickLabel);
}

DriveScreen::~DriveScreen(){
	printf("DriveScreen destructed\n");
	delete pair;
}

void DriveScreen::onStop(){
	calibThread.stop();
}

void DriveScreen::loop(){
	if(pair == nullptr) return;
	if(pair->getState() != PairService::State::Success) return;

	if(!paired){
		paired = true;
		gpio_config_t cfg = {
				.pin_bit_mask = 1ULL << LED_PAIR,
				.mode = GPIO_MODE_OUTPUT
		};
		gpio_config(&cfg);
		gpio_set_level((gpio_num_t) LED_PAIR, 1);
	}

	feed.nextFrame([this](const DriveInfo& info, const Color* img){
		auto lgfx = display->getLGFX();
		lgfx.pushImage(0, 4, 160, 120, img);
	});

	Event evt{};
	if(queue.get(evt, 0)){
		if(evt.facility == Facility::Encoders){
			auto data = (Encoders::Data*) evt.data;
			if(data->enc == Encoders::Cam){
				head = std::clamp(head + data->dir*5, 0, 255);
				comm->sendHead(head);
			}else if(data->enc == Encoders::Arm){
				arm = std::clamp(arm + data->dir*5, 0, 100);
				comm->sendArm(arm);
			}else if(data->enc == Encoders::Pinch){
				pinch = std::clamp(pinch + data->dir*5, 0, 100);
				comm->sendPinch(pinch);
			}
		}

		free(evt.data);
	}

	// lv_label_set_text_fmt(joystickLabel, "angle: %d   val: %d", (int)angle, (int)(val*100.0));
}

void DriveScreen::onStart(){
	Events::listen(Facility::Encoders, &queue);
	// calibThread.start();

	pair = new PairService();
	joySender.start();

}

void DriveScreen::calibLoop(){
	Event item{};
	if(queue.get(item, portMAX_DELAY)){
		auto data = (Input::Data*) item.data;
		if(data->btn == Input::Joy && data->action == Input::Data::Press){
			calibration = !calibration;
			printf(calibration ? "calibration start\n" : "calibration end\n");
			calibration ? joystick->startRangeCalib() : joystick->stopRangeCalib();
		}else if(data->btn == Input::Panic && data->action == Input::Data::Press){
			joystick->centerCalib();
			printf("center calibration\n");
		}
	}
	free(item.data);
}

void DriveScreen::sendJoy(){
	delayMillis(20);

	if(pair == nullptr) return;
	if(pair->getState() != PairService::State::Success) return;

	auto vec = joystick->getPos();
	float val = std::clamp(glm::length(vec), 0.f, 1.f);
	float angle = 0;
	if(val > 0){
		vec = glm::normalize(vec);
		angle = glm::degrees(glm::angle(vec, { 0.0, 1.0 }));
		if(vec.x < 0){
			angle = 360 - angle;
		}
	}
	comm->sendDriveDir({ static_cast<uint16_t>(angle), val });
	//printf("angle: %.2f, val: %.2f\n", angle, val);
}
