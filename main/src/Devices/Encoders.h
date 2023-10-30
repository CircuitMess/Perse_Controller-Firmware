#ifndef PERSE_MISSIONCTRL_ENCODERS_H
#define PERSE_MISSIONCTRL_ENCODERS_H


#include <optional>
#include "Util/Threaded.h"

class Encoders : private Threaded {
public:
	Encoders();
	virtual ~Encoders();

	enum Enc { Cam, Arm, Pinch, COUNT };
	static const std::unordered_map<Enc, const char*> Labels;

	struct Data {
		Enc enc;
		int8_t dir;
	};

private:
	struct EncPins { uint8_t pinA, pinB; };
	static const std::unordered_map<Enc, EncPins> PinMap;

	static constexpr uint64_t SleepTime = 2; // [ms]

	void loop() override;

	void scan();
	void scanOne(Enc enc);

	std::optional<bool> states[Enc::COUNT];

};


#endif //PERSE_MISSIONCTRL_ENCODERS_H
