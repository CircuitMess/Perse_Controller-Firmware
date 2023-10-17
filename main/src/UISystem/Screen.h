#ifndef PERSE_MISSIONCTRL_SCREEN_H
#define PERSE_MISSIONCTRL_SCREEN_H

#include "ElementContainer.h"
#include "Devices/Display.h"

class Modal;

class Screen : public ElementContainer{
	friend Modal;
public:
	Screen(Sprite& canvas);
	~Screen() override;

	void loop();

protected:
	virtual void onLoop(){}
	virtual void preDraw(){}
	virtual void postDraw(){}

	Sprite& canvas;

private:
	Modal* modal = nullptr;
};


#endif //PERSE_MISSIONCTRL_SCREEN_H
