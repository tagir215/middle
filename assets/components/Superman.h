#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	static std::string componentName = "Superman";

	struct Superman : public middle::Serializable{
		int power = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer);
	};

}
