#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLESNAPREF(X) \
	X(snapTargetId)

namespace components {
	struct SnapRef : public middle::Serializable{
		middle::Id snapTargetId;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLESNAPREF(X)
#undef X
		}
	};
}
