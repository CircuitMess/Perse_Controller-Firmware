#ifndef PERSE_MISSIONCTRL_COMM_H
#define PERSE_MISSIONCTRL_COMM_H

#include "TCPClient.h"
#include "Util/Threaded.h"
#include "Util/Events.h"
#include <CommData.h>

class Comm : private Threaded {
public:
	struct Event{
		CommType type = CommType::None;
		union{
			struct {
				bool changedOnRover;
				union {
					ArmPos armPos;
					ArmPinch armPinch;
					HeadlightsMode headlights;
					CameraRotation cameraRotation;
				};
			};
			uint8_t batteryPercent;
			ModulePlugData modulePlug;
			ModuleData moduleData;
		};
		uint8_t raw = 0;
	};

	Comm();
	~Comm() override;

	/**
	 * @param angle
	 * @param speed
	 */
	void sendDriveDir(DriveDir dir);
	void sendHeadlights(HeadlightsMode headlights);
	void sendArmPos(ArmPos position);
	void sendArmPinch(ArmPinch pinch);
	void sendCameraRotation(CameraRotation rotation);
	void sendFeedQuality(uint8_t quality);
	void sendModulesEnable(bool enable);
	void sendScanningEnable(bool enable);
	void sendEmergencyMode(bool state);

private:
	TCPClient& tcp;
	void loop() override;

	void sendPacket(const ControlPacket& packet);
	Event processPacket(const ControlPacket& packet);

	EventQueue queue;
};

#endif //PERSE_MISSIONCTRL_COMM_H
