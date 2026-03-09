#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "Position.h"
#include "Sphere.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "MouseGrabbable.h"
#include "PhysicsData.h"

namespace entities{

    inline void initJoint(middle::GameState* gameState, int index, const Vector3& position){
		middle::Shape shape;
		components::Sphere* sphere = middle::addComponent<components::Sphere>(shape);
		components::Position* pos = middle::addComponent<components::Position>(shape);
		components::PhysicsData* physics = middle::addComponent<components::PhysicsData>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::addComponent<components::LoopSociety>(shape);
		middle::registerShape(gameState, shape);
		sphere->radius = middle::DEF_RADIUS;
		pos->posX = position.x;
		pos->posY = position.y;
		pos->posZ = position.z;
		physics->mass = 1;
		physics->invMass = 1;
		physics->momentOfInertia = 1;
		physics->invMomentOfInertia = 1;
		physics->damX = middle::DEF_DAMPING;
		physics->damY = middle::DEF_DAMPING;
		physics->damZ = middle::DEF_DAMPING;
    }
}
