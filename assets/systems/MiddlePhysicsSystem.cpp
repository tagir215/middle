#pragma once
#include "game_state.h"
#include "registrars.h"
#include "middle_shape_utils.h"
#include "Constraint.h"
#include "PhysicsData.h"
#include "descart_loop.h"
#include "Position.h"

class MiddlePhysicsSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {

		// shape phsyics stuff
		// create pairs

		std::vector<Constraint> constraints;
		std::vector<BodyPair> pairs;
		std::vector<int> grounds;
		if (gameState->physicsBodies.size() < gameState->shapes.size()) {
			gameState->physicsBodies.resize(gameState->shapes.size());
		}

		for (int i = 0; i < gameState->shapes.size(); ++i) {
			gameState->physicsBodies[i].active = false;
			if (!isShapeAlive(gameState, i)) {
				continue;
			}

			auto& shape = getShape(gameState, i);

			auto constraint = middle::getComponent<components::Constraint>(shape);
			if (constraint) {
				if (constraint->indexA != UNASSIGNED && constraint->indexB != UNASSIGNED) {
					Constraint c;
					c.indexA = constraint->indexA;
					c.indexB = constraint->indexB;
					c.biasFactor = constraint->biasFactor;
					c.stiffness = constraint->stiffness;
					c.targetDistance = constraint->targetDistance;
					c.type = ConstraintType::distance;
					constraints.push_back(c);
				}
			}
			auto pcomp = middle::getComponent<components::PhysicsData>(shape);
			auto pos = middle::getComponent<components::Position>(shape);
			if (pcomp) {
				gameState->physicsBodies[i].active = true;
				assert(pos != nullptr);
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
			}
		}

		DescLoop(gameState->frameTime, pairs, constraints, gameState->physicsBodies);

		loopInstances(gameState, [&](int i, middle::Shape& shape) {
			auto pcomp = middle::getComponent<components::PhysicsData>(shape);
			auto pos = middle::getComponent<components::Position>(shape);
			if (pcomp != nullptr && pos != nullptr) {
				PhysicsBody& body = gameState->physicsBodies[i];
				pcomp->velX = body.linearVel.x;
				pcomp->velY = body.linearVel.y;
				pcomp->velZ = body.linearVel.z;
				pos->posX = body.position.x;
				pos->posY = body.position.y;
				pos->posZ = body.position.z;
			}
			});
	}
};

static middle::SystemRegistrar<MiddlePhysicsSystem> reg("MiddlePhysicsSystem");
