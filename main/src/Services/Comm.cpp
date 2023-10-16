#include "Comm.h"
#include "Util/Services.h"

Comm::Comm() : Threaded("Comm", 4 * 1024), tcp(*(TCPClient*) Services.get(Service::TCP)), queue(10){
	Events::listen(Facility::TCP, &queue);
	start();
}

Comm::~Comm(){
	Events::unlisten(&queue);
	stop();
}

void Comm::sendDriveDir(DriveDir dir){
	uint8_t code = CommData::encodeDriveDir(dir);
	ControlPacket packet{ CommType::DriveDir, code };
	sendPacket(packet);
}

void Comm::sendHeadlights(HeadlightsMode headlights) {
	const ControlPacket packet {
		.type = CommType::Headlights,
		.data = (uint8_t)headlights
	};

	sendPacket(packet);
}

void Comm::sendArmPos(ArmPos position) {
	const ControlPacket packet {
			.type = CommType::Headlights,
			.data = (uint8_t)position
	};

	sendPacket(packet);
}

void Comm::sendArmPinch(ArmPinch pinch) {
	const ControlPacket packet {
			.type = CommType::Headlights,
			.data = (uint8_t)pinch
	};

	sendPacket(packet);
}

void Comm::sendPacket(const ControlPacket& packet){
	if(!tcp.isConnected()) return;

	tcp.write((uint8_t*) &packet, sizeof(ControlPacket));
}

void Comm::loop(){
	bool readOK = false;
	if(tcp.isConnected()){
		ControlPacket packet{};
		readOK = tcp.read(reinterpret_cast<uint8_t*>(&packet), sizeof(ControlPacket));

		if(readOK){
			Event e = processPacket(packet);
			Events::post(Facility::Comm, e);
		}
	}

	if(!tcp.isConnected() || !readOK){
		::Event event{};
		while(!queue.get(event, portMAX_DELAY));
		free(event.data);
	}
}

Comm::Event Comm::processPacket(const ControlPacket& packet)
{
	Event event {
		.type = packet.type,
		.raw = packet.data
	};

	switch (packet.type){
        case CommType::Headlights: {
			event.headlights = packet.data > 0 ? HeadlightsMode::On : HeadlightsMode::Off;
			break;
		}
		default: {
			break;
		}
	}

	return event;
}