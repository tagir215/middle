#include "ComponentRefParent.h"

namespace components {
	void ComponentRefParent::serialize(std::ostream& ostream) {

	}

	void ComponentRefParent::deserialize(const std::vector<std::string>& buffer, int indexOffset) {

	}

	static middle::ComponentRegistrar<ComponentRefParent>reg("ComponentRefParent");
}
