#ifndef PERSE_MISSIONCTRL_LABELELEMENT_H
#define PERSE_MISSIONCTRL_LABELELEMENT_H

#include "Element.h"

struct TextStyle {
	const lgfx::IFont* font;
	uint16_t color;
	uint16_t scale;
	lgfx::textdatum_t datum;
};

class LabelElement : public Element {
public:
	LabelElement(ElementContainer* parent, const char* text);
	~LabelElement() override = default;

	void setText(const char* text);
	void draw(Sprite* canvas) override;

	void setStyle(TextStyle style);
	TextStyle getStyle();

	void setColor(uint16_t color);

private:
	TextStyle style = DefaultStyle;
	const char* text = "Text";

	static constexpr TextStyle DefaultStyle = { &lgfx::fonts::Font0, TFT_BLACK, 1, textdatum_t::top_left };
};


#endif //PERSE_MISSIONCTRL_LABELELEMENT_H
