#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "MidComp/MouseSelectable.h"
#include "MidComp/MouseIntersectable.h"
#include "MidComp/LoopSociety.h"
#include "MidComp/LoopTag.h"
#include "editor_actions.h"
#include "MidComp/LocalPosition.h"
#include "MidComp/GlobalTransform.h"
#include "MidComp/LocalScale.h"

namespace entities{

    inline void initLoop(middle::GameState* gameState, int index, std::vector<middle::Id>loopIds, const midMath::Vector3& position){
		middle::Shape shape = middle::createShape(gameState);
		middle::addComponent<components::LoopTag>(shape);
		auto loop = middle::addComponent<components::LoopSociety>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		middle::addComponent<components::GlobalTransform>(shape);
		middle::addComponent<components::LocalScale>(shape);
		auto pos = middle::addComponent<components::LocalPosition>(shape);
		middle::registerShape(gameState, shape);
		pos->pos = position;

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
