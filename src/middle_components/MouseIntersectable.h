#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct MouseIntersectable : public middle::Serializable{
        bool intersecting = false;
		bool wasIntersecting = false;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer);
	};

}
