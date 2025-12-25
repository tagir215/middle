#include "descart_constraints.h"

namespace descart {

	void SolveDistanceConstraint(Constraint& constraint, std::vector<PhysicsBody*>& bodies)
	{
		PhysicsBody* bodyA = bodies[constraint.indexA];
		PhysicsBody* bodyB = bodies[constraint.indexB];

		assert(constraint.targetDistance != 0);

		Vec deltaPos = SubV(bodyB->position, bodyA->position);
		float posError = MagV(deltaPos) - constraint.targetDistance;
		// if posError == 0 default to this axis
		Vec axis = { 1,0,0 };
		if (MagSqV(deltaPos) != 0) {
			axis = NormV(deltaPos);
		}
		Vec posErrorV = ScaleV(axis, posError);
		Vec velError = ScaleV(axis, DotV(SubV(bodyB->linearVel, bodyA->linearVel), axis));

		float invMass = bodyA->invMass + bodyB->invMass;

		Vec biasV = ScaleV(posErrorV, -constraint.biasFactor);
		Vec totalError = AddV(velError, posErrorV);
		Vec impulse = NegateV(DivideV(totalError, invMass));

		// remove y component for now
		impulse.y = 0;

		bodyA->linearVel = ScaleV(AddV(bodyA->linearVel, NegateV(impulse)), bodyA->invMass);
		bodyB->linearVel = ScaleV(AddV(bodyB->linearVel, impulse), bodyB->invMass);

		AssertNanVec(bodyA->linearVel);
		AssertNanVec(bodyB->linearVel);
	}

	void SolveCollisionConstraint(Constraint& constraint, std::vector<PhysicsBody*>& bodies) {

		PhysicsBody* bodyA = bodies[constraint.indexA];
		PhysicsBody* bodyB = bodies[constraint.indexB];

		float relVel = RelativeVelocity(bodyA, bodyB, constraint.normal);
		if (relVel < 0)
			return;
		float emass = bodyA->invMass + bodyB->invMass;
		float impulseMag = -(1 + constraint.restitution) * relVel / emass;

		Vec impulse = ScaleV(constraint.normal, impulseMag);
		bodyA->linearVel = AddV(bodyA->linearVel, NegateV(ScaleV(impulse, bodyA->invMass)));
		bodyB->linearVel = AddV(bodyB->linearVel, ScaleV(impulse, bodyB->invMass));

	}

}
