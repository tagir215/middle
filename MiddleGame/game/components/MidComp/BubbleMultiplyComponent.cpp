#include "BubbleMultiplyComponent.h"

namespace components {
	void BubbleMultiplyComponent::serialize(std::ostream& ostream) {
		middle::Serializer serializer{ ostream };
		reflect(serializer);
	}

	void BubbleMultiplyComponent::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		reflect(deserializer);
	}

	void BubbleMultiplyComponent::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		middle::FieldCollector collector{ fields, size };
		reflect(collector);
	}

	static middle::ComponentRegistrar<BubbleMultiplyComponent>reg("BubbleMultiplyComponent");
}
