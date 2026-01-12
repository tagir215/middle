#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct LoopSociety : public middle::Serializable{
        int parentLoopIndex = UNASSIGNED;
		std::vector<int>loopMemberIndexes;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset);
	};

}
