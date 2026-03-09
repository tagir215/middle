#pragma once
#include "middle_component_table.h"

namespace components {

	enum Desirability {
		INTERESTED,
		NOTINTERESTED
	};

	struct CacheCompType {
		Desirability desirability;
		int typeId;
	};

	class CompCache {
	public:
		std::vector<CacheCompType>typeIdVector;
		std::vector<middle::IComponentVectorContainer*>containerVector;
		std::vector<std::vector<int>>compOffsetsVector;
		std::vector<middle::Id>relevantIdVector;
		int componentTypeCount = -1;
		bool needsUpdate = true;

		int getSize() const {
			return relevantIdVector.size();
		}

		template<typename T>
		void addType(Desirability desirability = Desirability::INTERESTED) {
			int typeId = middle::getTypeId<T>();
			typeIdVector.push_back({desirability, typeId});
			componentTypeCount = typeIdVector.size();
			if (desirability == Desirability::INTERESTED) {
				containerVector.push_back(middle::componentListMap[typeId].get());
			}
			else {
				containerVector.push_back({});
			}
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
					auto type = cache->typeIdVector[i];
					if (typeId == cache->typeIdVector[i].typeId) {
						cacheTypeIndex = i;
					}
				}
				assert(cacheTypeIndex != -1);
				container = static_cast<middle::ComponentVectorContainer<T>*>(cache->containerVector[cacheTypeIndex]);
				if (cache->getSize() > 0) {
					compOffsets = &cache->compOffsetsVector[cacheTypeIndex];
				}
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
