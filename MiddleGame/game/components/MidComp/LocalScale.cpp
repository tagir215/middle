#include "LocalScale.h"

namespace components {
	void LocalScale::serialize(std::ostream& ostream) {
		middle::Serializer serializer{ ostream };
		reflect(serializer);
	}

	void LocalScale::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		reflect(deserializer);
	}

	void LocalScale::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		middle::FieldCollector collector{ fields, size };
		reflect(collector);
	}

	static middle::ComponentRegistrar<LocalScale>reg("LocalScale");
}
