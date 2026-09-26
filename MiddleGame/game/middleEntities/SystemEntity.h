#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "MidComp/MouseSelectable.h"
#include "MidComp/MouseIntersectable.h"
#include "MidComp/LoopSociety.h"
#include "MidComp/MouseGrabbable.h"
#include "MidComp/SystemReference.h"
#include "MidComp/LocalPosition.h"
#include "MidComp/LocalScale.h"
#include "MidComp/GlobalTransform.h"
#include "MidComp/EditorText.h"
#include "middle_math.h"

namespace entities{

    inline void initSystem(middle::GameState* gameState, int index, const midMath::Vector3& position, const std::string& systemName){
		middle::Shape shape = middle::createShape(gameState);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::addComponent<components::LoopSociety>(shape);
		auto system = middle::addComponent<components::SystemReference>(shape);
		auto text = middle::addComponent<components::EditorText>(shape);
		auto pos = middle::addComponent<components::LocalPosition>(shape);
		middle::addComponent<components::GlobalTransform>(shape);
		middle::addComponent<components::LocalScale>(shape);
		middle::registerShape(gameState, shape);
		system->systemName = systemName;
		pos->pos = position;
		text->text = systemName;
    }
}
