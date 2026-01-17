#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct Constraint : public middle::Serializable{
		middle::Id idA;
		middle::Id idB;
		float stiffness = 0.8f;
		float biasFactor = 0.8f;;
		float targetDistance;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
	};
}
