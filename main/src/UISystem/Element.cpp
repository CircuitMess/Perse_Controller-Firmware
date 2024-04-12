#include "Element.h"
#include "ElementContainer.h"

Element::Element(ElementContainer* parent, uint8_t layer) : parent(parent), layer(layer){
	if(parent == nullptr){
		return;
	}

	auto it = parent->elements.begin();
	for(; it != parent->elements.end(); it++){
		if(*it != nullptr && (*it)->layer >= layer){
			break;
		}
	}

	parent->elements.insert(it, this);
}

Element::~Element(){
	if(parent == nullptr){
		return;
	}
	
	parent->elements.erase(std::remove(parent->elements.begin(), parent->elements.end(), this), parent->elements.end());
}

int16_t Element::getX() const{
	return x;
}

void Element::setX(int16_t x){
	Element::x = x;
}

int16_t Element::getY() const{
	return y;
}

void Element::setY(int16_t y){
	Element::y = y;
}

void Element::setPos(int16_t x, int16_t y){
	Element::x = x;
	Element::y = y;
}
