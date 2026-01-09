#pragma once 
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cassert>
struct Component;

namespace middle {

	struct Concept {
		virtual	~Concept() = default;
	};

	struct ComponentId {
		int generation = -1;
		bool freeIndex = true;
	};

	template<typename T>
	struct Model : Concept{
		std::vector<T>data;
	};


	inline int globalTypeCounter = 0;
	template<typename T>
	inline int getTypeId() {
		static int id = globalTypeCounter++;
		return id;
	}

	inline std::unordered_map <std::string, int> componentTypeMap;
	inline std::unordered_map <int, std::unique_ptr<Concept>> componentArrayMap;
	inline std::unordered_map <std::string, std::vector<ComponentId>> componentIdMap;

	template<typename T>
	inline std::vector<T>getComponentArray() {
		int typeId = getTypeId<T>();
		if (componentArrayMap.find(typeId) == componentArrayMap.end()) {
			return std::vector<T>();
		}
		Model<T>* model = static_cast<Model<T>*>(componentArrayMap[typeId].get());
		return model->data;
	}

	template<typename T>
	inline void addToComponentArray(const std::string& componentName) {
		int typeId = componentTypeMap[componentName] = getTypeId<T>();
		Model<T> model;
		componentArrayMap[typeId] = std::make_unique <Concept>(model);
	}

	inline void reserveComponentType(const std::string& componentName) {
		componentTypeMap[componentName] = globalTypeCounter;
	}

	inline int freeComponentId(const std::string& componentName) {
		std::vector<ComponentId> ids = componentIdMap[componentName];
		for (int i = 0; i < ids.size(); ++i) {
			if (ids[i].freeIndex)
				return i;
		}
		assert(true);
	}
}