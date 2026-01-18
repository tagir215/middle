#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEPOSITION(X) \
	X(posX) \
	X(posY) \
	X(posZ) 

namespace components {
	struct Position : public middle::Serializable{
		float posX;
		float posY;
		float posZ;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;
		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEPOSITION(X)
#undef X
		}
	};
}
