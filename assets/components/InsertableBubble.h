#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEINSERTABLEBUBBLE(X)

namespace components {

	enum InsertableBubbleType {
		ADD_OUTER,
		MULTIPLY_OUTER,
		POWER_OUTER,
		ADD_X_MINUS_X,
		MULTIPLY_X_OVER_X
	};

	enum InsertableBubbleForm {
		DIRECT,
		NEGATED,
		INVERTED
	};

	struct InsertableBubble : public middle::Serializable {
		int insertableBubbleType = 0;
		int form = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEINSERTABLEBUBBLE(X)
#undef X
		}
	};
}
