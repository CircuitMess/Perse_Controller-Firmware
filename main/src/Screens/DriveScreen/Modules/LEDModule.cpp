#include "LEDModule.h"
#include "Devices/Input.h"
#include "Util/Services.h"

LEDModule::LEDModule(ElementContainer* parent, ModuleBus bus, ModuleType type) : ModuleElement(parent, bus, type), statusLabel(this, ""){
	auto style = statusLabel.getStyle();
	style.datum = datum;
	style.color = textColor;
	style.shadingStyle = { TFT_BLACK, 1 };
	statusLabel.setStyle(style);
	statusLabel.setPos(0, 9);

	Input* input = (Input*) Services.get(Service::Input);
	if(input != nullptr && input->getState(Input::SwLight)){
		statusLabel.setText("ON");
	}else{
		statusLabel.setText("OFF");
	}
}

void LEDModule::dataReceived(ModuleData data){
	if(data.type != ModuleType::LED){
		return;
	}

	if(data.ledState.on){
		statusLabel.setText("ON");
	}else{
		statusLabel.setText("OFF");
	}
}

