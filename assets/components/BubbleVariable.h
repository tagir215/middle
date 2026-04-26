#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEBUBBLEVARIABLE(X) \
	X(label) \
	X(isNegative)


namespace components {
	struct BubbleVariable : public middle::Serializable{
		std::string label;
		bool isNegative = false;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEBUBBLEVARIABLE(X)
#undef X
		}
	};
}
