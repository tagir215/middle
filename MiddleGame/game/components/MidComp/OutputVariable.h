#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEOUTPUTVARIABLE(X) \
	X(unitRef) \
	X(label)

namespace components {
	struct OutputVariable : public middle::Serializable{
		middle::Id unitRef;
		std::string label = "";

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEOUTPUTVARIABLE(X)
#undef X
		}
	};
}
