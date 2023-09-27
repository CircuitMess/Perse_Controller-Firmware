#ifndef PERSE_MISSIONCTRL_UITHREAD_H
#define PERSE_MISSIONCTRL_UITHREAD_H

#include "Util/Threaded.h"
#include "Devices/Display.h"
#include "Screen.h"

class UIThread : public Threaded{
public:
	UIThread(Display& display);
	~UIThread() override;

	void startScreen(std::function<std::unique_ptr<Screen>()> create);

protected:
	void loop() override;

private:
	Display& display;

	static constexpr uint32_t FrameTime = 25; // [ms]

	std::unique_ptr<Screen> currentScreen;
};


#endif //PERSE_MISSIONCTRL_UITHREAD_H
