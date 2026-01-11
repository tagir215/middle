#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "LoopTag.h"

namespace entities{

    inline void initLoop(middle::GameState* gameState, int index, std::vector<int>loopIndexes){
		auto& shape = gameState->shapes[index];
		++shape.id.generation;
		gameState->ids[index].generation = shape.id.generation;
		middle::addComponent<components::LoopTag>(shape);
		auto loop = middle::addComponent<components::LoopSociety>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		loop->loopMemberIndexes = loopIndexes;

		// assign parents the loop as parent to children
		for (int loopMember : loopIndexes) {
			auto& member = gameState->shapes[loopMember];
			assert(member.id == gameState->ids[loopMember]);
			auto memberLoop = middle::getComponentAssert<components::LoopSociety>(member);
			memberLoop->parentLoopIndex = index;
		}
    }
}
