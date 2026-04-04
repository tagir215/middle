#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLELEVELREFERENCE(X) \
	X(levelName) \
	X(complete)

namespace components {
	struct LevelReference : public middle::Serializable{
		std::string levelName = "";
		bool complete = false;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLELEVELREFERENCE(X)
#undef X
		}
	};
}

namespace bubbleLevelConstants {
	//std::string folder = "../bubbleData/problems/";
}
