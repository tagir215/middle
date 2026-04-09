#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEUICOMPONENT(X) \
	X(type)

namespace components {
	struct UiComponent : public middle::Serializable{
		int type = middle::UNASSIGNED;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEUICOMPONENT(X)
#undef X
		}
	};
}

namespace UiElementTypes {
	static int MOVES_LEFT_INDICATOR = 0;
	static int UI_BACKGROUND = 1;
	static int OUT_OF_STEPS = 2;
	static int STEPS_LEFT_TEXT = 3;
	static int NEW_TERM_CHECK_BOX = 4;
	static int POSITIVE_NEGATIVE_CHECK_BOX = 5;
	static int INVERSE_CHECK_BOX = 6;
	static int UNIT_TYPE_INDICATOR = 7;
}
