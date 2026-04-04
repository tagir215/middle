#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEINVENTORY(X) \
	X(horizontal) \
	X(rows) \
	X(freeLayout)

namespace components {
	struct Inventory : public middle::Serializable{
		bool horizontal = false;
		int rows = 1;
		bool freeLayout = false;

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
