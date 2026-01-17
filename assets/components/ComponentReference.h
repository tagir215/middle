#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct ComponentReference : public middle::Serializable{
		std::string componentName;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
	};
}
