#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"

class CompCacheSystem : public middle::MiddleGameplaySystem {

public:

	CompCacheSystem() {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void init(middle::GameState* gameState) override {


	}

	void updateCache(middle::GameState* gameState, components::CompCache* cache) {
		cache->relevantIdVector.clear();
		cache->compOffsetsVector.clear();
		cache->compOffsetsVector.resize(cache->componentTypeCount);

		// fill relevant ids and store comp offset for each component for each entity
		middle::loopInstances(gameState, [gameState, cache](int i, middle::Shape& shape) {
			// skip if not all components found, or if not interseted skip if found
			for (int compTypeIndex = 0; compTypeIndex < cache->componentTypeCount; ++compTypeIndex) {
				components::CacheCompType cacheCompType = cache->typeIdVector[compTypeIndex];
				int typeId = cacheCompType.typeId;
				if (cacheCompType.desirability == components::INTERESTED) {
					if (shape.componentMap.find(typeId) == shape.componentMap.end()) {
						return true;
					}
				}
				if (cacheCompType.desirability == components::NOTINTERESTED) {
					if (shape.componentMap.find(typeId) != shape.componentMap.end()) {
						return true;
					}
				}
			}
			// add the comp offsets and relevant ids
			for (int compTypeIndex = 0; compTypeIndex < cache->componentTypeCount; ++compTypeIndex) {
				auto cacheCompType = cache->typeIdVector[compTypeIndex];
				if (cacheCompType.desirability == components::INTERESTED) {
					middle::Component& comp = shape.componentMap[cacheCompType.typeId];
					cache->compOffsetsVector[compTypeIndex].push_back(comp.componentOffset);
				}
			}

			cache->relevantIdVector.push_back(shape.id);
			return true;
			});

		cache->needsUpdate = false;
	}


	void update(middle::GameState* gameState) override {

		if (!gameState->loaded) {
			return;
		}

		auto& structuralChanges = gameState->componentTypeIdSetWithStructuralChanges;
		if (structuralChanges.size() > 0) {
			for (auto& cache : gameState->compCaches) {
				for (auto cacheTypeId : cache->typeIdVector) {
					if (structuralChanges.find(cacheTypeId.typeId) != structuralChanges.end()) {
						cache->needsUpdate = true;
					}
				}
			}
			structuralChanges.clear();
		}

		for (auto& cache : gameState->compCaches) {
			if (cache->needsUpdate) {
				updateCache(gameState, cache.get());
			}
		}
	}
};

static middle::SystemRegistrar<CompCacheSystem> reg("CompCacheSystem");
