#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLETEXTURECOMPONENT(X) \
	X(path) \
	X(scale)

namespace components {
	struct TextureComponent : public middle::Serializable{
		std::string path;
		float scale = 1;
		Texture2D texture;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLETEXTURECOMPONENT(X)
#undef X
		}
	};
}
