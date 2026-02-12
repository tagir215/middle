#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLETESTCOMPONENT(X) \
	X(testVector3) \
	X(testVector2) \
	X(testColor) 


namespace components {
	struct TestComponent : public middle::Serializable{
		Vector3 testVector3;
		Vector2 testVector2;
		Color testColor;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLETESTCOMPONENT(X)
#undef X
		}
	};
}
