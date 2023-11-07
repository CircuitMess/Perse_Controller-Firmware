#include "GyroModule.h"

GyroModule::GyroModule(ElementContainer* parent, ModuleBus bus, ModuleType type) : ModuleElement(parent, bus, type), dataLabel(this, ""){

	auto style = dataLabel.getStyle();
	style.datum = datum;
	style.color = TFT_GREEN;
	dataLabel.setStyle(style);
	dataLabel.setText("X:\nY:");
	dataLabel.setPos(0, 8);
}

void GyroModule::dataReceived(ModuleData data){
	dataLabel.setText("X: " + std::to_string(data.gyro.x) + "\nY: " + std::to_string(data.gyro.y));
}
