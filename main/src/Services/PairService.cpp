#include "PairService.h"
#include "Util/Services.h"

PairService::PairService() : wifi(*(WiFiSTA*) Services.get(Service::WiFi)),
							 tcp(*(TCPClient*) Services.get(Service::TCP)),
							 thread([this](){ loop(); }, "PairService", 4 * 1024), queue(10){

	wifi.connect();
	Events::listen(Facility::WiFiSTA, &queue);

}

PairService::~PairService(){
	Events::unlisten(&queue);
	thread.stop();
}

PairService::State PairService::getState() const{
	return state;
}

void PairService::loop(){
	Event event{};
	while(!queue.get(event, portMAX_DELAY)){}

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
		state = res ? State::Success : State::Fail;
	}else{
		state = State::Fail;
	}

	thread.stop();
}
