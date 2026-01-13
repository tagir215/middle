#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct ComponentReference : public middle::Serializable{
		std::vector<std::string>componentNames;
		std::vector<float>positionsX;
		std::vector<float>positionsY;
		std::vector<float>positionsZ;
		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
	};
}
