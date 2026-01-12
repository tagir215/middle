#include "Constraint.h"

namespace components {
	void Constraint::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(indexA);
		ostream << middle::fieldToString(indexB);
		ostream << middle::fieldToString(stiffness);
		ostream << middle::fieldToString(biasFactor);
		ostream << middle::fieldToString(targetDistance);
	}

	void Constraint::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::fillField(&indexA, buffer[0]);
		middle::fillField(&indexB, buffer[1]);
		middle::fillField(&stiffness, buffer[2]);
		middle::fillField(&biasFactor, buffer[3]);
		middle::fillField(&targetDistance, buffer[4]);
		indexA += indexOffset;
		indexB += indexOffset;
	}

	static middle::ComponentRegistrar<Constraint>reg("Constraint");
}
