#ifndef PERSE_MISSIONCTRL_MODAL_H
#define PERSE_MISSIONCTRL_MODAL_H

#include "ElementContainer.h"

class Screen;

class Modal : public ElementContainer{
public:
	Modal(Screen* screen);
	~Modal() override;

	void loop();
private:
	Screen* screen = nullptr;
};


#endif //PERSE_MISSIONCTRL_MODAL_H
