#include "ComponentReference.h"

namespace components {
	void ComponentReference::serialize(std::ostream& ostream) {

	}

	void ComponentReference::deserialize(const std::vector<std::string>& buffer, int indexOffset) {

	}

	static middle::ComponentRegistrar<ComponentReference>reg("ComponentReference");
}
