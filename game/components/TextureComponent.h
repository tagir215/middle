#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLETEXTURECOMPONENT(X) \
	X(path) \
	X(scale) \
	X(textureType) \
	X(filename) 

namespace middleTextureType {
	inline int BILLBOARD = 0;
	inline int BACKGROUND = 1;
}


namespace components {
	struct TextureComponent : public middle::Serializable {
		std::string path;
		std::string filename;
		float scale = 1;
		int textureType = middleTextureType::BILLBOARD;
		Texture2D texture;
		bool initialized = false;

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


