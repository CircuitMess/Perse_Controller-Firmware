#include "IntroScreen.h"
#include "UISystem/ImageElement.h"
#include "Color.h"
#include "Util/stdafx.h"
#include "Util/Services.h"
#include "Screens/Pair/PairScreen.h"
#include "Devices/Battery.h"
#include "Screens/Pair/ShutdownScreen.h"

const std::vector<std::tuple<std::string, uint16_t, uint16_t>> IntroScreen::imageInfos = {
		{ "/spiffs/intro/logo-cm.raw",    91,  91 },
		{ "/spiffs/intro/logo-geek.raw",  97,  81 },
		{ "/spiffs/intro/logo-space.raw", 116, 29 },
		{ "/spiffs/intro/logo-perse.raw", 72,  71 }
};

const std::string IntroScreen::crossPath = "/spiffs/intro/cross.raw";

std::unique_ptr<Screen> IntroScreen::createScreen(Sprite& canvas) {
	return std::make_unique<IntroScreen>(canvas);
}

IntroScreen::IntroScreen(Sprite& canvas) : Screen(canvas), lastLoopTime(micros()) {
	createStaticElements();

	for (const std::tuple<std::string, uint16_t, uint16_t>& tuple : imageInfos) {
			createMovingImage(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple));
	}

	setBgColor(backgroundColor);
}

IntroScreen::~IntroScreen() = default;

void IntroScreen::preDraw() {
	setBgColor(backgroundColor);
}

void IntroScreen::onLoop() {
	const float deltaTime = (micros() - lastLoopTime) / 1000000.0f; // [s]
	lastLoopTime = micros();

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

	if (!movingImages.empty() && movingImages.back() != nullptr && movingImages.back()->getY() <= (getHeight() - movingImages.back()->getHeight()) / 2) {
		Battery* battery = (Battery*) Services.get(Service::Battery);
		if(battery->getLevel() == Battery::Critical){
			transition([](Sprite& canvas){ return std::make_unique<ShutdownScreen>(canvas); });
			return;
		}
		transition([](Sprite& canvas){ return std::make_unique<PairScreen>(canvas); });
		return;
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
	}

	if (paused) {
		++moveIndex;
	}
}

void IntroScreen::createStaticElements() {
	ImageElement* cross = nullptr;

	for (int x = 0; x < 2; ++x) {
		for (int y = 0; y < 2; ++y) {
			cross = new ImageElement(this, crossPath.c_str(), 9, 9);
			cross->setPos(crossMargin + x * (getWidth() - cross->getWidth() - 2 * crossMargin), crossMargin + y * (getHeight() - cross->getHeight() - 2 * crossMargin));
		}
	}
}

void IntroScreen::createMovingImage(const std::string& path, uint16_t width, uint16_t height) {
	ImageElement* image = new ImageElement(this, path.c_str(), width, height);
	movingImages.emplace_back(image);
	image->setPos((getWidth() - image->getWidth()) / 2, (getHeight() - image->getHeight()) / 2 + movingImages.size() * getHeight());
}

float IntroScreen::easeInOut(float x) {
	if (x < 0.5f) {
		return 4.0f * std::pow(x, 3);
	}

	return 1.0f - std::pow(-2.0f * x + 2, 3) / 2;
}