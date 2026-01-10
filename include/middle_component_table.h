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


	inline int globalTypeCounter = 0;
	template<typename T>
	inline int getTypeId() {
		static int id = globalTypeCounter++;
		return id;
	}

	extern std::unordered_map <std::string, int> componentTypeMap;
	extern std::unordered_map <int, std::any> componentListMap;
	extern std::unordered_map <int, std::vector<Serializable*>> componentSerializableRefMap;

	template<typename T>
	inline std::vector<T>* getComponentArray() {
		int typeId = getTypeId<T>();
		auto& vec = std::any_cast<std::vector<T>&>(componentListMap[typeId]);
		return &vec;
	}

	template<typename T>
	inline void registerToComponentTypes(const std::string& componentName) {
		int typeId = getTypeId<T>();
		componentTypeMap[componentName] = typeId;
		componentListMap[typeId] = std::vector<T>();
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
	inline T* addComponent(Shape& shape) {
		int typeId = getTypeId<T>();
		std::vector<T>* data = getComponentArray<T>();
		int nextIndex = data->size();
		T t;
		data->push_back(t);
		shape.componentMap[typeId] = Component();
		shape.componentMap[typeId].componentOffset = nextIndex;

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