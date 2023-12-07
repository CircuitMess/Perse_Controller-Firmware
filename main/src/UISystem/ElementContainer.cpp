#include "ElementContainer.h"
#include "Element.h"

ElementContainer::~ElementContainer(){
	std::vector<Element*> copy = elements;
	for(Element* element : copy){
		delete element;
	}
}

void ElementContainer::onElements(const std::function<void(Element*)>& func){
	for(auto el : elements){
		if(el == nullptr){
			continue;
		}

		func(el);
	}
}
