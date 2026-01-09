#include "Position.h"

namespace components {
	void Position::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(posX);
		ostream << middle::fieldToString(posY);
		ostream << middle::fieldToString(posZ);
	}

	void Position::deserialize(const std::vector<std::string>& buffer) {
		middle::fillField(&posX, buffer[0]);
		middle::fillField(&posY, buffer[1]);
		middle::fillField(&posZ, buffer[2]);
	}

	static middle::ComponentRegistrar<Position>reg("Position");
}
