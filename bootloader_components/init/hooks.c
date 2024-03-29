#include "esp_attr.h"
#include "hal/gpio_hal.h"
#include "esp_chip_info.h"

/** Function used to tell the linker to include this file with all its symbols. */
void bootloader_hooks_include(void){}

void setLEDs(){
	esp_chip_info_t info;
	esp_chip_info(&info);

	if(info.model == CHIP_ESP32C3){
		// Basic ctrl
		gpio_ll_output_enable(&GPIO, GPIO_NUM_6);
		gpio_ll_set_level(&GPIO, GPIO_NUM_6, 1);

		gpio_ll_output_enable(&GPIO, GPIO_NUM_8);
		gpio_ll_set_level(&GPIO, GPIO_NUM_8, 0);

		gpio_ll_output_enable(&GPIO, GPIO_NUM_9);
		gpio_ll_set_level(&GPIO, GPIO_NUM_9, 0);
	}
}

void IRAM_ATTR bootloader_before_init(void){
	/* Keep in my mind that a lot of functions cannot be called from here
	 * as system initialization has not been performed yet, including
	 * BSS, SPI flash, or memory protection. */
	// ESP_LOGI("HOOK", "This hook is called BEFORE bootloader initialization");
	setLEDs();
}

void bootloader_after_init(void){
	// ESP_LOGI("HOOK", "This hook is called AFTER bootloader initialization");
}
