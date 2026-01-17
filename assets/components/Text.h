#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

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
	};
}
