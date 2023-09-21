#ifndef CHATTER_FIRMWARE_LVMODAL_H
#define CHATTER_FIRMWARE_LVMODAL_H

#include "LVObject.h"

class LVScreen;

class LVModal : public LVObject {
public:
	LVModal(LVScreen* parent);
	virtual ~LVModal();

protected:
	lv_group_t* inputGroup;

private:
	static LVModal* current;

	const LVScreen* parentScreen;
	lv_obj_t* container;

};


#endif //CHATTER_FIRMWARE_LVMODAL_H
