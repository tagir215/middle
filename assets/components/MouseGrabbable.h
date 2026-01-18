#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
# define MIDDLEMOUSEGRABBABLE(X)

namespace components {
	struct MouseGrabbable : public middle::Serializable{
		bool grabbing = false;
		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;
		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEMOUSEGRABBABLE(X)
#undef X
		}
	};
}
