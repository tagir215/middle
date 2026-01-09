#include "Sphere.h"

namespace components {
	void Sphere::serialize(std::ostream& ostream) {
		ostream << middle::fieldToString(radius);
	}

	void Sphere::deserialize(const std::vector<std::string>& buffer) {
		middle::fillField(&radius, buffer[0]);
	}

	static middle::ComponentRegistrar<Sphere>reg("Sphere");
}
