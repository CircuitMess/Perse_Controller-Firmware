#ifndef PERSE_CTRL_DRIVESTATE_H
#define PERSE_CTRL_DRIVESTATE_H

#include "Services/StateMachine.h"

class DriveState : public State{
public:
	explicit DriveState();
	~DriveState() override;

protected:
	void loop() override;

private:

};


#endif //PERSE_CTRL_DRIVESTATE_H
