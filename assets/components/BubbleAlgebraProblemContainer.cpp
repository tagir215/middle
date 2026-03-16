#include "BubbleAlgebraProblemContainer.h"

namespace components {
	void BubbleAlgebraProblemContainer::serialize(std::ostream& ostream) {
		middle::Serializer serializer{ ostream };
		reflect(serializer);
	}

	void BubbleAlgebraProblemContainer::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		reflect(deserializer);
	}

	void BubbleAlgebraProblemContainer::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		middle::FieldCollector collector{ fields, size };
		reflect(collector);
	}

	static middle::ComponentRegistrar<BubbleAlgebraProblemContainer>reg("BubbleAlgebraProblemContainer");
}
