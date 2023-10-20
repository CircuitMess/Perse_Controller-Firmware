#include "IntroScreen.h"
#include "UISystem/ImageElement.h"
#include "Color.h"
#include "Util/stdafx.h"
#include "Util/Services.h"

std::unique_ptr<Screen> IntroScreen::createScreen(Sprite &canvas) {
	return std::make_unique<IntroScreen>(canvas);
}

IntroScreen::IntroScreen(Sprite& canvas) : Screen(canvas), lastLoopTime(micros()) {
	createStaticElements();

	createMovingImage("/spiffs/logo-cm.raw", 91, 91);
	createMovingImage("/spiffs/logo-geek.raw", 97, 81);
	createMovingImage("/spiffs/logo-space.raw", 116, 29);
	createMovingImage("/spiffs/logo-artemis.raw", 105, 14);
}

IntroScreen::~IntroScreen() = default;

void IntroScreen::onLoop() {
	Screen::onLoop();

	float deltaTime = (micros() - lastLoopTime) / 1000000.0f; // [s]
	lastLoopTime = micros();

	canvas.clear(C_RGB(backgroundColor.x, backgroundColor.y, backgroundColor.z));

	if (paused && pausedTime <= pauseDuration) {
		pausedTime += deltaTime;
		moveTime = 0.0f;
		return;
	}
	else {
		pausedTime = 0.0f;
		moveTime += deltaTime;
		paused = false;
	}

	const float deltaPercent = easeInOut(std::clamp(moveTime / moveDuration, 0.0f, 1.0f));
	const float delta = getHeight() * deltaPercent;

	for (int i = 0; i < movingImages.size(); ++i) {
		ImageElement* movingImage = movingImages[i];
		if (movingImage == nullptr) {
			continue;
		}

		const int16_t baseImageY = (i + 1) * getHeight() + (getHeight() - movingImage->getHeight()) / 2 - moveIndex * getHeight();

		const int16_t startingY = movingImage->getY();

		movingImage->setY(baseImageY - delta);

		const int16_t endingY = movingImage->getY();

		const int32_t centerY = (getHeight() - movingImage->getHeight()) / 2;

		if (startingY > centerY && endingY <= centerY) {
			paused = true;
		}

		if (i == movingImages.size() - 1 && endingY <= - movingImage->getHeight()) {
			// If all images are off screen, transition
			transition();
		}
	}

	if (paused) {
		++moveIndex;
	}
}

void IntroScreen::transition() {
	canvas.clear(0);

	UIThread* thread = (UIThread*)Services.get(Service::UI);
	if (thread == nullptr) {
		return;
	}

	// TODO: transition to the next screen
	//thread->startScreen();
}

void IntroScreen::createStaticElements() {
	ImageElement* cross = nullptr;

	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 2; ++y) {
			cross = new ImageElement(this, "/spiffs/cross.raw", 9, 9);
			cross->setPos(crossMargin + x * (getWidth() - cross->getWidth() - 2 * crossMargin), crossMargin + y * (getHeight() - cross->getHeight() - 2 * crossMargin));
			elements.emplace_back(cross);
		}
	}
}

void IntroScreen::createMovingImage(const std::string& path, uint16_t width, uint16_t height) {
	ImageElement* image = new ImageElement(this, path.c_str(), width, height);
	elements.emplace_back(image);
	movingImages.emplace_back(image);
	image->setPos((getWidth() - image->getWidth()) / 2, (getHeight() - image->getHeight()) / 2 + movingImages.size() * getHeight());
}

float IntroScreen::easeInOut(float x) {
	if (x < 0.5f) {
		return 4.0f * std::pow(x, 3);
	}

	return 1.0f - std::pow(-2.0f * x + 2, 3) / 2;
}