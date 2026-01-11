#include "LoopSociety.h"

namespace components {
	void LoopSociety::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(parentLoopIndex);
		ostream << middle::fieldToString(loopMemberIndexes);
	}

	void LoopSociety::deserialize(const std::vector<std::string>& buffer) {
		middle::fillField(&parentLoopIndex, buffer[0]);
		middle::fillField(&loopMemberIndexes, buffer[1]);
	}

	static middle::ComponentRegistrar<LoopSociety>reg("LoopSociety");
}
