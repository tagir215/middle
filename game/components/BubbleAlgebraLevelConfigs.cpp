#include "BubbleAlgebraLevelConfigs.h"

namespace components {
	void BubbleAlgebraLevelConfigs::serialize(std::ostream& ostream) {
		middle::Serializer serializer{ ostream };
		reflect(serializer);
	}

	void BubbleAlgebraLevelConfigs::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		reflect(deserializer);
	}

	void BubbleAlgebraLevelConfigs::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		middle::FieldCollector collector{ fields, size };
		reflect(collector);
	}

	static middle::ComponentRegistrar<BubbleAlgebraLevelConfigs>reg("BubbleAlgebraLevelConfigs");
}
