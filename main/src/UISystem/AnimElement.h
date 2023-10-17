#ifndef PERSE_MISSIONCTRL_ANIMELEMENT_H
#define PERSE_MISSIONCTRL_ANIMELEMENT_H

#include "Element.h"
#include "GIF/GIFSprite.h"

class AnimElement : public Element {
public:
	AnimElement(ElementContainer* parent, const char* path);
	void setPath(const char* path);
	void setLoopMode(GIF::LoopMode loopMode);

	void start();
	void stop();
	void reset();

	void draw(Sprite* canvas) override;
	void loop() override;
private:
	std::unique_ptr<GIFSprite> gif;
	uint32_t lastMicros = 0;
};


#endif //PERSE_MISSIONCTRL_ANIMELEMENT_H
