#include "TempHumModule.h"

TempHumModule::TempHumModule(ElementContainer* parent, ModuleBus bus, ModuleType type) : ModuleElement(parent, bus, type), tempValueLabel(this, ""),
																						 humLabel(this, ""), humValueLabel(this, ""){
	auto style = tempValueLabel.getStyle();
	style.datum = datum;
	style.color = textColor;
	tempValueLabel.setStyle(style);
	tempValueLabel.setText(std::string(valueLength, ' ') + 'C');
	tempValueLabel.setPos(0, 9);

	humLabel.setStyle(style);
	humLabel.setText("HUM");
	humLabel.setPos(0, 18);

	humValueLabel.setStyle(style);
	humValueLabel.setText(std::string(valueLength, ' ') + '%');
	humValueLabel.setPos(0, 27);
}

void TempHumModule::dataReceived(ModuleData data){
	tempValueLabel.setText(paddedValueRight(data.tempHum.temp, valueLength) + 'C');
	humValueLabel.setText(paddedValueRight(data.tempHum.humidity, valueLength) + '%');
}

