#include "Color.h"

namespace components {
	void Color::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(colorR);
		ostream << middle::fieldToString(colorG);
		ostream << middle::fieldToString(colorB);
		ostream << middle::fieldToString(colorA);
	}

	void Color::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::fillField(&colorR, buffer[0]);
		middle::fillField(&colorG, buffer[1]);
		middle::fillField(&colorB, buffer[2]);
		middle::fillField(&colorA, buffer[3]);
	}

	static middle::ComponentRegistrar<Color>reg("Color");
}
