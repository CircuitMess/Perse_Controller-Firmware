#ifndef PERSE_MISSIONCTRL_PAIRSCREEN_H
#define PERSE_MISSIONCTRL_PAIRSCREEN_H

#include "UISystem/Screen.h"
#include "UISystem/LabelElement.h"
#include "UISystem/ImageElement.h"
#include "UISystem/AnimElement.h"
#include "Util/Events.h"
#include "Services/PairService.h"
#include "Devices/Input.h"
#include "glm.hpp"

class PairScreen : public Screen {
public:
	PairScreen(Sprite& canvas, bool disconnectOccurred = false);
	virtual ~PairScreen();

private:
	EventQueue evts;

	uint32_t startTime = 0;
	bool disconnectMessageActive = false;

	ImageElement frame;
	ImageElement controllerRover;
	AnimElement signalAnim;
	AnimElement buttonAnim;
	ImageElement error;
	ImageElement description;


	std::unique_ptr<PairService> pair;

	void onLoop() override;

	void processInput(const Input::Data& evt);

	static constexpr const char* TextPaths[] = { "/spiffs/pair/textPair.raw", "/spiffs/pair/textFail.raw", "/spiffs/pair/textDC.raw" };
	static constexpr Color BackgroundColor = lgfx::color565(10, 6, 50);
	static constexpr glm::vec<2, uint16_t> ButtonPos = { 10, 70 };
	static constexpr glm::vec<2, uint16_t> ErrorPos = { 9, 72 };
	static constexpr glm::vec<2, uint16_t> SignalPos = {49, 22};
	static constexpr uint32_t DisconnectMessageDuration = 3000; // [ms]
};


#endif //PERSE_MISSIONCTRL_PAIRSCREEN_H
