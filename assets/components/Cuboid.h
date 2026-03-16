#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLECUBOID(X) \
	X(width) \
	X(height) \
	X(length)

namespace components {
	struct Cuboid : public middle::Serializable{
		float width = 0;
		float height = 0;
		float length = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLECUBOID(X)
#undef X
		}
	};
}
