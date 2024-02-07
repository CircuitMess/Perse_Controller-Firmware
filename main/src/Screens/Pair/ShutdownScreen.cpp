#include "ShutdownScreen.h"

ShutdownScreen::ShutdownScreen(Sprite& canvas) : Screen(canvas), image(this, IconPath, 91, 48){
	setBgColor(BackgroundColor);
	image.setPos((int16_t) ((this->getWidth() - image.getWidth()) / 2.0),
				 (int16_t) ((this->getHeight() - image.getHeight()) / 2.0));
}
