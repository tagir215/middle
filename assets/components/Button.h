#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEBUTTON(X) \
	X(function)


namespace components {
	struct Button : public middle::Serializable{
		int function = middle::UNASSIGNED;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEBUTTON(X)
#undef X
		}
	};
}


namespace bubbleButton{
    inline int SAVE_PROCEDURE_BUTTON = 0;
    inline int LOAD_PROCEDURE_BUTTON = 1;
    inline int START_PROCEDURE_BUTTON = 2;
    inline int STEP_FORWARD = 3;
    inline int STEP_BACKWARD = 4;
	inline int UNDO = 5;
	inline int DONE = 6;
	inline int BACK = 7;
	inline int CONTINUE = 8;
	inline int IMPORT_PROCEDURE = 9;
	inline int SELECT_PLUS = 10;
	inline int SELECT_MULTIPLY = 11;
	inline int SELECT_POSITIVE = 12;
	inline int SELECT_NEGATIVE = 13;
	inline int SELECT_NON_INVERSE = 14;
	inline int SELECT_INVERSE = 15;
	inline int NEXT_UNIT = 16;
	inline int PREV_UNIT = 17;
	inline int SCROLL_UP = 18;
	inline int SCROLL_DOWN = 19;
	inline int SELECT_INSERT_X_OVER_X = 20;
	inline int SELECT_INSERT_X_MINUS_X = 21;
	inline int REVERSE_PROCEDURE = 22;
}

