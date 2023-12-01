#include "PairService.h"
#include "Util/Services.h"
#include "Util/stdafx.h"

PairService::PairService() : wifi(*(WiFiSTA*) Services.get(Service::WiFi)),
							 tcp(*(TCPClient*) Services.get(Service::TCP)),
							 thread([this](){ loop(); }, "PairService", 4 * 1024), queue(10){

	Events::listen(Facility::WiFiSTA, &queue);
	thread.start();
	wifi.connect();
}

PairService::~PairService(){
	thread.stop(0);

	if(state == State::Pairing){
		wifi.disconnect();
	}

	while(thread.running()){
		delayMillis(1);
	}

	Events::unlisten(&queue);
}

PairService::State PairService::getState() const{
	return state;
}

void PairService::loop(){
	Event event{};
	if(!queue.get(event, portMAX_DELAY)) return;

	if(event.data == nullptr){
		return;
	}

	if(event.facility == Facility::WiFiSTA){
		auto& data = *((WiFiSTA::Event*) event.data);
		processEvent(data);
	}

	free(event.data);
}

void PairService::processEvent(const WiFiSTA::Event& event){
	if(event.action != WiFiSTA::Event::Connect) return;

	if(event.connect.success){
		bool res = tcp.connect();
		if(!res){
			wifi.disconnect();
		}
		state = res ? State::Success : State::Fail;
	}else{
		state = State::Fail;
	}

	thread.stop(0);
}
