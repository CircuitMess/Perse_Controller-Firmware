#ifndef PERSE_MISSIONCTRL_HWVERSION_H
#define PERSE_MISSIONCTRL_HWVERSION_H

#include <cstdint>
#include <esp_efuse.h>

class HWVersion {
public:
	static bool check();
	static bool write();
	static void log();

	static bool readVersion(uint16_t &version);

	static uint16_t getHardcodedVersion();

private:
	static inline uint16_t CachedVersion = 0;

#ifdef CTRL_TYPE_MISSIONCTRL
	static inline constexpr const uint16_t Version = 0x0001;
#elifdef CTRL_TYPE_BASIC
	static inline constexpr const uint16_t Version = 0x0002;
#endif

	static constexpr esp_efuse_desc_t Ver = { EFUSE_BLK3, 16, 16 };
	static constexpr const esp_efuse_desc_t* Efuse_ver[] = { &Ver, nullptr };
};

#endif //PERSE_MISSIONCTRL_HWVERSION_H