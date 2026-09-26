#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "MidComp/Sphere.h"
#include "MidComp/MouseSelectable.h"
#include "MidComp/MouseIntersectable.h"
#include "MidComp/LoopSociety.h"
#include "MidComp/MouseGrabbable.h"
#include "MidComp/PhysicsData.h"
#include "MidComp/GlobalTransform.h"
#include "MidComp/LocalPosition.h"
#include "MidComp/LocalScale.h"

namespace entities{

    inline void initJoint(middle::GameState* gameState, int index, const midMath::Vector3& position){
		middle::Shape shape = middle::createShape(gameState);
		components::Sphere* sphere = middle::addComponent<components::Sphere>(shape);
		components::LocalPosition* pos = middle::addComponent<components::LocalPosition>(shape);
		middle::addComponent<components::LocalScale>(shape);
		middle::addComponent<components::GlobalTransform>(shape);
		components::PhysicsData* physics = middle::addComponent<components::PhysicsData>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::addComponent<components::LoopSociety>(shape);
		middle::registerShape(gameState, shape);
		sphere->radius = middle::DEF_RADIUS;
		pos->pos = position;
		physics->mass = 1;
		physics->invMass = 1;
		physics->momentOfInertia = 1;
		physics->invMomentOfInertia = 1;
		physics->damX = middle::DEF_DAMPING;
		physics->damY = middle::DEF_DAMPING;
		physics->damZ = middle::DEF_DAMPING;
    }
}
