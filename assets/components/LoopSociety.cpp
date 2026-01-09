#include "LoopSociety.h"

namespace components {
	void LoopSociety::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(loopArrayOffset);
		ostream << middle::fieldToString(loopSize);
		ostream << middle::fieldToString(parentLoopIndex);
	}

	void LoopSociety::deserialize(const std::vector<std::string>& buffer) {
		middle::fillField(&loopArrayOffset, buffer[0]);
		middle::fillField(&loopSize, buffer[1]);
		middle::fillField(&parentLoopIndex, buffer[2]);
	}

	static middle::ComponentRegistrar<LoopSociety>reg("LoopSociety");
}
