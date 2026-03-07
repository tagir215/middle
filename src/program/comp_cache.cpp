#include "comp_cache.h"

namespace components {

	void CompCache::updateCache(middle::GameState* gameState) {
		relevantIdMap.clear();
		idCount = 0;
		compOffsetsVector.clear();
		compOffsetsVector.resize(componentTypeCount);

		// fill relevant ids and store comp offset for each component for each entity
		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			bool oneOrMoreComponents = false;
			for (int compTypeIndex = 0; compTypeIndex < this->componentTypeCount; ++compTypeIndex) {
				int typeId = this->typeIdVector[compTypeIndex];
				if (shape.componentMap.find(typeId) != shape.componentMap.end()) {
					middle::Component& comp = shape.componentMap[typeId];
					this->compOffsetsVector[compTypeIndex].push_back(comp.componentOffset);
					oneOrMoreComponents = true;
				}
			}
			if (oneOrMoreComponents) {
				this->relevantIdMap[shape.id.index] = this->idCount++;
			}
			return true;
			});
	}

}
