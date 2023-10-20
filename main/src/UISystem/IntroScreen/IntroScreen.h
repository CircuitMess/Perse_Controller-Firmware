#ifndef PERSE_MISSIONCTRL_INTROSCREEN_H
#define PERSE_MISSIONCTRL_INTROSCREEN_H

#include <glm.hpp>
#include <string>
#include "UISystem/Screen.h"
#include "UISystem/UIThread.h"

class IntroScreen : public Screen
{
public:
	static std::unique_ptr<Screen> createScreen(Sprite& canvas);

public:
	explicit IntroScreen(Sprite& canvas);
	virtual ~IntroScreen() override;

protected:
	virtual void onLoop() override;
	virtual void transition() override;

private:
	static constexpr glm::vec<3, uint8_t> backgroundColor = {38, 38, 73};
	static constexpr uint8_t crossMargin = 2;
	static constexpr float pauseDuration = 0.5f;
	static constexpr float moveDuration = 1.0f;

	uint64_t lastLoopTime = 0;
	uint8_t moveIndex = 0;
	float moveTime = 0.0f;
	float pausedTime = 0.0f;
	bool paused = false;
	std::vector<class ImageElement*> movingImages;

private:
	void createStaticElements();
	void createMovingImage(const std::string& path, uint16_t width, uint16_t height);
	static float easeInOut(float x);
};

#endif //PERSE_MISSIONCTRL_INTROSCREEN_H
