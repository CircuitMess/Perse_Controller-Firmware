#ifndef PERSE_MISSIONCTRL_LABELELEMENT_H
#define PERSE_MISSIONCTRL_LABELELEMENT_H

#include "Element.h"

struct ShadingStyle {
	uint16_t color;
	uint16_t width;
};

struct TextStyle {
	const lgfx::IFont* font;
	uint16_t color;
	uint16_t scale;
	lgfx::textdatum_t datum;
	ShadingStyle shadingStyle;
};

class LabelElement : public Element {
public:
	LabelElement(ElementContainer* parent, std::string text, uint8_t layer = 0);
	~LabelElement() override = default;

	void setText(const std::string& text);
	void draw(Sprite* canvas) override;

	void setStyle(const TextStyle& style);
	const TextStyle& getStyle() const;

	void setColor(uint16_t color);

private:
	TextStyle style = DefaultStyle;
	std::string text = "Text";

	static constexpr TextStyle DefaultStyle = { &lgfx::fonts::Font0, TFT_BLACK, 1, textdatum_t::top_left, { TFT_BLACK, 0 }};
};


#endif //PERSE_MISSIONCTRL_LABELELEMENT_H
