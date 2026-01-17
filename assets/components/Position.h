#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct Position : public middle::Serializable{
		float posX;
		float posY;
		float posZ;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
	};
}
