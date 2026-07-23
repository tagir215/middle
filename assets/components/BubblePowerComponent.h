#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEBUBBLEPOWERCOMPONENT(X) 

namespace components {
	enum PowerRole {
			POWER_ROLE_BASE,
			POWER_ROLE_EXPONENT
	};
	struct BubblePowerComponent : public middle::Serializable{

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEBUBBLEPOWERCOMPONENT(X)
#undef X
		}
	};
}
