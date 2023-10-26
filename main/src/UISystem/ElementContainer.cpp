#include "ElementContainer.h"
#include "Element.h"

ElementContainer::~ElementContainer(){
	std::vector<Element*> copy = elements;
	for(Element* element : copy){
		delete element;
	}
}

void ElementContainer::onElements(std::function<void(Element*)> func){
	for(auto el : elements){
		func(el);
	}
}
