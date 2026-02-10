#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEIFCOMPONENT(X) \
	X(type)

namespace components {
	struct IfComponent : public middle::Serializable{
		int type = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEIFCOMPONENT(X)
#undef X
		}
	};
}
