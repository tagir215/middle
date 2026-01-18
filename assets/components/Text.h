#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLETEXT(X) \
	X(text) \
	X(fontColorR) \
	X(fontColorG) \
	X(fontColorB) \
	X(fontColorA) \
	X(offsetX) \
	X(offsetY) \
	X(offsetZ) \
	X(fontSize) \
	X(visible) 

namespace components {
	struct Text : public middle::Serializable{
		std::string text;
		float fontColorR;
		float fontColorG;
		float fontColorB;
		float fontColorA;
		float offsetX = 0;
		float offsetY = 0;
		float offsetZ = 0;
		float fontSize = 1;
		bool visible = true;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;
		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLETEXT(X)
#undef X
		}
	};
}
