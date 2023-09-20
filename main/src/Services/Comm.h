#ifndef PERSE_MISSIONCTRL_COMM_H
#define PERSE_MISSIONCTRL_COMM_H

#include "TCPClient.h"
#include "Util/Threaded.h"
#include "Util/Events.h"
#include <CommData.h>

class Comm : private Threaded {
public:
	struct Event{
		CommType type;
		union{

		};
		uint8_t raw;
	};

	Comm();
	~Comm() override;

	/**
	 * @param angle
	 * @param speed
	 */
	void sendDriveDir(DriveDir dir);

private:
	TCPClient& tcp;
	void loop() override;

	void sendPacket(const ControlPacket& packet);
	Event processPacket(const ControlPacket& packet);

	EventQueue queue;
};


#endif //PERSE_MISSIONCTRL_COMM_H