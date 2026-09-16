#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLELAYER(X) \
	X(layer)


namespace components {
	struct Layer : public middle::Serializable{
		int layer = 0;
		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLELAYER(X)
#undef X
		}
	};
}
