#include "Text.h"

namespace components {
	void Text::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(text);
		ostream << middle::fieldToString(offsetX);
		ostream << middle::fieldToString(offsetY);
		ostream << middle::fieldToString(offsetZ);
		ostream << middle::fieldToString(fontColorR);
		ostream << middle::fieldToString(fontColorG);
		ostream << middle::fieldToString(fontColorB);
		ostream << middle::fieldToString(fontColorA);
		ostream << middle::fieldToString(fontSize);
	}

	void Text::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::fillField(&text, buffer[0]);
		middle::fillField(&offsetX, buffer[1]);
		middle::fillField(&offsetY, buffer[2]);
		middle::fillField(&offsetZ, buffer[3]);
		middle::fillField(&fontColorR, buffer[4]);
		middle::fillField(&fontColorG, buffer[5]);
		middle::fillField(&fontColorB, buffer[6]);
		middle::fillField(&fontColorA, buffer[7]);
		middle::fillField(&fontSize, buffer[8]);
	}

	static middle::ComponentRegistrar<Text>reg("Text");
}
