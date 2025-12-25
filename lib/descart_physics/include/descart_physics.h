#pragma once
#include "physics_body.h"

namespace descart {

	struct CollisionResult {
		Vec collisionPoint;
		Vec normal;
		bool collided = false;
	};

	struct SweepResult {
		Vec collisionPoint;
		Vec normal;
		bool collided = false;
		float toi;
	};


	struct ImpulseResult {
		Vec point;
		Vec impulse;
		Vec linearVelA;
		Vec linearVelB;
		Vec angularVelA;
		Vec angularVelB;
	};

	struct BodyPair {
		int indexA;
		int indexB;
		SweepResult sweepResult;
	};

	float RelativeVelocity(const PhysicsBody* bodyA, const PhysicsBody* bodyB, const Vec& normal);
	CollisionResult TestCircleLine(Vec posCircle, float radius, Vec linePosA, Vec linePosB);
	SweepResult SweepCircleLine(Vec posCircle, Vec displacementCircle, float radius, Vec linePosA, Vec linePosB);

	void AssertBody(PhysicsBody* body);

}
