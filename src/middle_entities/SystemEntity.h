#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "MouseGrabbable.h"
#include "SystemReference.h"
#include "LocalPosition.h"
#include "LocalScale.h"
#include "GlobalTransform.h"
#include "EditorText.h"

namespace entities{

    inline void initSystem(middle::GameState* gameState, int index, const Vector3& position, const std::string& systemName){
		middle::Shape shape;
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
