#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEGLOBALTRANSFORM(X) \
	X(pos) \
	X(scale) \
	X(rotation) 

namespace components {
	struct GlobalTransform : public middle::Serializable{
		midMath::Vector3 pos = { 0,0,0 };
		midMath::Vector3 scale = { 1,1,1 };
		midMath::Quaternion rotation = {0,0,0,0};

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEGLOBALTRANSFORM(X)
#undef X
		}
	};
}
