#include "JigHWTest.h"
#include <Pins.hpp>
#include <soc/efuse_reg.h>
#include <esp_efuse.h>
#include <iostream>
#include <esp_mac.h>
#include <driver/adc.h>
#include <driver/ledc.h>

#ifdef CTRL_TYPE_BASIC
#include <map>
#include "Devices/Input.h"
#include "Util/Events.h"
#elifdef CTRL_TYPE_MISSIONCTRL
#include "SPIFFSChecksum.hpp"
#endif

JigHWTest* JigHWTest::test = nullptr;
adc_oneshot_unit_handle_t JigHWTest::hndl = nullptr;

#ifdef CTRL_TYPE_MISSIONCTRL
Display* JigHWTest::display = nullptr;
LGFX_Device* JigHWTest::canvas = nullptr;
I2C* JigHWTest::i2c = nullptr;
AW9523* JigHWTest::aw9523 = nullptr;
#endif

JigHWTest::JigHWTest(){
	test = this;

#ifdef CTRL_TYPE_MISSIONCTRL
	display = new Display();
	canvas = &display->getLGFX();

	i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);
	aw9523 = new AW9523(*i2c, 0x5b);

	tests.push_back({ JigHWTest::SPIFFSTest, "SPIFFS", [](){} });
	tests.push_back({ JigHWTest::AW9523Check, "AW9523", [](){} });
#endif

	tests.push_back({ JigHWTest::BatteryCalib, "Battery calibration", [](){} });
	tests.push_back({ JigHWTest::BatteryCheck, "Battery check", [](){} });

#ifdef CTRL_TYPE_BASIC
	tests.push_back({ JigHWTest::ButtonCheck, "Button check", [](){} });
#endif
}

bool JigHWTest::checkJig(){
	char buf[7];
	int wp = 0;

	uint32_t start = millis();
	int c;
	while(millis() - start < CheckTimeout){
		vTaskDelay(1);
		c = getchar();
		if(c == EOF) continue;
		buf[wp] = (char) c;
		wp = (wp + 1) % 7;

		for(int i = 0; i < 7; i++){
			int match = 0;
			static const char* target = "JIGTEST";

			for(int j = 0; j < 7; j++){
				match += buf[(i + j) % 7] == target[j];
			}

			if(match == 7) return true;
		}
	}

	return false;
}


void JigHWTest::start(){
	uint64_t _chipmacid = 0LL;
	esp_efuse_mac_get_default((uint8_t*) (&_chipmacid));
	printf("\nTEST:begin:%llx\n", _chipmacid);

#ifdef CTRL_TYPE_MISSIONCTRL
	gpio_config_t cfg = {
			.pin_bit_mask = ((uint64_t) 1) << PIN_BL,
			.mode = GPIO_MODE_OUTPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&cfg);
	gpio_set_level((gpio_num_t) PIN_BL, 0);

	canvas->clear(0);
	rgb();

	canvas->clear(TFT_BLACK);
	canvas->setTextColor(TFT_GOLD);
	canvas->setTextWrap(true, true);
	canvas->setTextDatum(textdatum_t::middle_center);

	canvas->setTextFont(0);
	canvas->setTextSize(1);
	canvas->setCursor(0, 6);

	canvas->print("Mission Ctrl test");
	canvas->setCursor(canvas->width() / 2, 16);
	canvas->println();
#endif

	bool pass = true;
	for(const Test& test : tests){
		currentTest = test.name;

		printf("TEST:startTest:%s\n", currentTest);

		bool result = test.test();

	#ifdef CTRL_TYPE_MISSIONCTRL
		canvas->setTextColor(TFT_WHITE);
		canvas->printf("%s: ", test.name);
		canvas->setTextColor(result ? TFT_GREEN : TFT_RED);
		canvas->printf("%s\n", result ? "PASSED" : "FAILED");
	#endif

		printf("TEST:endTest:%s\n", result ? "pass" : "fail");

		if(!(pass &= result)){
			if(test.onFail){
				test.onFail();
			}

			break;
		}
	}

	if(!pass){
		printf("TEST:fail:%s\n", currentTest);
		vTaskDelete(nullptr);
	}

	printf("TEST:passall\n");

	//------------------------------------------------------
#ifdef CTRL_TYPE_MISSIONCTRL
	canvas->print("\n\n");
	canvas->setTextColor(TFT_GREEN);
	canvas->print("All OK!");
#endif

	AudioVisualTest();
}

#ifdef CTRL_TYPE_MISSIONCTRL
void JigHWTest::rgb(){
	static const char* names[] = { "RED", "GREEN", "BLUE" };
	static const uint16_t colors[] = { TFT_RED, TFT_GREEN, TFT_BLUE };
	for(int i = 0; i < 3; i++){
		canvas->clear(colors[i]);
		canvas->setCursor(20, 40);
		canvas->setTextFont(0);
		canvas->setTextSize(2);
		canvas->print(names[i]);
		vTaskDelay(500);
	}
}
#endif

void JigHWTest::log(const char* property, const char* value){
	printf("%s:%s:%s\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, float value){
	printf("%s:%s:%f\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, double value){
	printf("%s:%s:%lf\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, bool value){
	printf("%s:%s:%s\n", currentTest, property, value ? "TRUE" : "FALSE");
}

void JigHWTest::log(const char* property, uint32_t value){
	printf("%s:%s:%lu\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, int32_t value){
	printf("%s:%s:%ld\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, const std::string& value){
	printf("%s:%s:%s\n", currentTest, property, value.c_str());
}

bool JigHWTest::BatteryCalib(){
	if(Battery::getVoltOffset() != 0){
		test->log("calibrated", (int32_t) Battery::getVoltOffset());
	#ifdef CTRL_TYPE_MISSIONCTRL
		canvas->print("fused. ");
	#endif
		return true;
	}

	constexpr uint16_t numReadings = 50;
	constexpr uint16_t readDelay = 50;
	uint32_t reading = 0;

	adc_oneshot_unit_init_cfg_t config = {
			.unit_id = ADC_UNIT_1,
			.ulp_mode = ADC_ULP_MODE_DISABLE
	};
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&config, &hndl));

	adc_unit_t unit;
	adc_channel_t chan;
	adc_oneshot_io_to_channel(PIN_BATT, &unit, &chan);

	adc_oneshot_chan_cfg_t cfg = {
			.atten = ADC_ATTEN_DB_11,
			.bitwidth = ADC_BITWIDTH_12
	};
	adc_oneshot_config_channel(hndl, chan, &cfg);

	for(int i = 0; i < numReadings; i++){
		int val;
		adc_oneshot_read(hndl, chan, &val);
		reading += val;
		vTaskDelay(readDelay / portTICK_PERIOD_MS);
	}
	reading /= numReadings;

	uint32_t mapped = Battery::mapRawReading(reading);

	int16_t offset = referenceVoltage - mapped;

	test->log("reading", reading);
	test->log("mapped", mapped);
	test->log("offset", (int32_t) offset);

	if(abs(offset) >= 1000){
		test->log("offset too big, read voltage: ", (uint32_t) mapped);
		return false;
	}

	uint8_t offsetLow = offset & 0xff;
	uint8_t offsetHigh = (offset >> 8) & 0xff;

	esp_efuse_batch_write_begin();
	esp_efuse_write_field_blob((const esp_efuse_desc_t**) efuse_adc1_low, &offsetLow, 8);
	esp_efuse_write_field_blob((const esp_efuse_desc_t**) efuse_adc1_high, &offsetHigh, 8);
	esp_efuse_batch_write_commit();

	return true;
}


bool JigHWTest::BatteryCheck(){
	adc_unit_t unit;
	adc_channel_t chan;
	adc_oneshot_io_to_channel(PIN_BATT, &unit, &chan);

	adc_oneshot_chan_cfg_t cfg = {
			.atten = ADC_ATTEN_DB_11,
			.bitwidth = ADC_BITWIDTH_12
	};
	adc_oneshot_config_channel(hndl, chan, &cfg);

	constexpr uint16_t numReadings = 50;
	constexpr uint16_t readDelay = 10;
	uint32_t reading = 0;

	for(int i = 0; i < numReadings; i++){
		int val;
		adc_oneshot_read(hndl, chan, &val);
		reading += val;
		vTaskDelay(readDelay / portTICK_PERIOD_MS);
	}
	reading /= numReadings;

	uint32_t voltage = Battery::mapRawReading(reading) + Battery::getVoltOffset();
	if(voltage < referenceVoltage - 100 || voltage > referenceVoltage + 100){
		test->log("raw", reading);
		test->log("mapped", (int32_t) Battery::mapRawReading(reading));
		test->log("offset", (int32_t) Battery::getVoltOffset());
		test->log("mapped+offset", voltage);
		return false;
	}

	return true;
}

#ifdef CTRL_TYPE_BASIC
bool JigHWTest::ButtonCheck(){
	gpio_config_t cfg = {
			.pin_bit_mask = ((uint64_t) 1) << LED_PAIR,
			.mode = GPIO_MODE_OUTPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_DISABLE
	};

	gpio_config(&cfg);

	cfg.pin_bit_mask = ((uint64_t) 1) << LED_WARN;

	gpio_config(&cfg);

	new Input();
	EventQueue queue(1);
	Events::listen(Facility::Input, &queue);

	std::map<Input::Button, bool> buttonPresses;

	while(true){
		Event evt;
		if(queue.get(evt, 1)){
			auto data = (Input::Data*) evt.data;

			if(data->action == Input::Data::Press){
				test->log("Button Pressed", (uint32_t) data->btn);

				buttonPresses[data->btn] = true;

				gpio_set_level((gpio_num_t) LED_PAIR, 1);

				delayMillis(500);

				gpio_set_level((gpio_num_t) LED_PAIR, 0);
			}

			free(evt.data);
		}

		bool allPressed = true;
		for(int button = 0; button <= 5; ++button){
			if(!buttonPresses.contains((Input::Button) button)){
				allPressed = false;
				break;
			}

			if(!buttonPresses[(Input::Button) button]){
				allPressed = false;
				break;
			}
		}

		if(allPressed){
			gpio_set_level((gpio_num_t) LED_WARN, 0);
			break;
		}else{
			gpio_set_level((gpio_num_t) LED_WARN, 1);
		}
	}

	return true;
}
#endif

#ifdef CTRL_TYPE_MISSIONCTRL
bool JigHWTest::AW9523Check(){
	if(i2c->probe(0x5b, 200) == ESP_OK){
		return true;
	}

	return false;
}

bool JigHWTest::SPIFFSTest(){
	auto ret = esp_vfs_spiffs_register(&spiffsConfig);
	if(ret != ESP_OK){
		test->log("spiffs", false);
		return false;
	}

	for(const auto& f : SPIFFSChecksums){
		auto file = fopen(f.name, "rb");
		if(file == nullptr){
			test->log("missing", f.name);
			return false;
		}

		uint32_t sum = calcChecksum(file);
		fclose(file);

		if(sum != f.sum){
			test->log("file", f.name);
			test->log("expected", (uint32_t) f.sum);
			test->log("got", (uint32_t) sum);

			return false;
		}
	}

	return true;
}

uint32_t JigHWTest::calcChecksum(FILE* file){
	if(file == nullptr) return 0;

#define READ_SIZE 512

	uint32_t sum = 0;
	uint8_t b[READ_SIZE];
	size_t read = 0;
	while((read = fread(b, 1, READ_SIZE, file))){
		for(int i = 0; i < read; i++){
			sum += b[i];
		}
	}

	return sum;
}
#endif

void JigHWTest::AudioVisualTest(){
	ledc_timer_config_t ledc_timer = {
			.speed_mode       = static_cast<ledc_mode_t>((LEDC_CHANNEL_0 / 8)),
			.duty_resolution  = LEDC_TIMER_10_BIT,
			.timer_num        = static_cast<ledc_timer_t>(((LEDC_CHANNEL_0 / 2) % 4)),
			.freq_hz          = 5000,
			.clk_cfg          = LEDC_AUTO_CLK
	};
	ledc_timer_config(&ledc_timer);

	ledc_channel_config_t ledc_channel = {
			.gpio_num       = LED_POWER,
			.speed_mode     = static_cast<ledc_mode_t>((LEDC_CHANNEL_0 / 8)),
			.channel        = LEDC_CHANNEL_0,
			.intr_type      = LEDC_INTR_DISABLE,
			.timer_sel      = static_cast<ledc_timer_t>(((LEDC_CHANNEL_0 / 2) % 4)),
			.duty           = 0,
			.hpoint         = 0,
			.flags = { .output_invert = true }
	};
	ledc_channel_config(&ledc_channel);

	ledc_timer = {
			.speed_mode       = static_cast<ledc_mode_t>((LEDC_CHANNEL_1 / 8)),
			.duty_resolution  = LEDC_TIMER_10_BIT,
			.timer_num        = static_cast<ledc_timer_t>(((LEDC_CHANNEL_1 / 2) % 4)),
			.freq_hz          = 5000,
			.clk_cfg          = LEDC_AUTO_CLK
	};
	ledc_timer_config(&ledc_timer);

	ledc_channel = {
			.gpio_num       = LED_PAIR,
			.speed_mode     = static_cast<ledc_mode_t>((LEDC_CHANNEL_1 / 8)),
			.channel        = LEDC_CHANNEL_1,
			.intr_type      = LEDC_INTR_DISABLE,
			.timer_sel      = static_cast<ledc_timer_t>(((LEDC_CHANNEL_1 / 2) % 4)),
			.duty           = 0,
			.hpoint         = 0,
			.flags = { .output_invert = true }
	};
	ledc_channel_config(&ledc_channel);

	ledc_timer = {
			.speed_mode       = static_cast<ledc_mode_t>((LEDC_CHANNEL_2 / 8)),
			.duty_resolution  = LEDC_TIMER_10_BIT,
			.timer_num        = static_cast<ledc_timer_t>(((LEDC_CHANNEL_2 / 2) % 4)),
			.freq_hz          = 5000,
			.clk_cfg          = LEDC_AUTO_CLK
	};
	ledc_timer_config(&ledc_timer);

	ledc_channel = {
		#ifdef CTRL_TYPE_MISSIONCTRL
			.gpio_num       = LED_PANIC_L,
		#elifdef CTRL_TYPE_BASIC
			.gpio_num		= LED_ARMPINCH,
		#endif
			.speed_mode     = static_cast<ledc_mode_t>((LEDC_CHANNEL_2 / 8)),
			.channel        = LEDC_CHANNEL_2,
			.intr_type      = LEDC_INTR_DISABLE,
			.timer_sel      = static_cast<ledc_timer_t>(((LEDC_CHANNEL_2 / 2) % 4)),
			.duty           = 0,
			.hpoint         = 0,
			.flags = { .output_invert = true }
	};
	ledc_channel_config(&ledc_channel);

	ledc_timer = {
			.speed_mode       = static_cast<ledc_mode_t>((LEDC_CHANNEL_3 / 8)),
			.duty_resolution  = LEDC_TIMER_10_BIT,
			.timer_num        = static_cast<ledc_timer_t>(((LEDC_CHANNEL_3 / 2) % 4)),
			.freq_hz          = 5000,
			.clk_cfg          = LEDC_AUTO_CLK
	};
	ledc_timer_config(&ledc_timer);

	ledc_channel = {
		#ifdef CTRL_TYPE_MISSIONCTRL
			.gpio_num       = LED_PANIC_R,
		#elifdef CTRL_TYPE_BASIC
			.gpio_num		= LED_NAVIGATION,
		#endif
			.speed_mode     = static_cast<ledc_mode_t>((LEDC_CHANNEL_3 / 8)),
			.channel        = LEDC_CHANNEL_3,
			.intr_type      = LEDC_INTR_DISABLE,
			.timer_sel      = static_cast<ledc_timer_t>(((LEDC_CHANNEL_3 / 2) % 4)),
			.duty           = 0,
			.hpoint         = 0,
			.flags = { .output_invert = true }
	};
	ledc_channel_config(&ledc_channel);

#ifdef CTRL_TYPE_BASIC
	ledc_timer = {
			.speed_mode       = static_cast<ledc_mode_t>((LEDC_CHANNEL_4 / 8)),
			.duty_resolution  = LEDC_TIMER_10_BIT,
			.timer_num        = static_cast<ledc_timer_t>(((LEDC_CHANNEL_4 / 2) % 4)),
			.freq_hz          = 5000,
			.clk_cfg          = LEDC_AUTO_CLK
	};
	ledc_timer_config(&ledc_timer);

	ledc_channel = {
			.gpio_num		= LED_SOUNDLIGHT,
			.speed_mode     = static_cast<ledc_mode_t>((LEDC_CHANNEL_4 / 8)),
			.channel        = LEDC_CHANNEL_4,
			.intr_type      = LEDC_INTR_DISABLE,
			.timer_sel      = static_cast<ledc_timer_t>(((LEDC_CHANNEL_4 / 2) % 4)),
			.duty           = 0,
			.hpoint         = 0,
			.flags = { .output_invert = true }
	};
	ledc_channel_config(&ledc_channel);

	ledc_timer = {
			.speed_mode       = static_cast<ledc_mode_t>((LEDC_CHANNEL_5 / 8)),
			.duty_resolution  = LEDC_TIMER_10_BIT,
			.timer_num        = static_cast<ledc_timer_t>(((LEDC_CHANNEL_5 / 2) % 4)),
			.freq_hz          = 5000,
			.clk_cfg          = LEDC_AUTO_CLK
	};
	ledc_timer_config(&ledc_timer);

	ledc_channel = {
			.gpio_num		= LED_WARN,
			.speed_mode     = static_cast<ledc_mode_t>((LEDC_CHANNEL_5 / 8)),
			.channel        = LEDC_CHANNEL_5,
			.intr_type      = LEDC_INTR_DISABLE,
			.timer_sel      = static_cast<ledc_timer_t>(((LEDC_CHANNEL_5 / 2) % 4)),
			.duty           = 0,
			.hpoint         = 0,
			.flags = { .output_invert = true }
	};
	ledc_channel_config(&ledc_channel);
#endif

#ifdef CTRL_TYPE_MISSIONCTRL
	aw9523->pinMode(EXTLED_CAM_0, AW9523::LED);
	aw9523->pinMode(EXTLED_CAM_L1, AW9523::LED);
	aw9523->pinMode(EXTLED_CAM_L2, AW9523::LED);
	aw9523->pinMode(EXTLED_CAM_L3, AW9523::LED);
	aw9523->pinMode(EXTLED_CAM_L4, AW9523::LED);
	aw9523->pinMode(EXTLED_CAM_R1, AW9523::LED);
	aw9523->pinMode(EXTLED_CAM_R2, AW9523::LED);
	aw9523->pinMode(EXTLED_CAM_R3, AW9523::LED);
	aw9523->pinMode(EXTLED_CAM_R4, AW9523::LED);
	aw9523->pinMode(EXTLED_WARN, AW9523::LED);
	aw9523->pinMode(EXTLED_ARM, AW9523::LED);
	aw9523->pinMode(EXTLED_LIGHT, AW9523::LED);
	aw9523->pinMode(EXTLED_ARM_UP, AW9523::LED);
	aw9523->pinMode(EXTLED_ARM_DOWN, AW9523::LED);
	aw9523->pinMode(EXTLED_PINCH_OPEN, AW9523::LED);
	aw9523->pinMode(EXTLED_PINCH_CLOSE, AW9523::LED);
#endif

	for(;;){
	#ifdef CTRL_TYPE_MISSIONCTRL
		ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
	#elifdef CTRL_TYPE_BASIC
		ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1 << LEDC_TIMER_10_BIT);
	#endif
		ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

		ledc_set_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_1 / 8)), LEDC_CHANNEL_1, 0);
		ledc_update_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_1 / 8)), LEDC_CHANNEL_1);

		ledc_set_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_2 / 8)), LEDC_CHANNEL_2, 0);
		ledc_update_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_2 / 8)), LEDC_CHANNEL_2);

		ledc_set_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_3 / 8)), LEDC_CHANNEL_3, 0);
		ledc_update_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_3 / 8)), LEDC_CHANNEL_3);

	#ifdef CTRL_TYPE_BASIC
		ledc_set_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_4 / 8)), LEDC_CHANNEL_4, 0);
		ledc_update_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_4 / 8)), LEDC_CHANNEL_4);

		ledc_set_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_5 / 8)), LEDC_CHANNEL_5, 0);
		ledc_update_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_5 / 8)), LEDC_CHANNEL_5);
	#endif

	#ifdef CTRL_TYPE_MISSIONCTRL
		constexpr static const uint8_t LEDs[] = {EXTLED_CAM_0, EXTLED_CAM_L1, EXTLED_CAM_L2, EXTLED_CAM_L3, EXTLED_CAM_L4, EXTLED_CAM_R1, EXTLED_CAM_R2, EXTLED_CAM_R3, EXTLED_CAM_R4,
						  EXTLED_WARN, EXTLED_ARM, EXTLED_LIGHT, EXTLED_ARM_UP, EXTLED_ARM_DOWN, EXTLED_PINCH_OPEN, EXTLED_PINCH_CLOSE};

		for(const int LED : LEDs){
			aw9523->dim(LED, 10);
		}
	#endif

		vTaskDelay(500);

	#ifdef CTRL_TYPE_MISSIONCTRL
		ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1 << LEDC_TIMER_10_BIT);
	#elifdef CTRL_TYPE_BASIC
		ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
	#endif
		ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

		ledc_set_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_1 / 8)), LEDC_CHANNEL_1, 1 << LEDC_TIMER_10_BIT);
		ledc_update_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_1 / 8)), LEDC_CHANNEL_1);

		ledc_set_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_2 / 8)), LEDC_CHANNEL_2, 1 << LEDC_TIMER_10_BIT);
		ledc_update_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_2 / 8)), LEDC_CHANNEL_2);

		ledc_set_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_3 / 8)), LEDC_CHANNEL_3, 1 << LEDC_TIMER_10_BIT);
		ledc_update_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_3 / 8)), LEDC_CHANNEL_3);

	#ifdef CTRL_TYPE_BASIC
		ledc_set_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_4 / 8)), LEDC_CHANNEL_4, 1 << LEDC_TIMER_10_BIT);
		ledc_update_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_4 / 8)), LEDC_CHANNEL_4);

		ledc_set_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_5 / 8)), LEDC_CHANNEL_5, 1 << LEDC_TIMER_10_BIT);
		ledc_update_duty(static_cast<ledc_mode_t>((LEDC_CHANNEL_5 / 8)), LEDC_CHANNEL_5);
	#endif

	#ifdef CTRL_TYPE_MISSIONCTRL
		for(const int LED : LEDs){
			aw9523->dim(LED, 0);
		}
	#endif

		vTaskDelay(500);
	}
}
