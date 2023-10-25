#include "LabelElement.h"

LabelElement::LabelElement(ElementContainer* parent, std::string text) : Element(parent), text(std::move(text)){}

void LabelElement::setText(std::string text){
	this->text = std::move(text);
}

void LabelElement::draw(Sprite* canvas){
	canvas->setFont(style.font);
	canvas->setTextDatum(style.datum);
	canvas->setTextColor(style.color);
	canvas->setTextSize(style.scale);

	canvas->drawString(text.c_str(), x, y);
}

void LabelElement::setStyle(TextStyle style){
	this->style = style;
}

TextStyle LabelElement::getStyle(){
	return style;
}

void LabelElement::setColor(uint16_t color){
	style.color = color;
}

