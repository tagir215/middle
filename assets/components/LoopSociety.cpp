#include "LoopSociety.h"
#include "middle_constants.h"

namespace components {
	void LoopSociety::serialize(std::ostream& ostream) {
		if (parentLoopIndex >= GHOST_INDEX_OFFSET)
			parentLoopIndex = UNASSIGNED;
		for (int index : loopMemberIndexes) {
			if (index >= GHOST_INDEX_OFFSET) {
				loopMemberIndexes.clear();
			}
		}
		ostream << middle::fieldToString(parentLoopIndex);
		ostream << middle::fieldToString(loopMemberIndexes);
	}

	void LoopSociety::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::fillField(&parentLoopIndex, buffer[0]);
		middle::fillField(&loopMemberIndexes, buffer[1]);
		if (parentLoopIndex != UNASSIGNED) {
			parentLoopIndex += indexOffset;
		}
		for (int& memberIndex : loopMemberIndexes) {
			memberIndex += indexOffset;
		}
	}

	static middle::ComponentRegistrar<LoopSociety>reg("LoopSociety");
}
