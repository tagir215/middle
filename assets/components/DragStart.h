#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEDRAGSTART(X) \
	X(dragStartPos)

namespace components {
	struct DragStart : public middle::Serializable{
		Vector3 dragStartPos;
		Vector3 gizmoPos;
		Vector3 axis;
		int axisId;

		Quaternion initRotation;
		Vector3 initPosition;
		Vector3 initScale;

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
