#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLECAMERACOMPONENT(X) \
	X(targetX) \
	X(targetY) \
	X(targetZ) \
	X(upX) \
	X(upY) \
	X(upZ) \
	X(fovy) \
	X(projection) \
	X(active)

namespace components {
	struct CameraComponent : public middle::Serializable{
		float targetX;
		float targetY;
		float targetZ;
		float upX;
		float upY;
		float upZ;
		float fovy;             
		int projection;        
		bool active = false;
		float speedY = 0;
		float speedX = 0;
		float speedZ = 0;


		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLECAMERACOMPONENT(X)
#undef X
		}
	};
}
