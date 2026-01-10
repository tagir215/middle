#pragma once
#include <vector>
#include "physics_body.h"
#include "descart_physics.h"
#include "descart_constraints.h"

namespace descart {

	void DescLoop(float frameTime, std::vector<BodyPair>& pairs, std::vector<Constraint>& constraints, std::vector<PhysicsBody>& bodies);

	void SolveConstraints(std::vector<Constraint>& constraints, std::vector<PhysicsBody>& bodies);

	void EulerIntegrate(float frameTime, std::vector<PhysicsBody>& bodies);

}
