#include "Reference.h"

namespace components {
	void Reference::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(sceneName);
	}

	void Reference::deserialize(const std::vector<std::string>& buffer) {
		middle::fillField(&sceneName, buffer[0]);
	}

	static middle::ComponentRegistrar<Reference>reg("Reference");
}
