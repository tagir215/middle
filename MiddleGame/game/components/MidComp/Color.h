#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLECOLOR(X) \
	X(colorR) \
	X(colorG) \
	X(colorB) \
	X(colorA) \
	

namespace components {
	struct Color : public middle::Serializable{
		float colorR;
		float colorG;
		float colorB;
		float colorA;
		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;
		template<typename V>
		void reflect(V& v) {
		#define X(f) v(#f, f);
			MIDDLECOLOR(X)
		#undef X
		}
	};
}
