#include "MouseGrabbable.h"

namespace components {
	void MouseGrabbable::serialize(std::ostream& ostream) {
        ostream << middle::fieldToString(grabbing);
	}

	void MouseGrabbable::deserialize(const std::vector<std::string>& buffer) {
        middle::fillField(&grabbing, buffer[0]);
	}

	static middle::ComponentRegistrar<MouseGrabbable>reg("MouseGrabbable");
}
