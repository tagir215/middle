#include "middle_component_table.h"

namespace middle {
	std::unordered_map <std::string, int> componentTypeMap;
	std::unordered_map <int, std::string> componentNameMap;
	std::unordered_map <int, std::unique_ptr<IComponentVectorContainer>> componentListMap;
	std::unordered_map <int, std::vector<Serializable*>> componentSerializableRefMap;

}

