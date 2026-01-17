#include "MouseGrabbable.h"

namespace components {
	void MouseGrabbable::serialize(std::ostream& ostream) {

	}

	void MouseGrabbable::deserialize(const std::vector<std::string>& buffer, int indexOffset) {

	}

	static middle::ComponentRegistrar<MouseGrabbable>reg("MouseGrabbable");
}
