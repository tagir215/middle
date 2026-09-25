#include "InitializedTag.h"

namespace components {
	void InitializedTag::serialize(std::ostream& ostream) {
		middle::Serializer serializer{ ostream };
		reflect(serializer);
	}

	void InitializedTag::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		reflect(deserializer);
	}

	void InitializedTag::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		middle::FieldCollector collector{ fields, size };
		reflect(collector);
	}

	static middle::ComponentRegistrar<InitializedTag>reg("InitializedTag");
}
