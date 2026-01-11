#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "Position.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "MouseGrabbable.h"
#include "Reference.h"

namespace entities{

    void initReference(middle::GameState* gameState, int index, std::vector<int>members, const std::string& sceneName){
		auto& shape = gameState->shapes[index];
		++shape.id.generation;
		gameState->ids[index].generation = shape.id.generation;
		components::Position* pos = middle::addComponent<components::Position>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		auto reference = middle::addComponent<components::Reference>(shape);
		auto loop = middle::addComponent<components::LoopSociety>(shape);
		reference->sceneName = sceneName;
		loop->loopMemberIndexes = members;
		pos->posX = 0;
		pos->posY = 0;
		pos->posZ = 0;
    }
}
