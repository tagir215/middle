#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "Position.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "MouseGrabbable.h"
#include "SystemReference.h"

namespace entities{

    inline void initSystem(middle::GameState* gameState, int index, const Vector3& position, const std::string& systemName){
		auto& shape = gameState->shapes[index];
		++shape.id.generation;
		gameState->ids[index].generation = shape.id.generation;
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		auto system = middle::addComponent<components::SystemReference>(shape);
		auto pos = middle::addComponent<components::Position>(shape);
		system->systemName = systemName;
		pos->posX = position.x;
		pos->posY = position.y;
		pos->posZ = position.z;
    }
}
