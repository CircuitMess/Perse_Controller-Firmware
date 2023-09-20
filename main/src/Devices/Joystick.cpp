#include "Joystick.h"
#include "Util/Services.h"
#include "Util/stdafx.h"

Joystick::Joystick(gpio_num_t joyX, gpio_num_t joyY) : Threaded("Joystick", 4 * 1024), settings(*(Settings*) Services.get(Service::Settings)),
													   adcX(joyX), adcY(joyY),
													   calibThread([this](){ calibLoop(); }, "JoyCalib", 2 * 1024){
	calibration = settings.get().joyCalib;
	enableFilters();
	start();
}

Joystick::~Joystick(){
	stop();
}

float Joystick::getX() const{
	auto val = adcX.getVal();
	if(abs(val - calibration.centerX) <= DeadZoneVal) return 0;

	val = std::clamp((uint32_t) val, calibration.minX, calibration.maxX);

	float mapped;
	if(val >= calibration.centerX){
		mapped = map(val, calibration.centerX, calibration.maxX, 0, 1);
	}else{
		mapped = map(val, calibration.minX, calibration.centerX, 0, 1) - 1.0f;
	}

	return mapped;
}

float Joystick::getY() const{
	auto val = adcY.getVal();
	if(abs(val - calibration.centerY) <= DeadZoneVal) return 0;

	val = std::clamp((uint32_t) val, calibration.minY, calibration.maxY);

	float mapped;
	if(val >= calibration.centerY){
		mapped = map(val, calibration.centerY, calibration.maxY, 0, 1);
	}else{
		mapped = map(val, calibration.minY, calibration.centerY, 0, 1) - 1.0f;
	}

	return mapped;
}

glm::vec2 Joystick::getPos() const{
	return glm::vec2(getX(), getY());
}

void Joystick::loop(){
	adcX.sample();
	adcY.sample();
	delayMillis(10);
}

void Joystick::startRangeCalib(){
	if(inCalibration) return;

	calibration.maxX = calibration.minX = 4095 / 2;
	calibration.maxY = calibration.minY = 4095 / 2;

	stop();
	disableFilters();
	inCalibration = true;
	calibThread.start();
}

void Joystick::stopRangeCalib(){
	if(!inCalibration) return;

	calibThread.stop();

	auto setts = settings.get();
	setts.joyCalib = calibration;
	settings.set(setts);
	settings.store();

	inCalibration = false;
	enableFilters();
	start();
}

void Joystick::centerCalib(){
	if(inCalibration) return;

	stop();
	disableFilters();

	calibration.centerX = adcX.sample();
	calibration.centerY = adcY.sample();

	auto setts = settings.get();
	setts.joyCalib = calibration;
	settings.set(setts);
	settings.store();

	enableFilters();
	start();
}

void Joystick::enableFilters(){
	adcX.setEmaA(FilterStrength);
	adcY.setEmaA(FilterStrength);
}

void Joystick::disableFilters(){
	adcX.setEmaA(1);
	adcY.setEmaA(1);
}

void Joystick::calibLoop(){
	uint32_t valX = adcX.sample();
	uint32_t valY = adcY.sample();

	if(valX > calibration.maxX){
		calibration.maxX = valX;
	}
	if(valX < calibration.minX){
		calibration.minX = valX;
	}
	if(valY > calibration.maxY){
		calibration.maxY = valY;
	}
	if(valY < calibration.minY){
		calibration.minY = valY;
	}

	delayMillis(10);
}
