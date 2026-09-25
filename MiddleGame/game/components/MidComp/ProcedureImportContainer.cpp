#include "ProcedureImportContainer.h"

namespace components {
	void ProcedureImportContainer::serialize(std::ostream& ostream) {
		middle::Serializer serializer{ ostream };
		reflect(serializer);
	}

	void ProcedureImportContainer::deserialize(const std::vector<std::string>& buffer, int indexOffset) {
		middle::Deserializer deserializer{ buffer, indexOffset, 0 };
		reflect(deserializer);
	}

	void ProcedureImportContainer::getFields(std::vector<middle::FieldInfo>& fields, int* size)
	{
		middle::FieldCollector collector{ fields, size };
		reflect(collector);
	}

	static middle::ComponentRegistrar<ProcedureImportContainer>reg("ProcedureImportContainer");
}
