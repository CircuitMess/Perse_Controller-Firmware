#ifndef BIT_FIRMWARE_SETTINGS_H
#define BIT_FIRMWARE_SETTINGS_H

#include <nvs.h>

//TODO - change default joystick calibration values if the HW has predictable offsets
struct JoystickCalibration {
	uint32_t maxX = 4095, minX = 0;
	uint32_t maxY = 4095, minY = 0;
	uint32_t centerX = 4095 / 2, centerY = 4095 / 2;
};
struct SettingsStruct {
	bool sound = true;
	uint8_t screenBrightness = 80;
	uint8_t sleepTime = 3;
	JoystickCalibration joystickCalibration;
};

class Settings {
public:
	Settings();

	SettingsStruct get();
	void set(SettingsStruct& settings);
	void store();

	static constexpr uint8_t SleepSteps = 5;
	static constexpr const uint32_t SleepSeconds[SleepSteps] = { 0, 30, 60, 2 * 60, 5 * 60 };
	static constexpr const char* SleepText[SleepSteps] = { "OFF", "30 sec", "1 min", "2 min", "5 min" };

private:
	nvs_handle_t handle{};
	SettingsStruct settingsStruct;

	static constexpr const char* NVSNamespace = "Perseverance";
	static constexpr const char* BlobName = "Settings";

	void load();
};


#endif //BIT_FIRMWARE_SETTINGS_H
