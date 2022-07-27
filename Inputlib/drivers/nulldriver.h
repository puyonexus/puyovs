#pragma once

#include "driver.h"
#include "inputlib.h"

namespace ilib {

class NullDriver : public Driver {
public:
	NullDriver();
	~NullDriver() override;

	bool error() const override;
	void process() override;

	void enableEvents() override;
	void disableEvents() override;
	bool getEvent(InputEvent* e) override;
};

}
