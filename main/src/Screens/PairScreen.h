#ifndef PERSE_MISSIONCTRL_PAIRSCREEN_H
#define PERSE_MISSIONCTRL_PAIRSCREEN_H

#include "UISystem/Screen.h"
#include "UISystem/LabelElement.h"
#include "Util/Events.h"
#include "Services/PairService.h"
#include "Devices/Input.h"

class PairScreen : public Screen {
public:
	PairScreen(Sprite& canvas);
	virtual ~PairScreen();

private:

	LabelElement* text1;
	LabelElement* text2;

	EventQueue evts;

	std::unique_ptr<PairService> pair;

	void onLoop() override;

	void processInput(const Input::Data& evt);

};


#endif //PERSE_MISSIONCTRL_PAIRSCREEN_H
