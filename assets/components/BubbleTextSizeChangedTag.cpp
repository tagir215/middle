#include "BubbleTextSizeChangedTag.h"

namespace components {
	void BubbleTextSizeChangedTag::serialize(std::ostream& ostream) {
		middle::Serializer serializer{ ostream };
		reflect(serializer);
	}

	void BubbleTextSizeChangedTag::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		reflect(deserializer);
	}

	void BubbleTextSizeChangedTag::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		middle::FieldCollector collector{ fields, size };
		reflect(collector);
	}

	static middle::ComponentRegistrar<BubbleTextSizeChangedTag>reg("BubbleTextSizeChangedTag");
}
