#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"

class CompCacheSystem : public middle::MiddleGameplaySystem {

public:

	void init(middle::GameState* gameState) override {
		systemUpdateType = middle::SystemUpdateType::INITFRAME;
		systemModeType = middle::SystemModeType::ENGINE;
		updatePriority = 1;
	}

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
		for (components::CacheCompType& type : cache->typeIdVector) {
			if (changesMap.find(type.typeId) != changesMap.end()) {
				auto& ids = changesMap[type.typeId];
				for (middle::Id id : ids) {
					if (std::find(result.begin(), result.end(), id) != result.end()) {
						continue;
					}
					result.push_back(id);
				}
			}
		}
		return result;
	}

	void updateCache(middle::GameState* gameState, components::CompCache* cache) {

		if (cache->compOffsetsVector.size() == 0) {
			cache->compOffsetsVector.resize(cache->componentTypeCount);
		}

		int systemNameIndex = getSystemNameIndex(gameState, cache->systemName);

		auto candidates = getRelevantCandidates(gameState, cache);
		for (middle::Id id : candidates) {


			bool includeInCache = true;

			bool isValid = middle::isValidId(gameState, id);
			if (!isValid) {
				includeInCache = false;
			}

			if (isValid) {
				auto& shape = middle::getShape(gameState, id.index);
				// skip if not all components found, or if not interseted skip if found
				for (int compTypeIndex = 0; compTypeIndex < cache->componentTypeCount; ++compTypeIndex) {
					components::CacheCompType cacheCompType = cache->typeIdVector[compTypeIndex];
					int typeId = cacheCompType.typeId;
					if (cacheCompType.desirability == components::INTERESTED) {
						if (shape.componentMap.find(typeId) == shape.componentMap.end()) {
							includeInCache = false;
						}
					}
					if (cacheCompType.desirability == components::NOTINTERESTED) {
						if (shape.componentMap.find(typeId) != shape.componentMap.end()) {
							includeInCache = false;
						}
					}
				}
			}
			if (includeInCache) {
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
						middle::Component& comp = shape.componentMap[cacheCompType.typeId];
						cache->compOffsetsVector[compTypeIndex].push_back(comp.componentOffset);
					}
				}
			}
			else {
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
				}

			}
		}
		cache->needsUpdate = false;
	}

	void update(middle::GameState* gameState) override {

		if (!gameState->loaded) {
			return;
		}

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
	}
};

static middle::SystemRegistrar<CompCacheSystem> reg("CompCacheSystem");
