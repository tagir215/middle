#pragma once 
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cassert>
#include "entity.h"
#include <any>


namespace middle {
	struct Component;

	struct Serializable {
		virtual void serialize(std::ostream& istream) = 0;
		virtual void deserialize(const std::vector<std::string>& buffer) = 0;
	};
	struct IComponentVectorContainer {
		virtual int grow() = 0;
	};
	template<typename T> 
	struct ComponentVectorContainer : public IComponentVectorContainer {
		std::vector<T> vectorData;
		int grow() override {
			int size = vectorData.size();
			vectorData.resize(size+1);
			updateSerializableMap<T>();
			return size + 1;
		}
	};

	inline int globalTypeCounter = 0;
	template<typename T>
	inline int getTypeId() {
		static int id = globalTypeCounter++;
		return id;
	}

	extern std::unordered_map <std::string, int> componentTypeMap;
	extern std::unordered_map <int, std::string> componentNameMap;
	extern std::unordered_map <int, std::unique_ptr<IComponentVectorContainer>> componentListMap;
	extern std::unordered_map <int, std::vector<Serializable*>> componentSerializableRefMap;

	template<typename T>
	inline std::vector<T>* getComponentArray() {
		int typeId = getTypeId<T>();
		IComponentVectorContainer* iContainer = componentListMap[typeId].get();
		ComponentVectorContainer<T>* vectorContainer = static_cast<ComponentVectorContainer<T>*>(iContainer);
		return &vectorContainer->vectorData;
	}

	template<typename T>
	inline void registerToComponentTypes(const std::string& componentName) {
		int typeId = getTypeId<T>();
		componentTypeMap[componentName] = typeId;
		componentNameMap[typeId] = componentName;
		// vector container
		auto vectorContainer = std::make_unique<ComponentVectorContainer<T>>();
		vectorContainer->vectorData = std::vector<T>();
		componentListMap[typeId] = std::move(vectorContainer);
		// serializable
		componentSerializableRefMap[typeId] = std::vector<Serializable*>();
	}

	template<typename T>
	inline T* getComponent(Shape& shape) {
		int typeId = getTypeId<T>();
		if (shape.componentMap.find(typeId) == shape.componentMap.end()) {
			return nullptr;
		}
		int componentId = shape.componentMap[typeId].componentOffset;
		std::vector<T>* v = getComponentArray<T>();
		T& t = (*v)[componentId];
		return &t;
	}

	template<typename T>
	inline void updateSerializableMap() {
		// add to serializable list
		int typeId = getTypeId<T>();
		std::vector<Serializable*>& serVec = componentSerializableRefMap[typeId];
		std::vector<T>* data = getComponentArray<T>();
		serVec.resize(data->size());
		for (int i = 0; i < serVec.size(); ++i) {
			serVec[i] = static_cast<Serializable*>(&(*data)[i]);
		}
	}

	template<typename T>
	inline T* addComponent(Shape& shape) {
		int typeId = getTypeId<T>();
		std::vector<T>* data = getComponentArray<T>();
		int nextIndex = data->size();
		T t;
		data->push_back(t);
		shape.componentMap[typeId] = Component();
		shape.componentMap[typeId].componentOffset = nextIndex;
		updateSerializableMap<T>();
		return &(*data)[nextIndex];
	}

	template<typename T>
	inline T* getComponentAssert(Shape& shape) {
		int typeId = getTypeId<T>();
		if (shape.componentMap.find(typeId) == shape.componentMap.end()) {
			assert(true, "component doesn't exist");
		}
		int componentId = shape.componentMap[typeId].componentOffset;
		std::vector<T>* v = getComponentArray<T>();
		T& t = (*v)[componentId];
		return &t;
	}

	Serializable* getSerializableComponent(Shape& shape, int typeId);
}