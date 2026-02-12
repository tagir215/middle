#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEOFFSET(X) \
	X(offsetX) \
	X(offsetY) \
	X(offsetZ) 

namespace components {
	struct Offset : public middle::Serializable{
		float offsetX = 0;
		float offsetY = 0;
		float offsetZ = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEOFFSET(X)
#undef X
		}
	};
}
