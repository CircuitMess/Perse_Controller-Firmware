#include "LabelElement.h"

LabelElement::LabelElement(ElementContainer* parent, std::string text) : Element(parent), text(std::move(text)){}

void LabelElement::setText(const std::string& text){
	this->text = text;
}

void LabelElement::draw(Sprite* canvas){
	if(canvas == nullptr){
		return;
	}

	canvas->setFont(style.font);
	canvas->setTextDatum(style.datum);
	canvas->setTextSize(style.scale);

	if(style.shadingStyle.width > 0){
		canvas->setTextColor(style.shadingStyle.color);
		for(int i = 1; i <= style.shadingStyle.width; ++i){
			canvas->drawString(text.c_str(), x + i, y);
			canvas->drawString(text.c_str(), x - i, y);
			canvas->drawString(text.c_str(), x, y + i);
			canvas->drawString(text.c_str(), x, y - i);
			canvas->drawString(text.c_str(), x + i, y + i);
			canvas->drawString(text.c_str(), x - i, y + i);
			canvas->drawString(text.c_str(), x + i, y - i);
			canvas->drawString(text.c_str(), x - i, y - i);
		}
	}

	canvas->setTextColor(style.color);
	canvas->drawString(text.c_str(), x, y);
}

void LabelElement::setStyle(const TextStyle& style){
	this->style = style;
}

const TextStyle& LabelElement::getStyle() const{
	return style;
}

void LabelElement::setColor(uint16_t color){
	style.color = color;
}

