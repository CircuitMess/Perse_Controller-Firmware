#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <esp_sleep.h>
#include "Periph/WiFiSTA.h"
#include "Periph/SPIFFS.h"
#include "Periph/ADC.h"
#include "Devices/Battery.h"
#include "Devices/Input.h"
#include "Services/TCPClient.h"
#include "Services/Comm.h"
#include "Services/RoverState.h"
#include "Services/LEDService.h"
#include "Services/Settings.h"
#include "Services/InactivityService.h"
#include "Util/Services.h"
#include "Pins.hpp"
#include "Util/stdafx.h"
#include "JigHWTest/JigHWTest.h"

#ifdef CTRL_TYPE_MISSIONCTRL

#include "Periph/I2C.h"
#include "Devices/Potentiometers.h"
#include "Devices/Display.h"
#include "Devices/Backlight.h"
#include "Devices/AW9523.h"
#include "Devices/Joystick.h"
#include "Devices/Encoders.h"
#include "UISystem/UIThread.h"
#include "Screens/IntroScreen.h"

#elifdef CTRL_TYPE_BASIC

#include "Services/StateMachine.h"
#include "States/PairState.h"
#include "Services/BatteryLowService.h"

#endif

void preShutdown(){
#ifdef CTRL_TYPE_MISSIONCTRL
	if(Backlight* bl = (Backlight*) Services.get(Service::Backlight)){
		bl->fadeOut();
	}
#endif

	if(LEDService* led = (LEDService*) Services.get(Service::LED)){
		for(int i = 0; i < (uint8_t) LED::COUNT; i++){
			led->off((LED) i);
		}
	}

	//delay to ensure queued LED off instructions are processed
	delayMillis(1000);

#ifdef CTRL_TYPE_BASIC
	//necessary since LED_ARMPINCH (GPIO8) is internally pulled-up in deep sleep
		gpio_config_t conf{
				1 << LED_ARMPINCH, GPIO_MODE_OUTPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_ENABLE, GPIO_INTR_DISABLE
		};
		gpio_config(&conf);
		gpio_set_level((gpio_num_t) LED_ARMPINCH, 0);
		gpio_hold_en((gpio_num_t) LED_ARMPINCH);
		gpio_deep_sleep_hold_en();
#endif
}

[[noreturn]] void shutdown(){
#ifdef CTRL_TYPE_MISSIONCTRL
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO));
#endif
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RC_FAST, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_CPU, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL));
	esp_deep_sleep_start();
}

void init(){
	if(JigHWTest::checkJig()){
		printf("Jig\n");
		auto test = new JigHWTest();
		test->start();
		vTaskDelete(nullptr);
	}

	auto adc = new ADC(ADC_UNIT_1);

	auto battery = new Battery(*adc);
	Services.set(Service::Battery, battery);

#ifdef CTRL_TYPE_MISSIONCTRL
	auto i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);
	auto aw9523 = new AW9523(*i2c, 0x5b);
#endif

	if(battery->isShutdown()){
	#ifdef CTRL_TYPE_MISSIONCTRL
		aw9523->resetDimOutputs();
	#endif
		shutdown();
		return;
	}

#ifdef CTRL_TYPE_MISSIONCTRL
	auto led = new LEDService(*aw9523);
#elifdef CTRL_TYPE_BASIC
	auto led = new LEDService();
#endif

	Services.set(Service::LED, led);
	led->on(LED::Power);

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

#ifdef CTRL_TYPE_MISSIONCTRL
	auto encoders = new Encoders();

	auto joy = new Joystick(*adc);
	Services.set(Service::Joystick, joy);

	auto display = new Display();

	auto bl = new Backlight(LEDC_CHANNEL_0);
	Services.set(Service::Backlight, bl);

	auto uiThread = new UIThread(*display);
	Services.set(Service::UI, uiThread);

	auto potentiometers = new Potentiometers(*adc);
	Services.set(Service::Potentiometers, potentiometers);

	uiThread->startScreen(&IntroScreen::createScreen);

	uiThread->start();
	bl->fadeIn();
#elifdef CTRL_TYPE_BASIC
	auto stateMachine = new StateMachine();
	Services.set(Service::StateMachine, stateMachine);

	stateMachine->transition<PairState>();
	stateMachine->begin();

	auto lowBatteryService = new BatteryLowService();
	Services.set(Service::LowBattery, lowBatteryService);
#endif

	auto inactivityService = new InactivityService();

	battery->setShutdownCallback([](){
		preShutdown();
		shutdown();
	});

	battery->begin();
}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
