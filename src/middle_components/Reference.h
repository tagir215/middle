#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {

	struct Reference : public middle::Serializable{
        std::string sceneName;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset);
	};

}
