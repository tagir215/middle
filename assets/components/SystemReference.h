#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {

	struct SystemReference : public middle::Serializable{
        std::string systemName;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer);
	};

}
