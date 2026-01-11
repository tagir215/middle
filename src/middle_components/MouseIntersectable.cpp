#include "MouseIntersectable.h"

namespace components {
	void MouseIntersectable::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(intersecting);
		ostream << middle::fieldToString(wasIntersecting);
	}

	void MouseIntersectable::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::fillField(&intersecting, buffer[0]);
		middle::fillField(&wasIntersecting, buffer[1]);
	}

	static middle::ComponentRegistrar<MouseIntersectable>reg("MouseIntersectable");
}
