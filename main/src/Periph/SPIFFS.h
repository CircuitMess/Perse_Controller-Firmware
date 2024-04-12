#ifndef PERSE_MISSIONCTRL_SPIFFS_H
#define PERSE_MISSIONCTRL_SPIFFS_H

#include <cstdio>

class SPIFFS
{
public:
	SPIFFS();
	virtual ~SPIFFS();

private:
	static constexpr size_t MaxFileNumber = 18;
	static constexpr char* BasePath = "/spiffs";
	static constexpr char* PartitionLabel = "storage";
};

#endif //PERSE_MISSIONCTRL_SPIFFS_H
