#include "EditorConfigs.h"

namespace components {
	void EditorConfigs::serialize(std::ostream& ostream) {
		middle::Serializer serializer{ ostream };
		reflect(serializer);
	}

	void EditorConfigs::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		reflect(deserializer);
	}

	void EditorConfigs::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		middle::FieldCollector collector{ fields, size };
		reflect(collector);
	}

	static middle::ComponentRegistrar<EditorConfigs>reg("EditorConfigs");
}
