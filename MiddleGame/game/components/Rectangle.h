#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLERECTANGLE(X) \
	X(width) \
	X(height)

namespace components {
	struct Rectangle : public middle::Serializable{
		float width = 0;
		float height = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLERECTANGLE(X)
#undef X
		}
	};
}
