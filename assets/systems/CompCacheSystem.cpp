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
			// skip if not all components found
			for (int compTypeIndex = 0; compTypeIndex < cache->componentTypeCount; ++compTypeIndex) {
				int typeId = cache->typeIdVector[compTypeIndex];
				if (shape.componentMap.find(typeId) == shape.componentMap.end()) {
					return true;
				}
			}
			// add the comp offsets and relevant ids
			for (int compTypeIndex = 0; compTypeIndex < cache->componentTypeCount; ++compTypeIndex) {
				int typeId = cache->typeIdVector[compTypeIndex];
				middle::Component& comp = shape.componentMap[typeId];
				cache->compOffsetsVector[compTypeIndex].push_back(comp.componentOffset);
			}

			cache->relevantIdVector.push_back(shape.id);
			// update here cause sometimes shapes don't even exist at update point
			cache->needsUpdate = false;
			return true;
			});

	}


	void update(middle::GameState* gameState) override {

		if (gameState->mutatedIdMap.size() > 0) {
			for (auto& cache : gameState->compCaches) {
				for (middle::Id& id : cache->relevantIdVector) {
					if (gameState->mutatedIdMap.find(id.index) != gameState->mutatedIdMap.end()) {
						cache->needsUpdate = true;
						break;
					}
				}
			}
		}

		for (auto& cache : gameState->compCaches) {
			if (cache->needsUpdate) {
				updateCache(gameState, cache.get());
			}
		}
	}
};

static middle::SystemRegistrar<CompCacheSystem> reg("CompCacheSystem");
