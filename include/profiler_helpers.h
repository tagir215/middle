#pragma once
#include "game_state.h"
#include <stack>
#include <chrono>
#include <unordered_map>

namespace middleProfiling {

	inline float slowSystemThreshold = 0.2f;
	inline float slowActionThreshold = 0.2f;
	inline std::stack<std::chrono::time_point<std::chrono::steady_clock>>timeStamps;
	inline std::unordered_map<std::string, float>timeMap;

	inline void reviewSystemTime(middle::GameState* gameState, const middle::MiddleGameplaySystem* system) {
		float timeMs = system->updateTime.count();
		if (timeMs > slowSystemThreshold) {
			gameState->slowSystems.push_back(system->systemName + ": " + std::to_string(timeMs));
		}
	}

}

inline void mstart() {
	middleProfiling::timeStamps.push(std::chrono::high_resolution_clock::now());
}

inline void mendmicro(const std::string& label) {
	auto start = middleProfiling::timeStamps.top();
	middleProfiling::timeStamps.pop();
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
	auto it = middleProfiling::timeMap.find(label);
	if (it == middleProfiling::timeMap.end()) {
		middleProfiling::timeMap[label] = duration.count();
	}
	else {
		it->second += duration.count();
	}
}

inline void mflushmicro(middle::GameState* gameState) {
	for (auto& pair : middleProfiling::timeMap) {
		auto& label = pair.first;
		auto time = pair.second;
		gameState->debugInfo.push_back(label + ": " + std::to_string(time) + "\u00B5");
	}
	middleProfiling::timeMap.clear();
	assert(middleProfiling::timeStamps.size() == 0);
}
