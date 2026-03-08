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

	struct IComponentVectorContainer {
		virtual ~IComponentVectorContainer() = default;
		virtual int grow() = 0;
		virtual void shrink(int componentOffset) = 0;
		virtual Serializable* getSerializable(int componentOffset) = 0;
	};
	template<typename T> 
	struct ComponentVectorContainer : public IComponentVectorContainer {
		std::vector<T> vectorData;
		std::vector<int>freeList;
		int grow() override {
			if (freeList.size() > 0) {
				int nextFreeIndex = freeList.back();
				freeList.pop_back();
				return nextFreeIndex;
			}

			int nextFreeIndex = vectorData.size();
			vectorData.resize(nextFreeIndex+1);
			return nextFreeIndex;
		}

		void shrink(int componentOffset) {
			freeList.push_back(componentOffset);
			vectorData[componentOffset] = T();
		}

		Serializable* getSerializable(int componentOffset) {
			return static_cast<Serializable*>(&vectorData[componentOffset]);
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

	template<typename T>
	inline ComponentVectorContainer<T>* getComponentVectorContainer() {
		int typeId = getTypeId<T>();
		IComponentVectorContainer* iContainer = componentListMap[typeId].get();
		ComponentVectorContainer<T>* vectorContainer = static_cast<ComponentVectorContainer<T>*>(iContainer);
		return vectorContainer;
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
	}

	template<typename T>
	inline T* getComponent(Shape& shape) {
		int typeId = getTypeId<T>();
		if (shape.componentMap.find(typeId) == shape.componentMap.end()) {
			return nullptr;
		}
		int componentId = shape.componentMap[typeId].componentOffset;
		ComponentVectorContainer<T>* vectorContainer = getComponentVectorContainer<T>();
		T& t = vectorContainer->vectorData[componentId];
		return &t;
	}


	template<typename T>
	inline T* addComponent(Shape& shape) {
		int typeId = getTypeId<T>();
		ComponentVectorContainer<T>* vectorContainer = getComponentVectorContainer<T>();
		auto& data = vectorContainer->vectorData;
		int nextIndex = vectorContainer->grow();
		T t;
		data[nextIndex] = t;
		shape.componentMap[typeId] = Component();
		shape.componentMap[typeId].componentOffset = nextIndex;
		return &data[nextIndex];
	}

	template<typename T>
	inline void deleteComponent(Shape& shape) {
		int typeId = getTypeId<T>();
		ComponentVectorContainer<T>* vectorContainer = getComponentVectorContainer<T>();
		int componentOffset = shape.componentMap[typeId].componentOffset;
		vectorContainer->shrink(componentOffset);
		shape.componentMap.erase(typeId);
	}

}