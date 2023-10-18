#ifndef PERSE_MISSIONCTRL_ELEMENTCONTAINER_H
#define PERSE_MISSIONCTRL_ELEMENTCONTAINER_H

#include <vector>

class Element;

class ElementContainer {
	friend Element;
public:
	virtual ~ElementContainer();

protected:
	std::vector<Element*> elements;
};


#endif //PERSE_MISSIONCTRL_ELEMENTCONTAINER_H
