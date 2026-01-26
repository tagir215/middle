#include "descart_constraints.h"

namespace descart {

	void SolveDistanceConstraint(Constraint& constraint, std::vector<PhysicsBody>& bodies, float time)
	{
		PhysicsBody& bodyA = bodies[constraint.indexA];
		PhysicsBody& bodyB = bodies[constraint.indexB];

		assert(constraint.targetDistance != 0);

		Vec deltaPos = SubV(bodyB.position, bodyA.position);
		float posError = MagV(deltaPos) - constraint.targetDistance;
		// if posError == 0 default to this axis
		Vec axis = { 1,0,0 };
		if (MagSqV(deltaPos) != 0) {
			axis = NormV(deltaPos);
		}
		Vec posErrorV = ScaleV(axis, posError);
		Vec velError = ScaleV(axis, DotV(SubV(bodyB.linearVel, bodyA.linearVel), axis));

		float invMass = bodyA.invMass + bodyB.invMass;

		Vec biasV = ScaleV(posErrorV, -constraint.biasFactor);
		Vec totalError = AddV(velError, posErrorV);
		Vec impulse = NegateV(DivideV(totalError, invMass));

		// remove y component for now
		impulse.y = 0;

		bodyA.linearVel = ScaleV(AddV(bodyA.linearVel, NegateV(impulse)), bodyA.invMass);
		bodyB.linearVel = ScaleV(AddV(bodyB.linearVel, impulse), bodyB.invMass);

		//if (posError * 1.1f > constraint.targetDistance) {
		//	const float idk = 0.0001f;
		//	bodyA.position = AddV(bodyA.position, ScaleV(bodyA.linearVel, time * idk));
		//	bodyB.position = AddV(bodyB.position, ScaleV(bodyB.linearVel, time * idk));
		//	bodyA.timeLeft = 0;
		//	bodyB.timeLeft = 0;
		//}

		AssertNanVec(bodyA.linearVel);
		AssertNanVec(bodyB.linearVel);
	}

	void SolveCollisionConstraint(Constraint& initConstraint, std::vector<PhysicsBody>& bodies) {

		PhysicsBody& bodyA = bodies[initConstraint.indexA];
		PhysicsBody& bodyB = bodies[initConstraint.indexB];

		float relVel = RelativeVelocity(bodyA, bodyB, initConstraint.normal);
		if (relVel < 0)
			return;
		float emass = bodyA.invMass + bodyB.invMass;
		float impulseMag = -(1 + initConstraint.restitution) * relVel / emass;

		Vec impulse = ScaleV(initConstraint.normal, impulseMag);
		bodyA.linearVel = AddV(bodyA.linearVel, NegateV(ScaleV(impulse, bodyA.invMass)));
		bodyB.linearVel = AddV(bodyB.linearVel, ScaleV(impulse, bodyB.invMass));

	}

}
