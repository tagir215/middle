#include "PhysicsData.h"

namespace components {
	void PhysicsData::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(mass);
		ostream << middle::fieldToString(momentOfInertia);
		ostream << middle::fieldToString(invMass);
		ostream << middle::fieldToString(invMomentOfInertia);
		ostream << middle::fieldToString(radius);
		ostream << middle::fieldToString(linearDamping);
		ostream << middle::fieldToString(velX);
		ostream << middle::fieldToString(velY);
		ostream << middle::fieldToString(velZ);
		ostream << middle::fieldToString(accX);
		ostream << middle::fieldToString(accY);
		ostream << middle::fieldToString(accZ);
		ostream << middle::fieldToString(infiniteMass);
	}

	void PhysicsData::deserialize(const std::vector<std::string>& buffer) {
		middle::fillField(&mass, buffer[0]);
		middle::fillField(&momentOfInertia, buffer[1]);
		middle::fillField(&invMass, buffer[2]);
		middle::fillField(&invMomentOfInertia, buffer[3]);
		middle::fillField(&radius, buffer[4]);
		middle::fillField(&linearDamping, buffer[5]);
		middle::fillField(&velX, buffer[6]);
		middle::fillField(&velY, buffer[7]);
		middle::fillField(&velZ, buffer[8]);
		middle::fillField(&accX, buffer[9]);
		middle::fillField(&accY, buffer[10]);
		middle::fillField(&accZ, buffer[11]);
		middle::fillField(&infiniteMass, buffer[12]);
	}

	static middle::ComponentRegistrar<PhysicsData>reg("PhysicsData");
}
