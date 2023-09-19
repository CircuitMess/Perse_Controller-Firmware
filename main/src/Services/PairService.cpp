#include "PairService.h"
#include "Util/Services.h"

PairService::PairService() : wifi(*(WiFiSTA*) Services.get(Service::WiFi)),
							 tcp(*(TCPClient*) Services.get(Service::TCP)),
							 thread([this](){ loop(); }, "PairService", 4 * 1024), queue(10){

	wifi.connect();
	Events::listen(Facility::WiFiSTA, &queue);
	thread.start();
}

PairService::~PairService(){
	Events::unlisten(&queue);
	if(thread.running()){
		thread.stop();
	}
}

PairService::State PairService::getState() const{
	return state;
}

void PairService::loop(){
	Event event{};
	if(queue.get(event, portMAX_DELAY)){
		if(event.facility == Facility::WiFiSTA){
			auto data = (WiFiSTA::Event*) event.data;
			if(data->action == WiFiSTA::Event::Connect && data->connect.success){
				bool res = tcp.connect();
				if(res){
					state = State::Success;
				}else{
					state = State::Fail;
				}
				thread.stop();
			}else if(data->action == WiFiSTA::Event::Connect && !data->connect.success){
				state = State::Fail;
				thread.stop();
			}
		}
		free(event.data);
	}
}
