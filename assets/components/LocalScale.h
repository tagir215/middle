#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLELOCALSCALE(X) \
	X(scale)

namespace components {
	struct LocalScale : public middle::Serializable{
		Vector3 scale = { 1,1,1 };

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLELOCALSCALE(X)
#undef X
		}
	};
}
