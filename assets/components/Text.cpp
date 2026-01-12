#include "Text.h"

namespace components {
	void Text::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(text);
	}

	void Text::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::fillField(&text, buffer[0]);
	}

	static middle::ComponentRegistrar<Text>reg("Text");
}
