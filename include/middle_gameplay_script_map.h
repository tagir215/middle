#pragma once
#include <unordered_map>
#include "middle_gameplay_script.h"
#include "entity.h"

class MiddleGameplaySystem;

namespace middle {
	inline auto& getSystemMap() {
		static std::unordered_map <std::string, std::unique_ptr<MiddleGameplaySystem>> systemMap;
		return systemMap;
	}
}

