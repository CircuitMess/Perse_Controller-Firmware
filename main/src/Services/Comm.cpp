#include "Comm.h"
#include "Util/Services.h"

Comm::Comm() : Threaded("Comm", 4 * 1024), tcp(*(TCPClient*) Services.get(Service{/*TODO - set actual service*/})), queue(10){
	Events::listen(Facility::TCP, &queue);
	start();
}

Comm::~Comm(){
	Events::unlisten(&queue);
	stop();
}

void Comm::sendPacket(const ControlPacket& packet){
	if(!tcp.isConnected()) return;

	tcp.write((uint8_t*) &packet, sizeof(ControlPacket));
}

void Comm::loop(){
	if(tcp.isConnected()){
		ControlPacket packet{};
		tcp.read(reinterpret_cast<uint8_t*>(&packet), sizeof(ControlPacket));

		if(tcp.isConnected()){
			Event e = processPacket(packet);
			Events::post(Facility::Comm, e);
		}
	}

	if(!tcp.isConnected()){
		::Event event{};
		while(!queue.get(event, portMAX_DELAY));
		//Only a TCP connected event will unblock the thread
		free(event.data);
	}
}

void Comm::sendDriveDir(DriveDir dir){
	uint8_t code = CommData::encodeDriveDir(dir);
	ControlPacket packet{ CommType::DriveDir, code };
	sendPacket(packet);
}

Comm::Event Comm::processPacket(const ControlPacket& packet){
	Event e{};
	e.raw = packet.data;

	return e;
}

