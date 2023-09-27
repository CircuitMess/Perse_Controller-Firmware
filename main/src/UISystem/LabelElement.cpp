#include "LabelElement.h"

LabelElement::LabelElement(ElementContainer* parent, const char* text) : Element(parent){}

void LabelElement::setText(const char* text){
	this->text = text;
}

void LabelElement::draw(Sprite* canvas){
	canvas->setFont(style.font);
	canvas->setTextDatum(style.datum);
	canvas->setTextColor(style.color);
	canvas->setTextSize(style.scale);

	canvas->drawString(text, x, y);
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

