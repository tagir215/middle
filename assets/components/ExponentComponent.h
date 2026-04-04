#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEEXPONENTCOMPONENT(X) \
	X(power) \
	X(isInverse)

namespace components {
	struct ExponentComponent : public middle::Serializable{
		int power = 1;
		bool isInverse = false;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEEXPONENTCOMPONENT(X)
#undef X
		}
	};
}
