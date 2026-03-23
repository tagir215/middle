#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#include "bubble_actions.h"
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
	inline int NEW_TERM = 1;
	inline int NEW_MULTERM = 2;
	inline int POP = 3;
	inline int INVERSE = 4;
	inline int EXIT_LOOP = 5;
	inline int EQUAL = 6;
	inline int GREATER = 7;
	inline int GREATEREQUAL = 8;
	inline int LESS = 9;
	inline int LESSEQUAL = 10;
	inline int NEGATE = 11;
	inline int COPY = 12;
	inline int ZERO = 13;
	inline int ONE = 14;
	inline int FIND_BUBBLE = 15;
	inline int FIND_FRACTION = 16;
	inline int MUL_ONE = 17;
	inline int BREAK = 18;
	inline int COMPRESS = 19;
	inline int FIND_UNIT = 20;
}

