#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEEDITORCONFIGS(X) \
	X(gridSize) \
	X(visibleGridPointRadiusCount)

namespace components {
	struct EditorConfigs : public middle::Serializable{
		int gridSize = 1;
		int visibleGridPointRadiusCount = 10;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEEDITORCONFIGS(X)
#undef X
		}
	};
}

