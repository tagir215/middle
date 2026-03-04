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
    inline int SAVE_BUTTON = 0;
    inline int LOAD_BUTTON = 1;
    inline int START_PROCEDURE_BUTTON = 2;
    inline int STEP_FORWARD = 3;
    inline int STEP_BACKWARD = 4;
	inline int UNDO = 5;
}

