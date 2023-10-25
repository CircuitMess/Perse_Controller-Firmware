#ifndef PERSE_MISSIONCTRL_IMAGEELEMENT_H
#define PERSE_MISSIONCTRL_IMAGEELEMENT_H

#include "Element.h"

class ImageElement : public Element {
public:
	ImageElement(ElementContainer* parent, const char* path, uint16_t width, uint16_t height);
	~ImageElement() override;

	void setPath(const char* path);
	void draw(Sprite* canvas) override;

private:
	FILE* file;
	uint16_t width, height;

	/**
	 * @brief Draw a raw binary RGB565 file icon on a sprite at the specified position and size.
	 *
	 * @param sprite The sprite on which the file icon will be drawn.
	 * @param icon The file icon to be drawn.
	 * @param x The x-coordinate of the top-left corner of the file icon.
	 * @param y The y-coordinate of the top-left corner of the file icon.
	 * @param width The width of the file icon.
	 * @param height The height of the file icon.
	 * @param scale The scaling factor of the file icon (default: 1).
	 * @param maskingColor The color used to mask the file icon (default: TFT_TRANSPARENT).
	 */
	static void drawFile(Sprite& sprite, FILE* file, int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t scale, int32_t maskingColor);


};


#endif //PERSE_MISSIONCTRL_IMAGEELEMENT_H
