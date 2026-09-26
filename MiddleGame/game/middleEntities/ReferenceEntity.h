#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "MidComp/LocalPosition.h"
#include "MidComp/GlobalTransform.h"
#include "MidComp/LocalScale.h"
#include "MidComp/MouseSelectable.h"
#include "MidComp/MouseIntersectable.h"
#include "MidComp/LoopSociety.h"
#include "MidComp/MouseGrabbable.h"
#include "MidComp/Reference.h"

namespace entities{

    void initReference(middle::GameState* gameState, int index, std::vector<middle::Id>members, const std::string& folder, const std::string& sceneName){
		middle::Shape shape = middle::createShape(gameState);
		components::LocalPosition* pos = middle::addComponent<components::LocalPosition>(shape);
		middle::addComponent<components::GlobalTransform>(shape);
		middle::addComponent<components::LocalScale>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		auto reference = middle::addComponent<components::Reference>(shape);
		auto loop = middle::addComponent<components::LoopSociety>(shape);
		middle::registerShapeAtIndex(gameState, shape, index);
		reference->sceneName = sceneName;
		reference->folder = folder;
		loop->loopMemberIds = members;
		midMath::Vector3 targetPos = { 0,0,0 };
		if (members.size() == 1) {
			targetPos = middle::getGlobalPosition(gameState, members[0]);
		}
		pos->pos = targetPos;
    }
}
