#ifndef PERSE_MISSIONCTRL_ELEMENTCONTAINER_H
#define PERSE_MISSIONCTRL_ELEMENTCONTAINER_H

#include <vector>
#include <functional>

class Element;

class ElementContainer {
	friend Element;
public:
	virtual ~ElementContainer();

protected:
	void onElements(const std::function<void(Element*)>& func);

private:
	std::vector<Element*> elements;
};


#endif //PERSE_MISSIONCTRL_ELEMENTCONTAINER_H
