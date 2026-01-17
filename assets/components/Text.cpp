#include "Text.h"

namespace components {
	void Text::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(fontColorR);
		ostream << middle::fieldToString(fontColorG);
		ostream << middle::fieldToString(fontColorB);
		ostream << middle::fieldToString(fontColorA);
		ostream << middle::fieldToString(offsetX);
		ostream << middle::fieldToString(offsetY);
		ostream << middle::fieldToString(offsetZ);
		ostream << middle::fieldToString(fontSize);
		ostream << middle::fieldToString(visible);
	}

	void Text::deserialize(const std::vector<std::string>& buffer, int indexOffset) {

		middle::fillField(&fontColorR, buffer[0]);
		middle::fillField(&fontColorG, buffer[1]);
		middle::fillField(&fontColorB, buffer[2]);
		middle::fillField(&fontColorA, buffer[3]);
		middle::fillField(&offsetX, buffer[4]);
		middle::fillField(&offsetY, buffer[5]);
		middle::fillField(&offsetZ, buffer[6]);
		middle::fillField(&fontSize, buffer[7]);
		middle::fillField(&visible, buffer[8]);
	}

	static middle::ComponentRegistrar<Text>reg("Text");
}
