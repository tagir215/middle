#include "ComponentRefParent.h"
#include <vector>

namespace components {
	void ComponentRefParent::serialize(std::ostream& ostream) {
		std::vector<int> children;
	}

	void ComponentRefParent::deserialize(const std::vector<std::string>& buffer, int indexOffset) {

	}

	static middle::ComponentRegistrar<ComponentRefParent>reg("ComponentRefParent");
}
