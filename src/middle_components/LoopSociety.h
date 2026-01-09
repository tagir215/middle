#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct LoopSociety : public middle::Serializable{
        int loopArrayOffset = UNASSIGNED;
        int loopSize = 0;
        int parentLoopIndex = UNASSIGNED;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer);
	};

}
