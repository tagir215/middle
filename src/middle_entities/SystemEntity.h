#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "Position.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "LoopSociety.h"
#include "MouseGrabbable.h"
#include "SystemReference.h"
#include "Text.h"

namespace entities{

    inline void initSystem(middle::GameState* gameState, int index, const Vector3& position, const std::string& systemName){
		middle::Shape shape;
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseGrabbable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::addComponent<components::LoopSociety>(shape);
		auto system = middle::addComponent<components::SystemReference>(shape);
		auto pos = middle::addComponent<components::Position>(shape);
		auto text = middle::addComponent<components::Text>(shape);
		middle::registerShape(gameState, shape);
		system->systemName = systemName;
		pos->posX = position.x;
		pos->posY = position.y;
		pos->posZ = position.z;
		text->fontColorR = middle::REF_TEXT_COLOR.a;
		text->fontColorG = middle::REF_TEXT_COLOR.a;
		text->fontColorB = middle::REF_TEXT_COLOR.a;
		text->fontColorA = middle::REF_TEXT_COLOR.a;
		text->offsetX = 2;
		text->offsetY = 0;
		text->offsetZ = 10;
		text->text = systemName;
		text->fontSize = middle::REF_TEXT_SIZE;
    }
}
