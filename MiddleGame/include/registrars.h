#pragma once
#include <string>
#include "middle_component_table.h"
#include <ostream>

namespace middle {
	template<typename T>
	struct ComponentRegistrar {
		ComponentRegistrar(const std::string& componentName) {
			registerToComponentTypes<T>(componentName);
		}
	};
}