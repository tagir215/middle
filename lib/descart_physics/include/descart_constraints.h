#pragma once
#include <vector>
#include "physics_body.h"
#include "descart_physics.h"

namespace descart {

	enum ConstraintType {
		none,
		collision,
		distance,
		spring
	};

	struct Constraint {
		ConstraintType type;
		int indexA = -1;
		int indexB = -1;
		float stiffness = 0.7f;
		float biasFactor = 0.9f;
		float targetDistance;
		Vec collisionPoint;
		Vec normal;
		float restitution;
		float toi;
	};

	void SolveDistanceConstraint(Constraint& constraint, std::vector<PhysicsBody*>& bodies);

	void SolveCollisionConstraint(Constraint& constraint, std::vector<PhysicsBody*>& bodies);

}
