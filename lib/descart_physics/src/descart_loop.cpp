#include "descart_loop.h"
#include <cassert>

namespace descart {

	void SubLoop(float frameTime, std::vector<BodyPair>& pairs, std::vector<Constraint>& constraints, std::vector<PhysicsBody>& bodies) {

		SolveConstraints(constraints, bodies);

		for (BodyPair& pair : pairs) {
			PhysicsBody& bodyA = bodies[pair.indexA];
			PhysicsBody& bodyB = bodies[pair.indexB];

			if (bodyA.normalizedTimeElapsed >= 1) {
				continue;
			}

			if (bodyA.colliderType == ColliderType::CIRC && bodyB.colliderType == ColliderType::LIN) {
				SweepResult sweepResult = SweepCircleLine(bodyA.position, ScaleV(bodyA.linearVel, frameTime),
					bodyA.radius, bodyB.PointA, bodyB.PointB);

				if (bodyA.normalizedTimeElapsed == 0 || sweepResult.collided) {
					pair.sweepResult = sweepResult;
				}

				if (sweepResult.collided) {
					Constraint collision;
					collision.type = ConstraintType::collision;
					collision.biasFactor = 1;
					collision.collisionPoint = sweepResult.collisionPoint;
					collision.normal = sweepResult.normal;
					collision.restitution = 1;
					collision.indexA = pair.indexA;
					collision.indexB = pair.indexB;
					collision.toi = sweepResult.toi;
					//constraints.push_back(collision);
					SolveCollisionConstraint(collision, bodies);
				}

				// update time
				if (sweepResult.collided) {
					// move to collision point
					bodyA.position = AddV(bodyA.position, ScaleV(bodyA.linearDisp, sweepResult.toi));
					bodyA.normalizedTimeElapsed += sweepResult.toi;
				}
				else {
					bodyA.normalizedTimeElapsed = 1 - bodyA.normalizedTimeElapsed;
				}
			}


		}


	}

	void SolveConstraints(std::vector<Constraint>& constraints, std::vector<PhysicsBody>& bodies) {
		for (Constraint& initConstraint : constraints) {
			switch (initConstraint.type) {
			case ConstraintType::collision:
				break;
			case ConstraintType::distance:
				SolveDistanceConstraint(initConstraint, bodies);
				break;
			}
		}
	}

	void DescLoop(float frameTime, std::vector<BodyPair>& pairs, std::vector<Constraint>& constraints, std::vector<PhysicsBody>& bodies, int iterations)
	{
		for (auto& body : bodies) {
			if (!body.active)
				continue;
			AssertBody(body);
			body.normalizedTimeElapsed = 1;
			body.collided = false;
			// add gravity
			body.linearVelBefore = body.linearVel;
			body.linearDispBefore = ScaleV(body.linearVel, frameTime);
			body.linearVel = AddV(body.linearVel, ScaleV(body.linearAcc, frameTime));
			body.linearDisp = ScaleV(body.linearVel, frameTime);
			body.linearVelAfterForces = body.linearVel;
			body.linearDispAfterForces = body.linearDisp;
		}

		// for collision pairs set normalized time to 0
		for (auto& pair : pairs) {
			auto& bodyA = bodies[pair.indexA];
			auto& bodyB = bodies[pair.indexB];
			bodyA.normalizedTimeElapsed = 0;
			bodyB.normalizedTimeElapsed = 0;
		}

		for (int i = 0; i < iterations; ++i) {
			SubLoop(frameTime, pairs, constraints, bodies);
			SubLoop(frameTime, pairs, constraints, bodies);
		}

		for (auto& body : bodies) {
			if (!body.active)
				continue;
			body.linearVelAfterImpulses = body.linearVel;
			body.linearDispAfterImpulses = ScaleV(body.linearVel, frameTime);
		}

		EulerIntegrate(frameTime, bodies);

		for (int i = constraints.size() - 1; i >= 0; --i) {
			auto& initConstraint = constraints[i];
			if (initConstraint.type == ConstraintType::collision) {
				constraints.pop_back();
			}
		}
	}

	void EulerIntegrate(float frameTime, std::vector<PhysicsBody>& bodies)
	{
		for (PhysicsBody& body : bodies) {
			if (!body.active)
				continue;
			body.position = AddV(ScaleV(body.linearVel, frameTime * body.normalizedTimeElapsed), body.position);
			body.transform = MatTranslate(body.position);
			body.linearVel = ScaleV(body.linearVel, (1 - body.linearDamping));
			AssertBody(body);
		}
	}

}
