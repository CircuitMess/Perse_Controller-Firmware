#ifndef PERSE_MISSIONCTRL_UITHREAD_H
#define PERSE_MISSIONCTRL_UITHREAD_H

#include "Util/Threaded.h"
#include "Devices/Display.h"

class Screen;

typedef std::function<std::unique_ptr<Screen>(Sprite& canvas)> ScreenCreateFunc;

class UIThread : public Threaded {
public:
	explicit UIThread(Display& display);
	~UIThread() override;

	void startScreen(const ScreenCreateFunc& create);

protected:
	void loop() override;

private:

	Display& display;

	static constexpr uint32_t FrameTime = 25; // [ms]

	ScreenCreateFunc creator;

	std::unique_ptr<Screen> currentScreen;
};


#endif //PERSE_MISSIONCTRL_UITHREAD_H
