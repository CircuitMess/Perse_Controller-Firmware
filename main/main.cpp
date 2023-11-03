#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <esp_sleep.h>
#include "Periph/WiFiSTA.h"
#include "Periph/SPIFFS.h"
#include "Periph/I2C.h"
#include "Periph/ADC.h"
#include "Devices/Battery.h"
#include "Devices/Display.h"
#include "Devices/Backlight.h"
#include "Devices/Input.h"
#include "Devices/AW9523.h"
#include "Devices/Joystick.h"
#include "Devices/Encoders.h"
#include "Services/TCPClient.h"
#include "Services/Comm.h"
#include "Services/RoverState.h"
#include "Services/LEDService.h"
#include "UISystem/UIThread.h"
#include "Screens/IntroScreen.h"
#include "Util/Services.h"
#include "Pins.hpp"

[[noreturn]] void shutdown(){
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RC_FAST, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_CPU, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL));
	esp_deep_sleep_start();
}

void init(){
	auto adc = new ADC(ADC_UNIT_1);

	auto battery = new Battery(*adc);
	if(battery->isShutdown()){
		shutdown();
		return;
	}

	const gpio_config_t cfg = {
			.pin_bit_mask = 0,
			.mode = GPIO_MODE_OUTPUT
	};
	gpio_config(&cfg);

	auto i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);
	auto aw9523 = new AW9523(*i2c, 0x5b);
	auto led = new LEDService(*aw9523);
	Services.set(Service::LED, led);

	led->on(LED::Power);
	led->off(LED::Pair);
	led->off(LED::PanicLeft);
	led->off(LED::PanicRight);

	auto ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	const auto spiffs = new SPIFFS();

	auto settings = new Settings();
	Services.set(Service::Settings, settings);

	auto wifi = new WiFiSTA();
	Services.set(Service::WiFi, wifi);
	auto tcp = new TCPClient();
	Services.set(Service::TCP, tcp);

	auto rover = new RoverState();
	Services.set(Service::RoverState, rover);

	auto comm = new Comm();
	Services.set(Service::Comm, comm);

	auto input = new Input();
	Services.set(Service::Input, input);

	auto encoders = new Encoders();

	auto joy = new Joystick(*adc);
	Services.set(Service::Joystick, joy);

	auto display = new Display();

	auto bl = new Backlight(LEDC_CHANNEL_0);
	Services.set(Service::Backlight, bl);

	auto uiThread = new UIThread(*display);
	Services.set(Service::UI, uiThread);

	uiThread->startScreen(&IntroScreen::createScreen);

	uiThread->start();
	bl->fadeIn();

	battery->begin();
}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
