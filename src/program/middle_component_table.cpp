#include "middle_component_table.h"

namespace middle {
	std::unordered_map <std::string, int> componentTypeMap;
	std::unordered_map <int, std::string> componentNameMap;
	std::unordered_map <int, std::unique_ptr<IComponentVectorContainer>> componentListMap;
	std::unordered_map <int, std::vector<Serializable*>> componentSerializableRefMap;

	Serializable* getSerializableComponent(Shape& shape, int typeId) {
		if (shape.componentMap.find(typeId) == shape.componentMap.end()) {
			assert(true);
		}
		auto component = shape.componentMap[typeId];
		Serializable* result = componentSerializableRefMap[typeId][component.componentOffset];
		assert(result != nullptr);
		return result;
	}
}

