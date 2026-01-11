#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct Sphere : public middle::Serializable{
        float radius;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset);
	};

}
