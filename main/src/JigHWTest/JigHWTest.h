#ifndef CLOCKSTAR_FIRMWARE_JIGHWTEST_H
#define CLOCKSTAR_FIRMWARE_JIGHWTEST_H

#include <vector>
#include "Util/stdafx.h"
#include "Devices/Battery.h"
#include <esp_efuse.h>
#include <esp_spiffs.h>
#include <Pins.hpp>
#include <string>

#ifdef CTRL_TYPE_MISSIONCTRL
#include "Devices/Display.h"
#include "Periph/I2C.h"
#include "Devices/AW9523.h"
#endif

struct Test {
	bool (* test)();
	const char* name;
	void (* onFail)();
};

class JigHWTest {
public:
	JigHWTest();
	static bool checkJig();
	void start();

private:
#ifdef CTRL_TYPE_MISSIONCTRL
	static Display* display;
	static LGFX_Device* canvas;
	static I2C* i2c;
	static AW9523* aw9523;
#endif
	static JigHWTest* test;
	std::vector<Test> tests;
	const char* currentTest;

	static adc_oneshot_unit_handle_t hndl;

	void log(const char* property, const char* value);
	void log(const char* property, float value);
	void log(const char* property, double value);
	void log(const char* property, bool value);
	void log(const char* property, uint32_t value);
	void log(const char* property, int32_t value);
	void log(const char* property, const std::string& value);

	static bool BatteryCalib();
	static bool BatteryCheck();
	static bool HWVersion();

#ifdef CTRL_TYPE_BASIC
	static bool ButtonCheck();
#elifdef CTRL_TYPE_MISSIONCTRL
	static bool AW9523Check();
	static bool SPIFFSTest();
	void rgb();
	static uint32_t calcChecksum(FILE* file);
#endif

	static void AudioVisualTest();

#ifdef CTRL_TYPE_MISSIONCTRL
	static const int16_t referenceVoltage = 3590; //USB power is 4V, after schottky diode: 3620mv - 30mV backlight voltage drop
#elifdef CTRL_TYPE_BASIC
	static const int16_t referenceVoltage = 4050; // 50mV for backlight voltage drop compensation
#endif
	static int16_t voltOffset;

	static constexpr uint32_t CheckTimeout = 500;

	static constexpr esp_efuse_desc_t adc1_low = { EFUSE_BLK3, 0, 8 };
	static constexpr const esp_efuse_desc_t* efuse_adc1_low[] = { &adc1_low, nullptr };
	static constexpr esp_efuse_desc_t adc1_high = { EFUSE_BLK3, 8, 8 };
	static constexpr const esp_efuse_desc_t* efuse_adc1_high[] = { &adc1_high, nullptr };

	static constexpr esp_vfs_spiffs_conf_t spiffsConfig = {
			.base_path = "/spiffs",
			.partition_label = "storage",
			.max_files = 8,
			.format_if_mount_failed = false
	};
};

#endif //CLOCKSTAR_FIRMWARE_JIGHWTEST_H
