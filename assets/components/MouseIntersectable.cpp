#include "MouseIntersectable.h"

namespace components {
	void MouseIntersectable::serialize(std::ostream& ostream) {

	}

	void MouseIntersectable::deserialize(const std::vector<std::string>& buffer, int indexOffset) {

	}

	static middle::ComponentRegistrar<MouseIntersectable>reg("MouseIntersectable");
}
