#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Devices/Input.h"
#include "Devices/Encoders.h"
#include "Util/Events.h"
#include "Devices/LEDController.h"
#include "Pins.hpp"
#include "Periph/ADC.h"
#include "Devices/Display.h"
#include "Periph/I2C.h"
#include "Devices/AW9523.h"
#include <esp_spiffs.h>

bool initSPIFFS(){
	esp_vfs_spiffs_conf_t conf = {
			.base_path = "/spiffs",
			.partition_label = "storage",
			.max_files = 8,
			.format_if_mount_failed = false
	};

	auto ret = esp_vfs_spiffs_register(&conf);
	if(ret != ESP_OK){
		if(ret == ESP_FAIL){
			printf("FS: Failed to mount or format filesystem\n");
		}else if(ret == ESP_ERR_NOT_FOUND){
			printf("FS: Failed to find SPIFFS partition\n");
		}else{
			printf("FS: Failed to initialize SPIFFS (%s)\n", esp_err_to_name(ret));
		}
		return false;
	}

	return true;
}

void init(){
	initSPIFFS();

	auto pairPWM = new PWM(LED_PAIR, LEDC_CHANNEL_0);
	auto panic1PWM = new PWM(LED_PANIC1, LEDC_CHANNEL_1);
	auto panic2PWM = new PWM(LED_PANIC2, LEDC_CHANNEL_2);

	auto pairLED = new SingleLEDController(*pairPWM); pairLED->begin();
	auto panic1LED = new SingleLEDController(*panic1PWM); panic1LED->begin();
	auto panic2LED = new SingleLEDController(*panic2PWM); panic2LED->begin();

	for(auto& led : { pairLED, panic1LED, panic2LED }){
		led->breathe(0, 255, 1000);
	}


	EventQueue* queue = new EventQueue(12);
	Events::listen(Facility::Input, queue);
	Events::listen(Facility::Encoders, queue);
	auto scanner = new ThreadedClosure([queue](){
		Event evt;
		if(!queue->get(evt, portMAX_DELAY)) return;

		if(evt.facility == Facility::Input){
			auto data = (Input::Data*) evt.data;
			printf("Btn %s %s\n", Input::PinLabels.at(data->btn), data->action == Input::Data::Press ? "press" : "release");
		}else if(evt.facility == Facility::Encoders){
			auto data = (Encoders::Data*) evt.data;
			printf("Enc %s moved %d\n", Encoders::Labels.at(data->enc), data->dir);
		}

		free(evt.data);
	}, "Scanner", 4 * 1024);
	scanner->start();

	auto input = new Input();
	auto encoders = new Encoders();


	auto adcSamples = [](ADC* adc, const char* name){
		int* val = new int(0);
		auto scanner = new ThreadedClosure([adc, name, val](){
			int newVal = adc->sample();
			if(newVal != *val){
				printf("%s: %d\n", name, newVal);
				*val = newVal;
			}
			vTaskDelay(10);
		}, name, 4 * 1024);
		scanner->start();
	};

	auto adcQual = new ADC((gpio_num_t) PIN_QUAL, 0.1);
	auto adcBatt = new ADC((gpio_num_t) PIN_BATT, 0.1);
	auto adcJoyHor = new ADC((gpio_num_t) JOY_H, 0.05);
	auto adcJoyVer = new ADC((gpio_num_t) JOY_V, 0.05);

	//adcSamples(adcQual, "Quality");
	//adcSamples(adcBatt, "Battery");
	//adcSamples(adcJoyHor, "Joy-Horizontal");
	//adcSamples(adcJoyVer, "Joy-Vertical");


	auto display = new Display();
	display->getLGFX().drawBmpFile("/spiffs/mars.bmp", 0, 0, 128, 128, 0, 0, 1, 1);

	auto bl = new PWM(PIN_BL, LEDC_CHANNEL_3);
	bl->setDuty(80);


	auto i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);
	auto aw = new AW9523(*i2c, 0x5b);
	aw->setCurrentLimit(AW9523::IMAX_1Q);
	for(int i = 0; i < 16; i++){
		aw->pinMode(i, AW9523::OUT);
		aw->write(i, true);
	}

	auto state = new bool(0);
	auto dimmer = new ThreadedClosure([aw, state](){
		for(int i = 0; i < 16; i++){
			aw->write(i, *state);
		}
		*state = !*state;
		vTaskDelay(1000);
	}, "Dimmer", 2048);
	dimmer->start();
}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
