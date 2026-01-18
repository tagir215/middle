#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLECONSTRAINT(X) \
	X(idA) \
	X(idB) \
	X(stiffness) \
	X(biasFactor) \
	X(targetDistance) 

namespace components {
	struct Constraint : public middle::Serializable{
		middle::Id idA;
		middle::Id idB;
		float stiffness = 0.8f;
		float biasFactor = 0.8f;;
		float targetDistance;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;
		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLECONSTRAINT(X)
#undef X
		}
	};
}
