#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct Text : public middle::Serializable{
		std::string text = "Text";
		float offsetX = 0;
		float offsetY = 0;
		float offsetZ = 0;
		float fontColorR = 0;
		float fontColorG = 0;
		float fontColorB = 0;
		float fontColorA = 0;
		int fontSize = 1;
		bool visible = true;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
	};
}
