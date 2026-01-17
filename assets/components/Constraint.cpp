#include "Constraint.h"

namespace components {
	void Constraint::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(idA);
		ostream << middle::fieldToString(idB);
		ostream << middle::fieldToString(stiffness);
		ostream << middle::fieldToString(biasFactor);
		ostream << middle::fieldToString(targetDistance);
	}

	void Constraint::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::fillField(&idA, buffer[0], indexOffset);
		middle::fillField(&idB, buffer[1], indexOffset);
		middle::fillField(&stiffness, buffer[2]);
		middle::fillField(&biasFactor, buffer[3]);
		middle::fillField(&targetDistance, buffer[4]);
	}

	static middle::ComponentRegistrar<Constraint>reg("Constraint");
}
