#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEBUBBLEROOTCOMPONENT(X) \
	X(power) \
	X(isNegative) \
	X(isInverse)

namespace components {
	struct BubbleRootComponent : public middle::Serializable{
		int power = 1;
		bool isNegative = false;
		bool isInverse = false;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEBUBBLEROOTCOMPONENT(X)
#undef X
		}
	};
}
