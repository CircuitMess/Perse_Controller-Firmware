#include "PairScreen.h"
#include "Devices/Input.h"
#include "Screens/DriveScreen.h"

PairScreen::PairScreen(Sprite& canvas) : Screen(canvas), evts(6){
	text1 = new LabelElement(this, "Hold PAIR to pair...");
	text2 = new LabelElement(this, "");

	const TextStyle style = {
			.color = TFT_GREEN,
			.datum = CC_DATUM
	};
	text1->setStyle(style);
	text2->setStyle(style);

	text1->setPos(64, 60);
	text2->setPos(64, 70);

	Events::listen(Facility::Input, &evts);
}

PairScreen::~PairScreen(){
	Events::unlisten(&evts);

	delete text1;
	delete text2;
}

void PairScreen::onLoop(){
	{
		Event evt{};
		if(evts.get(evt, 0)){
			if(evt.facility == Facility::Input && evt.data != nullptr){
				auto data = (Input::Data*) evt.data;

				if(data != nullptr){
					processInput(*data);
				}
			}

			free(evt.data);
		}
	}

	if(pair && pair->getState() != PairService::State::Pairing){
		volatile const auto state = pair->getState();
		pair.reset();

		if(state == PairService::State::Success){
			transition([](Sprite& canvas){ return std::make_unique<DriveScreen>(canvas); });
		}else if(state == PairService::State::Fail){
			if(text1 != nullptr){
				text1->setText("Pair failed.");
			}

			if(text2 != nullptr){
				text2->setText("Hold PAIR to pair...");
			}
		}

		return;
	}
}

void PairScreen::processInput(const Input::Data& evt){
	if(evt.btn != Input::Pair) return;

	if(evt.action == Input::Data::Press && !pair){
		pair = std::make_unique<PairService>();

		if(text1 != nullptr){
			text1->setText("Pairing.");
		}

		if(text2 != nullptr){
			text2->setText("Hold PAIR button.");
		}
	}else if(evt.action == Input::Data::Release && pair){
		pair.reset();

		if(text1 != nullptr){
			text1->setText("Pair aborted.");
		}

		if(text2 != nullptr){
			text2->setText("Hold PAIR to pair...");
		}
	}
}
