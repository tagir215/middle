#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEDRAGSTART(X) \
	X(dragStartPos)

namespace components {
	struct DragStart : public middle::Serializable{
		midMath::Vector3 dragStartPos;
		midMath::Vector3 gizmoPos;
		midMath::Vector3 axis;
		int axisId;

		midMath::Quaternion initRotation;
		midMath::Vector3 initPosition;
		midMath::Vector3 initScale;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEDRAGSTART(X)
#undef X
		}
	};
}
