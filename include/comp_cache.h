#pragma once
#include "game_state.h"
#include "middle_component_table.h"
#include "middle_shape_utils.h"

namespace components {

	class CompCache {
	public:
		std::vector<int>typeIdVector;
		std::vector<middle::IComponentVectorContainer*>containerVector;
		std::vector<std::vector<int>>compOffsetsVector;
		// key: id entity index  value: id cache index;  used to check if need to update
		std::unordered_map<int, int>relevantIdMap;
		int componentTypeCount = -1;
		int idCount;

		void updateCache(middle::GameState* gameState);

		int getSize() const {
			return idCount;
		}

		template<typename T>
		void addType() {
			int typeId = middle::getTypeId<T>();
			typeIdVector.push_back(typeId);
			containerVector.push_back(middle::componentListMap[typeId].get());
			componentTypeCount = typeIdVector.size();
		}


		template<typename T>
		struct Iterator {
			int cacheIdIndex = 0;
			middle::ComponentVectorContainer<T>* container = nullptr;
			std::vector<int>* compOffsets = nullptr;

			Iterator(CompCache* cache) {
				int typeId = middle::getTypeId<T>();
				int cacheTypeIndex = -1;
				for (int i = 0; i < cache->componentTypeCount; ++i) {
					if (typeId == cache->typeIdVector[i]) {
						cacheTypeIndex = i;
					}
				}
				assert(cacheTypeIndex != -1);
				container = static_cast<middle::ComponentVectorContainer<T>*>(cache->containerVector[cacheTypeIndex]);
				compOffsets = &cache->compOffsetsVector[cacheTypeIndex];
			}

			T* operator *() {
				int compOffset = (*compOffsets)[cacheIdIndex];
				++cacheIdIndex;
				return &container->vectorData[compOffset];
			}
		};

		template<typename T>
		Iterator<T> begin() {
			return Iterator<T>(this);
		}
	};

}
