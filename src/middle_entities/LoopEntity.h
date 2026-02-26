#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "LoopTag.h"
#include "Position.h"
#include "editor_actions.h"

namespace entities{

    inline void initLoop(middle::GameState* gameState, int index, std::vector<middle::Id>loopIds, const Vector3& position){
		auto& shape = middle::addShape(gameState, index);
		middle::addComponent<components::LoopTag>(shape);
		auto loop = middle::addComponent<components::LoopSociety>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		auto pos = middle::addComponent<components::Position>(shape);
		pos->posX = position.x;
		pos->posY = position.y;
		pos->posZ = position.z;

		// assign parents the loop as parent to children
		for (middle::Id loopMember : loopIds) {
			auto& member = gameState->shapes[loopMember.index];
			assert(member.id == gameState->ids[loopMember.index]);
			auto memberLoop = middle::getComponent<components::LoopSociety>(member);
			auto reparentAction = middle::EditorActionReparent(shape.id.index, loopMember.index);
			reparentAction.execute(gameState);
		}
    }
}
