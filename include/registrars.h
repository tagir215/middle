#pragma once
#include <string>
#include "middle_gameplay_script_map.h"
#include "middle_component_table.h"
#include <functional>
#include <memory>
#include <fstream>

namespace middle {
	template<typename T>
	struct SystemRegistrar {
		SystemRegistrar(std::string scriptName) {
			systemMap[scriptName] = std::make_unique<T>();
		}
	};

	template<typename T>
	struct ComponentRegistrar {
		ComponentRegistrar(const std::string& componentName) {
			registerToComponentTypes<T>(componentName);
		}
	};
}