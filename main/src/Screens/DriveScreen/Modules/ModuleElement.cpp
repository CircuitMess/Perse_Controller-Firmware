#include "ModuleElement.h"
#include "Services/Comm.h"

const std::unordered_map<ModuleType, const char*> moduleNames = {
		{ ModuleType::Gyro,     "GYRO" },
		{ ModuleType::TempHum,  "TMP" },
		{ ModuleType::AltPress, "ALT" },
		{ ModuleType::CO2,      "CO2" },
		{ ModuleType::Motion,   "SCAN" },
		{ ModuleType::PhotoRes, "LGHT" },
		{ ModuleType::LED,      "LED" },
		{ ModuleType::RGB,      "RGB" },
		{ ModuleType::Unknown,  "ERR" }
};

ModuleElement::ModuleElement(ElementContainer* parent, ModuleBus bus, ModuleType type) : Element(parent, 1), bus(bus), type(type),
																						 datum((bus == ModuleBus::Left) ? middle_left : middle_right),
																						 nameLabel(this, ""), queue(4){
	Events::listen(Facility::Comm, &queue);
	auto style = nameLabel.getStyle();
	style.datum = datum;
	style.color = TFT_GREEN;
	style.shadingStyle = { TFT_BLACK, 1 };
	nameLabel.setStyle(style);
	nameLabel.setText(moduleNames.at(type));
}

ModuleElement::~ModuleElement(){
	Events::unlisten(&queue);
}

void ModuleElement::draw(Sprite* canvas){
	onElements([canvas, this](Element* e){
		auto relX = getX() + e->getX();
		auto relY = getY() + e->getY();
		auto prevX = e->getX();
		auto prevY = e->getY();
		e->setPos(relX, relY);
		e->draw(canvas);
		e->setPos(prevX, prevY);
	});
}

void ModuleElement::loop(){
	Event event{};
	if(!queue.get(event, 0)) return;

	auto* data = ((Comm::Event*) event.data);


	if(data->type == CommType::ModuleData && data->moduleData.bus == bus && data->moduleData.type == type){
		dataReceived(data->moduleData);
	}

	free(data);
}

ModuleType ModuleElement::getType() const{
	return type;
}

