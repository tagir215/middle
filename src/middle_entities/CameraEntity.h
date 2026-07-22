#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "LocalPosition.h"
#include "LocalScale.h"
#include "GlobalTransform.h"
#include "Sphere.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "MouseGrabbable.h"
#include "PhysicsData.h"
#include "middle_shape_utils.h"
#include "CameraComponent.h"

namespace entities{

    void initCamera(middle::GameState* gameState, int index, const Vector3& position, const Vector3& up, const Vector3& target, float fovy, int projection){
		middle::Shape shape;
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
