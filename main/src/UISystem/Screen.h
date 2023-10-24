#ifndef PERSE_MISSIONCTRL_SCREEN_H
#define PERSE_MISSIONCTRL_SCREEN_H

#include "ElementContainer.h"
#include "Devices/Display.h"
#include "Color.h"

class Modal;

class Screen : public ElementContainer{
	friend Modal;
public:
	Screen(Sprite& canvas);
	~Screen() override;

	void loop();
	void draw();

protected:
	virtual void onLoop(){}
	virtual void preDraw(){}
	virtual void postDraw(){}

	Sprite& canvas;

	void setBgColor(Color color);

private:
	Modal* modal = nullptr;
	Color bgColor = TFT_BLACK;

};


#endif //PERSE_MISSIONCTRL_SCREEN_H