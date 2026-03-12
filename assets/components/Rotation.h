#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEROTATION(X) \
	X(rotation)

namespace components {
	struct Rotation : public middle::Serializable{
		Quaternion rotation;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEROTATION(X)
#undef X
		}
	};

}

namespace middle {
	const Vector3 ROTATION_FORWARD = { 0,0,1 };
}
