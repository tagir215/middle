#pragma once
#include "middle_gameplay_script_map.h"
#include <string>
#include <memory>

namespace middle {
	template<typename T>
	struct SystemRegistrar {
		SystemRegistrar(std::string scriptName) {
			getSystemMap()[scriptName] = std::make_unique<T>();
		}
	};
}
