#include "GyroModule.h"

GyroModule::GyroModule(ElementContainer* parent, ModuleBus bus, ModuleType type) : ModuleElement(parent, bus, type), xLabel(this, ""), yLabel(this, ""){

	auto style = xLabel.getStyle();
	style.datum = datum;
	style.color = textColor;
	xLabel.setStyle(style);
	xLabel.setText("X " +  std::string(gyroValLength, ' '));
	xLabel.setPos(0, 9);

	yLabel.setStyle(style);
	yLabel.setText("Y " +  std::string(gyroValLength, ' '));
	yLabel.setPos(0, 18);
}

void GyroModule::dataReceived(ModuleData data){
	xLabel.setText("X " + paddedValueLeft(data.gyro.x, gyroValLength));
	yLabel.setText("Y " + paddedValueLeft(data.gyro.y, gyroValLength));
}
