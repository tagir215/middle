#pragma once
#include "descart_math.h"

namespace descart{

	enum ColliderType {
		CIRC,
		POL,
		LIN,
		ELLPS,
		SPHE
	};

	struct PhysicsBody {
		bool active;
		Matr transform;
		float mass;
		float momentOfInertia;
		float invMass;
		float invMomentOfInertia;
		float radius = 0;
		float linearDamping = 0;
		bool infiniteMass = false;
		bool collided = false;
		float normalizedTimeElapsed = 0;
		float timeLeft = 1;

		Vec position;
		Vec PointA;
		Vec PointB;
		Vec linearVel;
		Vec linearVelBefore;
		Vec linearVelAfterForces;
		Vec linearVelAfterImpulses;
		Vec linearDisp;
		Vec linearDispBefore;
		Vec linearDispAfterForces;
		Vec linearDispAfterImpulses;
		Vec linearAcc;
		ColliderType colliderType;
	};


}
