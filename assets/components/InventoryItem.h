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

namespace bubbleInventoryitemType {
	inline int DEFAULT = 0;
	inline int MULTIPLICATION = 1;
	inline int ADD = 2;
	inline int POP = 3;
	inline int TIMES_ONE = 4;
	inline int BREAK_2 = 5;
	inline int BREAK_3 = 6;
	inline int BREAK_4 = 7;
	inline int BREAK_5 = 8;
	inline int BREAK_6 = 9;
	inline int BREAK_7 = 10;
	inline int BREAK_8 = 11;
	inline int BREAK_9 = 12;
	inline int BREAK_10 = 13;
	inline int COMPRESS = 14;
}
