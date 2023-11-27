#include "CO2Module.h"

CO2Module::CO2Module(ElementContainer* parent, ModuleBus bus, ModuleType type) : ModuleElement(parent, bus, type), statusLabel(this, ""){
	auto style = statusLabel.getStyle();
	style.datum = datum;
	statusLabel.setStyle(style);
	statusLabel.setPos(0, 9);
}

void CO2Module::loop(){
	ModuleElement::loop();
	if(status) return;

	if(millis() - lastMillis >= FlashingInterval){
		flashStatus = !flashStatus;
		lastMillis = millis();
		if(flashStatus){
			statusLabel.setText("");
		}else{
			statusLabel.setText("N-OK");
		}
	}
}

void CO2Module::dataReceived(ModuleData data){
	if(data.gas.isOk == status) return;

	status = data.gas.isOk;

	if(status){
		statusLabel.setText("OK");
		statusLabel.setColor(OKColor);
	}else{
		statusLabel.setText("N-OK");
		statusLabel.setColor(NOKColor);
		lastMillis = millis();
		flashStatus = true;
	}
}

