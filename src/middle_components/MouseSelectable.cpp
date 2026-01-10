#include "MouseSelectable.h"

namespace components {
	void MouseSelectable::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(selected);
	}

	void MouseSelectable::deserialize(const std::vector<std::string>& buffer) {
		middle::fillField(&selected, buffer[0]);
	}

	static middle::ComponentRegistrar<MouseSelectable>reg("MouseSelectable");
}
