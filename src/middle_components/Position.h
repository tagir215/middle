#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct Position : public middle::Serializable{
        float posX = 0;
        float posY = 0;
        float posZ = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
	};

}
