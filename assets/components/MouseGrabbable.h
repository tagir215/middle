#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct MouseGrabbable : public middle::Serializable{
		bool grabbing = false;
		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
	};
}
