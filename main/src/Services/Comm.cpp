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

void Comm::sendPacket(const ControlPacket& packet){
	if(!tcp.isConnected()) return;

	tcp.write((uint8_t*) &packet, sizeof(ControlPacket));
}

void Comm::loop(){
	if(tcp.isConnected()){
		ControlPacket packet{};
		tcp.read(reinterpret_cast<uint8_t*>(&packet), sizeof(ControlPacket));

		if(tcp.isConnected()){
			Events::post(Facility::Comm, packet);
		}
	}

	if(!tcp.isConnected()){
		Event event{};
		while(!queue.get(event, portMAX_DELAY));
		//Only a TCP connected event will unblock the thread
		free(event.data);
	}
}

void Comm::sendDriveDir(uint8_t direction, uint8_t speed){
	uint8_t code = direction & 0b1111;
	code |= (speed << 4);
	ControlPacket packet{ ComType::DriveDir, code };
	sendPacket(packet);
}

