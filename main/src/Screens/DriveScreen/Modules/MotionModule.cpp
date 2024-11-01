#include "MotionModule.h"

MotionModule::MotionModule(ElementContainer* parent, const ModuleBus& bus, const ModuleType& type) : ModuleElement(parent, bus, type), statusLabel(this, ""){
	auto style = statusLabel.getStyle();
	style.datum = datum;
	style.color = textColor;
	style.shadingStyle = { TFT_BLACK, 1 };
	statusLabel.setStyle(style);
	statusLabel.setPos(0, 9);
}

void MotionModule::dataReceived(ModuleData data){
	if(data.motion.motionDetected == status) return;

	status = data.motion.motionDetected;
	lastMillis = millis();

	if(status){
		statusLabel.setText("MOTION");
		flashStatus = true;
	}else{
		statusLabel.setText("");
		dotCounter = 0;
	}
}

void MotionModule::loop(){
	ModuleElement::loop();

	if(millis() - lastMillis >= FlashingInterval){
		lastMillis = millis();
		if(status){
			dotCounter = (dotCounter + 1) % DotLimit;
			statusLabel.setText(std::string(dotCounter, '.') + std::string(DotLimit - dotCounter, ' '));
		}else{
			flashStatus = !flashStatus;
			if(flashStatus){
				statusLabel.setText("");
			}else{
				statusLabel.setText("MOTION");
			}
		}
	}

}
