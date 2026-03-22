#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEALGEBRANODE(X) \
	X(type) \
	X(variableLabel) \
	X(value) \
	X(children) 


namespace components {
	struct AlgebraNode : public middle::Serializable{
		int type = middle::UNASSIGNED;
		std::string variableLabel = "";
		float value = 0;
		std::vector<middle::Id>children;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEALGEBRANODE(X)
#undef X
		}
	};

	enum class AlgebraNodeType {
		BUBBLE,
		VARIABLE,
		UNIT,
		FRACTION,
		MULTIPLICATION,
	};
}

