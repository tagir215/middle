#include "NonPhysicalBubbleTag.h"

namespace components {
	void NonPhysicalBubbleTag::serialize(std::ostream& ostream) {
		middle::Serializer serializer{ ostream };
		reflect(serializer);
	}

	void NonPhysicalBubbleTag::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		reflect(deserializer);
	}

	void NonPhysicalBubbleTag::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		middle::FieldCollector collector{ fields, size };
		reflect(collector);
	}

	static middle::ComponentRegistrar<NonPhysicalBubbleTag>reg("NonPhysicalBubbleTag");
}
