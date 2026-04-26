#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEINVENTORYITEM(X) \
	X(itemType) 

namespace components {
	struct InventoryItem : public middle::Serializable{
		int itemType = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEINVENTORYITEM(X)
#undef X
		}
	};
}

namespace bubbleInventoryItemType {
	inline int DEFAULT = 0;
	inline int NEW_ADDITION_TERM = 1;
	inline int NEW_MULTIPLICATION_TERM = 2;
	inline int POP = 3;
	inline int MUL_ONE = 4;
	inline int BREAK_2 = 5;
	inline int BREAK_3 = 6;
	inline int BREAK_4 = 7;
	inline int BREAK_5 = 8;
	inline int BREAK_6 = 9;
	inline int BREAK_7 = 10;
	inline int BREAK_8 = 11;
	inline int BREAK_9 = 12;
	inline int BREAK_10 = 13;
	inline int COMPRESS_MULTIPLICATION = 14;
	inline int BUBBLIFY = 15;
	inline int SIMPLIFY = 16;
	inline int PROCEDURE = 17;
	inline int CANCEL = 18;
	inline int INSERT_X_OVER_X = 19;
	inline int INSERT_X_MINUS_X = 20;
	inline int MUL_NEGATIVE_ONE = 21;
	inline int COMPRESS_EXPONENT = 22;
}
