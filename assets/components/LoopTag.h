#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct LoopTag : public middle::Serializable{
		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset);
	};

}
