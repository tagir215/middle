#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

#define MIDDLEPHYSICSDATA(X) \
	X(mass) \
	X(invMass) \
	X(momentOfInertia) \
	X(velX) \
	X(velY) \
	X(velZ) \
	X(damX) \
	X(damY) \
	X(damZ) \
	X(accX) \
	X(accY) \
	X(accZ) \
	X(infiniteMass) 

namespace components {
	struct PhysicsData : public middle::Serializable{
		float mass = 1;
		float invMass = 1;
		float momentOfInertia = 1;
		float invMomentOfInertia = 1;
		float velX = 0;
		float velY = 0;
		float velZ = 0;
		float damX = 0;
		float damY = 0;
		float damZ = 0;
		float accX = 0;
		float accY = 0;
		float accZ = 0;
		bool infiniteMass = false;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;
		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEPHYSICSDATA(X)
#undef X
		}
	};
}
