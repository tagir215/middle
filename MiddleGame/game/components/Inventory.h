#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEINVENTORY(X) \
	X(maxSize)

namespace components {
	enum InventoryInsertType {
		INSERT_ADD,
		INSERT_MULTIPLY,
		INSERT_POWER
	};
	enum InventoryInvariantType {
		X_OVER_X,
		X_MINUS_X
	};
	struct Inventory : public middle::Serializable{
		int maxSize = 5;
		int activeIndex = 0;
		bool invert = false;
		bool negate = false;
		InventoryInsertType insertType;
		InventoryInvariantType invariantType;
		

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEINVENTORY(X)
#undef X
		}
	};
}
