#pragma once 
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cassert>
#include "entity.h"


namespace middle {
	struct Component;

	struct Serializable {
		virtual void serialize(std::ostream& istream) = 0;
		virtual void deserialize(const std::vector<std::string>& buffer) = 0;
	};


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
	inline std::unordered_map <int, std::vector<ComponentId>> componentIdMap;
	inline std::unordered_map <int, std::vector<Serializable*>> componentSerializableRefMap;

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
		int typeId = componentTypeMap[componentName];
		std::vector<ComponentId> ids = componentIdMap[typeId];
		for (int i = 0; i < ids.size(); ++i) {
			if (ids[i].freeIndex)
				return i;
		}
		assert(true);
	}

	template<typename T>
	inline T* getComponent(Shape& shape) {
		int typeId = getTypeId<T>();
		if (shape.componentMap.find(typeId) == shape.componentMap.end()) {
			return nullptr;
		}
		int componentId = shape.componentMap[typeId].componentId;
		auto& v = getComponentArray<T>();
		return &v[componentId];
	}

	template<typename T>
	inline T* getComponentAssert(const Shape& shape) {
		int typeId = getTypeId<T>();
		if (shape.shapeComponentMap.find(typeId) == shape.shapeComponentMap.end()) {
			assert(true);
		}
		int componentId = shape.shapeComponentMap[typeId].componentId;
		auto& v = getComponentArray<T>();
		return &v[componentId];
	}

	inline Serializable* getSerializableComponent(Shape& shape, int typeId) {
		if (shape.componentMap.find(typeId) == shape.componentMap.end()) {
			assert(true);
		}
		auto component = shape.componentMap[typeId];
		int componentId = component.componentId;
		// check generation
		if (componentIdMap[typeId][componentId].freeIndex) {
			assert(true);
		}
		Serializable* result = componentSerializableRefMap[typeId][componentId];
		assert(result != nullptr);
		return result;
	}
}