#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLETRIANGLE(X) \
	X(width) \
	X(height)

namespace components {
	struct Triangle : public middle::Serializable{
		float width;
		float height;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLETRIANGLE(X)
#undef X
		}
	};
}
