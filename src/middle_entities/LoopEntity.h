#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"

namespace entities{

	inline std::vector<int>LoopEntity{
		middle::getTypeId<components::MouseSelectable>(),
		middle::getTypeId<components::MouseIntersectable>(),
		middle::getTypeId<components::LoopSociety>(),
		middle::getTypeId<components::MouseGrabbable>(),
    };

    inline void initLoop(middle::GameState* gameState, int index, std::vector<int>loopIndexes){
		auto& shape = gameState->shapes[index];
		++shape.id.generation;
		gameState->ids[index].generation = shape.id.generation;
		auto loop = middle::addComponent<components::LoopSociety>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		loop->loopMemberIndexes = loopIndexes;
    }
}
