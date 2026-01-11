#pragma once
#include "registrars.h"
#include "editor_file_utils.h"

namespace components {
	struct PhysicsData : public middle::Serializable{
        float mass = 0;
        float momentOfInertia = 0;
        float invMass = 0;
        float invMomentOfInertia = 0;
        float radius = 0;
        float velX = 0;
        float velY = 0;
        float velZ = 0;
        float accX = 0;
        float accY = 0;
        float accZ = 0;
        float damX = 0;
        float damY = 0;
        float damZ = 0;
        bool infiniteMass = false;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer);
	};

}
