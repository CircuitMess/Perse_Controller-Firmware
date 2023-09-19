#ifndef PERSE_MISSIONCTRL_COMM_H
#define PERSE_MISSIONCTRL_COMM_H

#include "TCPClient.h"
#include "Util/Threaded.h"
#include "Util/Events.h"
#include <Comm.h>

class Comm : private Threaded {
public:
	Comm();
	~Comm() override;

	/**
	 * @param direction Bitwise OR-ed values of direction buttons, using only the lowest 4 bits.
	 * Lowest to highest bit represents forward, backward, left, right, respectively
	 * @param speed Lowest 4 bits indicate driving speed (0 - 15), used for rough speed estimates, otherwise sendDriveSpeed() is preferred.
	 * If speed is zero, then a default value is used.
	 */
	void sendDriveDir(uint8_t direction, uint8_t speed = 0);

private:
	TCPClient& tcp;
	void loop() override;

	void sendPacket(const ControlPacket& packet);

	EventQueue queue;
};


#endif //PERSE_MISSIONCTRL_COMM_H
