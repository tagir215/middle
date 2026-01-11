#include "LoopTag.h"

namespace components {
	void LoopTag::serialize(std::ostream& ostream) {
	}

	void LoopTag::deserialize(const std::vector<std::string>& buffer) {
	}

	static middle::ComponentRegistrar<LoopTag>reg("LoopTag");
}
