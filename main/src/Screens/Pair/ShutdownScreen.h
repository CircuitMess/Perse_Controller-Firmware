#ifndef PERSE_CTRL_SHUTDOWNSCREEN_H
#define PERSE_CTRL_SHUTDOWNSCREEN_H

#include "UISystem/Screen.h"
#include "UISystem/ImageElement.h"

class ShutdownScreen : public Screen {
public:
	explicit ShutdownScreen(Sprite& canvas);
	~ShutdownScreen() override = default;

private:
	static constexpr Color BackgroundColor = lgfx::color565(10, 6, 50);
	static constexpr const char* IconPath = "/spiffs/battery/shutdown.raw";
	ImageElement image;
};

#endif //PERSE_CTRL_SHUTDOWNSCREEN_H
