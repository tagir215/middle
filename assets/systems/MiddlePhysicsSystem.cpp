#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Constraint.h"
#include "PhysicsData.h"
#include "descart_loop.h"
#include "Position.h"

class MiddlePhysicsSystem : public middle::MiddleGameplaySystem {
public:
	MiddlePhysicsSystem() {
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
		systemModeType = middle::SystemModeType::EDITOR;
	}

	components::CompCache* physicsCache;
	components::CompCache* constraintCache;

	void init(middle::GameState* gameState) {
		physicsCache = middle::newCompCache(gameState);
		physicsCache->addType<components::PhysicsData>();
		physicsCache->addType<components::Position>();
		constraintCache = middle::newCompCache(gameState);
		constraintCache->addType<components::Constraint>();
	}

	void update(middle::GameState* gameState) override {

		// create pairs

		std::vector<Constraint> constraints;
		std::vector<BodyPair> pairs;
		std::vector<int> grounds;

		if (gameState->physicsBodies.size() < physicsCache->getSize()) {
			gameState->physicsBodies.resize(physicsCache->getSize());
		}

		auto physicsIt = physicsCache->begin<components::PhysicsData>();
		auto posIt = physicsCache->begin<components::Position>();
		for (int i = 0; i < physicsCache->getSize(); ++i) {
			auto pcomp = *physicsIt;
			auto pos = *posIt;
			gameState->physicsBodies[i].active = true;
			if (gameState->physicsBodies.size() <= i) {
				gameState->physicsBodies.resize(i + 100);
			}
			PhysicsBody& body = gameState->physicsBodies[i];
			body.infiniteMass = pcomp->infiniteMass;
			body.mass = pcomp->mass;
			body.invMass = pcomp->invMass;
			body.momentOfInertia = pcomp->momentOfInertia;
			body.invMomentOfInertia = pcomp->invMomentOfInertia;
			body.colliderType = ColliderType::CIRC;
			body.linearAcc = { pcomp->accX, pcomp->accY, pcomp->accZ };
			body.linearVel = { pcomp->velX, pcomp->velY, pcomp->velZ };
			body.linearDamping = pcomp->damY;
			body.position = { pos->posX, pos->posY, pos->posZ };
			body.timeLeft = 1;
		}

		auto constraintsIt = constraintCache->begin<components::Constraint>();
		for (int i = 0; i < constraintCache->getSize(); ++i) {
			auto constraint = *constraintsIt;
			if (constraint->idA.index != middle::UNASSIGNED && constraint->idB.index != middle::UNASSIGNED) {
				Constraint c;
				c.indexA = constraint->idA.index;
				c.indexB = constraint->idB.index;
				c.biasFactor = constraint->biasFactor;
				c.stiffness = constraint->stiffness;
				c.targetDistance = constraint->targetDistance;
				c.type = ConstraintType::distance;
				constraints.push_back(c);
			}
		}

		const int iterations = 12;
		DescLoop(gameState->frameTime, pairs, constraints, gameState->physicsBodies, iterations);

		physicsIt = physicsCache->begin<components::PhysicsData>();
		posIt = physicsCache->begin<components::Position>();
		for (int i = 0; i < physicsCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, physicsCache->relevantIdVector[i].index);
			auto pcomp = *physicsIt;
			auto pos = *posIt;
			if (pcomp != nullptr && pos != nullptr) {
				PhysicsBody& body = gameState->physicsBodies[i];
				pcomp->velX = body.linearVel.x;
				pcomp->velY = body.linearVel.y;
				pcomp->velZ = body.linearVel.z;
				pos->posX = body.position.x;
				pos->posY = body.position.y;
				pos->posZ = body.position.z;
			}
		}
	}
};

static middle::SystemRegistrar<MiddlePhysicsSystem> reg("MiddlePhysicsSystem");
