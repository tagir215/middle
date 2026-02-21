#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLECODEFUNCTION(X) \
	X(type)

namespace components {
	struct CodeFunction : public middle::Serializable{
		int type = middle::UNASSIGNED;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLECODEFUNCTION(X)
#undef X
		}
	};
}

namespace functionTypes {
	inline int COMBINE = 0;
	inline int ADD_TERM = 1;
	inline int NEW_TERM = 2;
	inline int NEW_MULTERM = 3;
	inline int FIND_BUBBLE = 4;
	inline int FIND_FRACTION = 5;
	inline int POP = 6;
	inline int INVERSE = 7;
	inline int EXIT_LOOP = 8;
}
