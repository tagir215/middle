#include "/*componentName*/.h"

namespace components {
	void /*componentName*/::serialize(std::ostream& ostream) {

	}

	void /*componentName*/::deserialize(const std::vector<std::string>& buffer, int indexOffset) {

	}

	static middle::ComponentRegistrar</*componentName*/>reg("/*componentName*/");
}
