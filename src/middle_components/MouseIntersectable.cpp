#include "MouseIntersectable.h"

namespace components {
	void MouseIntersectable::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(intersecting);
	}

	void MouseIntersectable::deserialize(const std::vector<std::string>& buffer) {
		middle::fillField(&intersecting, buffer[0]);
	}

	static middle::ComponentRegistrar<MouseIntersectable>reg("MouseIntersectable");
}
