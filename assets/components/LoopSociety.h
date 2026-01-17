#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct LoopSociety : public middle::Serializable{
		middle::Id parentLoopId;
		std::vector<middle::Id>loopMemberIds;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
	};
}
