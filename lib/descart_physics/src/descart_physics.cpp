#include "descart_physics.h"

namespace descart {

	float RelativeVelocity(const PhysicsBody* bodyA, const PhysicsBody* bodyB, const Vec& normal)
	{
		// TODO
		Vec deltaVel = SubV(bodyB->linearVel, bodyA->linearVel);
		return DotV(normal, deltaVel);
	}

	CollisionResult TestCircleLine(Vec posCircle, float radius, Vec linePosA, Vec linePosB) {
		Vec relA = NormV(RelV(linePosB, linePosA));
		Vec relB = RelV(posCircle, linePosA);
		float dot = DotV(relA, relB);
		Vec collisionPoint = AddV(linePosA, ScaleV(relA, dot));

		Vec distanceVec = RelV(collisionPoint, posCircle);
		float distSq = MagSqV(distanceVec);

		return {
			collisionPoint,
			{0,1,0},
			distSq < radius * radius,
		};
	}

	SweepResult SweepCircleLine(Vec posCircle, Vec displacementCircle, float radius, Vec linePosA, Vec linePosB) {

		// find closest point on circle
		Vec lineDir = NormV(SubV(linePosB, linePosA));
		Vec toCircle = SubV(posCircle, linePosA);
		float projectionMag = DotV(lineDir, toCircle);
		Vec closestPointOnLine = AddV(linePosA, ScaleV(lineDir, projectionMag));
		Vec toClosest = NormV(SubV(closestPointOnLine, posCircle));
		Vec closestPointOnCircle = AddV(posCircle, ScaleV(toClosest, radius));


		// sweep closest point with line
		Vec velDir = NormV(displacementCircle);
		float vax = velDir.x;
		float vay = velDir.z;
		float vbx = lineDir.x;
		float vby = lineDir.z;
		float pax = posCircle.x;
		float pay = posCircle.z;
		float pbx = linePosA.x;
		float pby = linePosB.z;
		float l = (vax * (pay - pby) + vay * (pbx - pax)) / (vby * vax - vay * vbx);
		Vec collisionPoint = AddV(linePosA, ScaleV(lineDir, l));

		bool approachingLine = DotV(SubV(closestPointOnCircle, collisionPoint), displacementCircle) < 0;

		SweepResult result;
		result.toi = MagV(SubV(collisionPoint, closestPointOnCircle)) / MagV(displacementCircle);
		result.collided = result.toi < 1.0f && result.toi >= 0 && approachingLine;
		if (result.collided) {
			Vec posAfter = AddV(posCircle, ScaleV(displacementCircle, result.toi));
			result.normal = NormV(SubV(posAfter, collisionPoint));
			result.collisionPoint = collisionPoint;
		}

		return result;
	}

	void AssertBody(PhysicsBody* body)
	{
		AssertNanVec(body->linearVel);
		AssertNanVec(body->position);
		assert(body->mass);
		assert(body->invMass);
		assert(body->momentOfInertia);
		assert(body->invMomentOfInertia);
	}


}
