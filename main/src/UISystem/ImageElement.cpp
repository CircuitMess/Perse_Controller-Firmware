#include "ImageElement.h"
#include "esp_log.h"

static const char* TAG = "ImageElement";

ImageElement::ImageElement(ElementContainer* parent, const char* path, uint16_t width, uint16_t height) :
		Element(parent), width(width), height(height){
	file = fopen(path, "r");
	if(ferror(file) != 0){
		ESP_LOGE(TAG, "Couldn't open file %s", path);
	}
}

ImageElement::~ImageElement(){
	if(!ferror(file)){
		fclose(file);
	}
}

void ImageElement::setPath(const char* path){
	file = fopen(path, "r");
	if(ferror(file) != 0){
		ESP_LOGE(TAG, "Couldn't open file %s", path);
	}
}

void ImageElement::draw(Sprite* canvas){
	drawFile(*canvas, file, x, y, width, height, 1, TFT_TRANSPARENT);
}

void ImageElement::drawFile(Sprite& sprite, FILE* icon, int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t scale, int32_t maskingColor){
	static constexpr uint32_t BufferLength = 512;

	fseek(icon, 0, SEEK_SET);
	uint16_t buffer[BufferLength];
	size_t bufferPos = 0;
	size_t available = fread(reinterpret_cast<uint8_t*>(buffer), 1, BufferLength, icon) / 2;

	for(int i = 0; i < height; i++){
		for(int j = 0; j < width; j++){
			if(bufferPos == available){
				available = fread(reinterpret_cast<uint8_t*>(buffer), 1, BufferLength, icon) / 2;
				if(available == 0){
					return;
				}
				bufferPos = 0;
			}

			uint16_t color = buffer[bufferPos++];

			if(color != maskingColor || maskingColor == -1){
				sprite.fillRect(x + j * scale, y + i * scale, scale, scale, color);
			}
		}
	}
}

