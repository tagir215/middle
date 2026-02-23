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
	inline int ADD = 0;
	inline int MULTIPLY = 1;
	inline int NEW_TERM = 2;
	inline int NEW_MULTERM = 3;
	inline int POP = 4;
	inline int INVERSE = 5;
	inline int EXIT_LOOP = 6;
	inline int EQUAL = 7;
	inline int GREATER = 8;
	inline int GREATEREQUAL = 9;
	inline int LESS = 10;
	inline int LESSEQUAL = 11;
	inline int NEGATE = 12;
	inline int COPY = 13;
	inline int ZERO = 14;
	inline int ONE = 15;
	inline int FIND_BUBBLE = 16;
	inline int FIND_FRACTION = 17;
}

