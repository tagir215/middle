#include "SystemReference.h"

namespace components {
	void SystemReference::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(systemName);
	}

	void SystemReference::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::fillField(&systemName, buffer[0]);
	}

	static middle::ComponentRegistrar<SystemReference>reg("SystemReference");
}
