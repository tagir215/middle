#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "MidComp/LocalPosition.h"
#include "MidComp/LocalScale.h"
#include "MidComp/GlobalTransform.h"
#include "MidComp/Sphere.h"
#include "MidComp/MouseSelectable.h"
#include "MidComp/MouseIntersectable.h"
#include "MidComp/LoopSociety.h"
#include "MidComp/MouseGrabbable.h"
#include "MidComp/PhysicsData.h"
#include "middle_shape_utils.h"
#include "MidComp/CameraComponent.h"

namespace entities{

    void initCamera(middle::GameState* gameState, int index, const midMath::Vector3& position, const midMath::Vector3& up, const midMath::Vector3& target, float fovy, int projection){
		middle::Shape shape = middle::createShape(gameState);
		components::Sphere* sphere = middle::addComponent<components::Sphere>(shape);
		components::LocalPosition* pos = middle::addComponent<components::LocalPosition>(shape);
		middle::addComponent<components::LocalScale>(shape);
		middle::addComponent<components::GlobalTransform>(shape);
		components::CameraComponent* camera = middle::addComponent<components::CameraComponent>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::addComponent<components::LoopSociety>(shape);
		middle::registerShape(gameState, shape);
		sphere->radius = middle::DEF_RADIUS;
		pos->pos = position;
		camera->targetX = target.x;
		camera->targetY = target.y;
		camera->targetZ = target.z;
		camera->upX = up.x;
		camera->upY = up.y;
		camera->upZ = up.z;
		camera->projection = projection;
		camera->fovy = fovy;
    }
}
