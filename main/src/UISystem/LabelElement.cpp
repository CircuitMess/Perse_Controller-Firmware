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
	canvas->setTextColor(style.color);
	canvas->setTextSize(style.scale);

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

