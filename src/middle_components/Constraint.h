#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct Constraint : public middle::Serializable{
        int indexA = -1;
        int indexB = -1;
        float stiffness = 0.7f;
        float biasFactor = 0.9f;
        float targetDistance = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset);
	};

}
