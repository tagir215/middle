#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct Color : public middle::Serializable{
        float colorR;
        float colorG;
        float colorB;
        float colorA;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset);
	};

}
