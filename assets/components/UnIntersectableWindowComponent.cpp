#include "UnIntersectableWindowComponent.h"

namespace components {
	void UnIntersectableWindowComponent::serialize(std::ostream& ostream) {
		middle::Serializer serializer{ ostream };
		reflect(serializer);
	}

	void UnIntersectableWindowComponent::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		reflect(deserializer);
	}

	void UnIntersectableWindowComponent::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		middle::FieldCollector collector{ fields, size };
		reflect(collector);
	}

	static middle::ComponentRegistrar<UnIntersectableWindowComponent>reg("UnIntersectableWindowComponent");
}
