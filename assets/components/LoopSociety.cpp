#include "LoopSociety.h"

namespace components {
	void LoopSociety::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(parentLoopId);
		ostream << middle::fieldToString(loopMemberIds);
	}

	void LoopSociety::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::fillField(&parentLoopId, buffer[0], indexOffset);
		if (buffer.size() > 1) {
			middle::fillField(&loopMemberIds, buffer[1], indexOffset);
		}
	}

	static middle::ComponentRegistrar<LoopSociety>reg("LoopSociety");
}
