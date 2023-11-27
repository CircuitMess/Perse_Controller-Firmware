#include "LEDModule.h"

LEDModule::LEDModule(ElementContainer* parent, ModuleBus bus, ModuleType type) : ModuleElement(parent, bus, type), statusLabel(this, ""){
	auto style = statusLabel.getStyle();
	style.datum = datum;
	style.color = textColor;
	statusLabel.setStyle(style);
	statusLabel.setText("OFF");
	statusLabel.setPos(0, 9);
	//TODO - settanje ovoga on/off
}

void LEDModule::dataReceived(ModuleData data){

}

