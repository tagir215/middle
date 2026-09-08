#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include <unordered_set>

class CompCacheSystem : public middle::MiddleGameplaySystem {

public:

	uint32_t stamp = 1;
	std::vector<uint32_t> memo;


	void init(middle::GameState* gameState) override {
		systemUpdateType = middle::SystemUpdateType::INITFRAME;
		systemModeType = middle::SystemModeType::ENGINE;
		updatePriority = 1;
		memo.resize(middle::MAX_SHAPE_COUNT);
	}

	float deletionTime = 0;
	float attachmentTime = 0;
	float componentCheckTime = 0;
	float relevantCheckTime = 0;

	int getSystemNameIndex(middle::GameState* gameState, const std::string& name) {
		for (int i = 0; i < gameState->systemNames.size(); ++i) {
			if (name == gameState->systemNames[i]) {
				return i;
			}
		}
		assert(false);
	}


	std::vector<middle::Id> getRelevantCandidates(middle::GameState* gameState, components::CompCache* cache) {
		auto& changesMap = gameState->structuralChangesMap;
		std::vector<middle::Id>result;

		for (const components::CacheCompType& type : cache->typeIdVector) {
			auto it = changesMap.find(type.typeId);

			if (it == changesMap.end()) {
				continue;
			}

			for (const middle::Id& id : it->second) {
				if (memo[id.index] != stamp) {
					result.push_back(id);
					memo[id.index] = stamp;
				}
			}
		}
		++stamp;
		return result;
	}

	void updateCache(middle::GameState* gameState, components::CompCache* cache) {

		if (cache->compOffsetsVector.size() == 0) {
			cache->compOffsetsVector.resize(cache->componentTypeCount);
		}

		int systemNameIndex = getSystemNameIndex(gameState, cache->systemName);

		auto s = std::chrono::high_resolution_clock::now();
		auto candidates = getRelevantCandidates(gameState, cache);
		auto e = std::chrono::high_resolution_clock::now();
		auto d = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
		relevantCheckTime += d.count();

		for (middle::Id id : candidates) {


			bool includeInCache = true;

			bool isValid = middle::isValidId(gameState, id);
			if (!isValid) {
				includeInCache = false;
			}

			if (isValid) {
				auto start = std::chrono::high_resolution_clock::now();
				auto& shape = middle::getShape(gameState, id.index);
				// skip if not all components found, or if not interseted skip if found
				for (int compTypeIndex = 0; compTypeIndex < cache->componentTypeCount; ++compTypeIndex) {
					components::CacheCompType cacheCompType = cache->typeIdVector[compTypeIndex];
					int typeId = cacheCompType.typeId;
					auto compInfo = middle::getCompInfo(shape, typeId);
					if (cacheCompType.desirability == components::INTERESTED) {
						if (compInfo == shape.components.end()) {
							includeInCache = false;
							break;
						}
					}
					if (cacheCompType.desirability == components::NOTINTERESTED) {
						if (compInfo != shape.components.end()) {
							includeInCache = false;
							break;
						}
					}
				}
				auto end = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				componentCheckTime += duration.count();
			}
			if (includeInCache) {
				auto start = std::chrono::high_resolution_clock::now();
				auto& ids = cache->relevantIdVector;
				if (std::find(ids.begin(), ids.end(), id) != ids.end()) {
					continue;
				}
				// add id
				ids.push_back(id);
				// debug thing
				auto& shape = middle::getShape(gameState, id.index);
				shape.affectingSystems.insert(systemNameIndex);

				// add the comp offsets and relevant ids
				for (int compTypeIndex = 0; compTypeIndex < cache->componentTypeCount; ++compTypeIndex) {
					auto cacheCompType = cache->typeIdVector[compTypeIndex];
					if (cacheCompType.desirability == components::INTERESTED) {
						auto compInfo = middle::getCompInfo(shape, cacheCompType.typeId);
						cache->compOffsetsVector[compTypeIndex].push_back(compInfo->componentOffset);
					}
				}
				auto end = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				attachmentTime += duration.count();
			}
			else {
				auto start = std::chrono::high_resolution_clock::now();
				auto& ids = cache->relevantIdVector;
				for (int i = 0; i < ids.size(); ++i) {
					if (ids[i] != id) {
						continue;
					}
					// erase id
					ids.erase(ids.begin() + i);
					// erase component
					for (int compTypeIndex = 0; compTypeIndex < cache->componentTypeCount; ++compTypeIndex) {
						auto cacheCompType = cache->typeIdVector[compTypeIndex];
						if (cacheCompType.desirability == components::INTERESTED) {
							auto& offsets = cache->compOffsetsVector[compTypeIndex];
							offsets.erase(offsets.begin() + i);
						}
					}
					break;
				}
				auto end = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				deletionTime += duration.count();
			}
		}
		cache->needsUpdate = false;
	}

	void update(middle::GameState* gameState) override {

		if (!gameState->loaded) {
			return;
		}
		for (auto& t : memo) {
			t = 0;
		}
		stamp = 1;

		auto& structuralChanges = gameState->structuralChangesMap;
		if (structuralChanges.size() > 0) {
			for (auto& cache : gameState->compCaches) {
				for (auto cacheTypeId : cache->typeIdVector) {
					if (structuralChanges.find(cacheTypeId.typeId) != structuralChanges.end()) {
						cache->needsUpdate = true;
					}
				}
			}
		}

		for (auto& cache : gameState->compCaches) {
			if (cache->needsUpdate) {
				updateCache(gameState, cache.get());
			}
		}

		if (structuralChanges.size() > 0) {
			structuralChanges.clear();
		}

		gameState->debugInfo.push_back("deletionTime: " + std::to_string(deletionTime));
		gameState->debugInfo.push_back("includeTime: " + std::to_string(attachmentTime));
		gameState->debugInfo.push_back("relevantCheckTime: " + std::to_string(relevantCheckTime));
		gameState->debugInfo.push_back("compoenntCheckTime: " + std::to_string(componentCheckTime));
		deletionTime = 0;
		attachmentTime = 0;
		relevantCheckTime = 0;
		componentCheckTime = 0;
	}
};

static middle::SystemRegistrar<CompCacheSystem> reg("CompCacheSystem");
