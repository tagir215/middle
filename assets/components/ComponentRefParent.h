#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct ComponentRefParent : public middle::Serializable{
		std::vector<middle::Id>memberIds;
		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
	};
}
