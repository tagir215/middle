#pragma once
#include "middle_gameplay_script_map.h"
#include <string>
#include <memory>

namespace middle {
	template<typename T>
	struct SystemRegistrar {
		SystemRegistrar(std::string scriptName) {
			auto t = std::make_unique<T>();
			t->systemName = scriptName;
			getSystemMap()[scriptName] = std::move(t);
		}
	};
}
