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
#include "middle_shape_utils.h"
#include "CameraComponent.h"

namespace entities{

    void initCamera(middle::GameState* gameState, int index, const Vector3& position, const Vector3& up, const Vector3& target, float fovy, int projection){
		auto& shape = middle::addShape(gameState,index);
		components::Sphere* sphere = middle::addComponent<components::Sphere>(shape);
		components::Position* pos = middle::addComponent<components::Position>(shape);
		components::CameraComponent* camera = middle::addComponent<components::CameraComponent>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::addComponent<components::LoopSociety>(shape);
		sphere->radius = middle::DEF_RADIUS;
		pos->posX = position.x;
		pos->posY = position.y;
		pos->posZ = position.z;
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
