#include "MultiplicationTag.h"

namespace components {
	void MultiplicationTag::serialize(std::ostream& ostream) {
		//middle::Serializer serializer{ ostream };
		//reflect(serializer);
	}

	void MultiplicationTag::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		//middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		//reflect(deserializer);
	}

	void MultiplicationTag::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		//middle::FieldCollector collector{ fields, size };
		//reflect(collector);
	}

	static middle::ComponentRegistrar<MultiplicationTag>reg("MultiplicationTag");
}
