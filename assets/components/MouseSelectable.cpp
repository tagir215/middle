#include "MouseSelectable.h"

namespace components {
	void MouseSelectable::serialize(std::ostream& ostream) {

	}

	void MouseSelectable::deserialize(const std::vector<std::string>& buffer, int indexOffset) {

	}

	static middle::ComponentRegistrar<MouseSelectable>reg("MouseSelectable");
}
