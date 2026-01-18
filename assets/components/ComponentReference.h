#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLECOMPONENTREFERENCE(X) \
	X(componentName)

namespace components {
	struct ComponentReference : public middle::Serializable{
		std::string componentName;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;
		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLECOMPONENTREFERENCE(X)
#undef X
		}
	};
}
