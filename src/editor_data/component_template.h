#pragma once
#include "registrars.h"

namespace components {
	static std::string componentName = "/*componentName*/";

	struct /*componentName*/ {

	};

	inline middle::ComponentRegistrar</*componentName*/> reg(componentName);
}
