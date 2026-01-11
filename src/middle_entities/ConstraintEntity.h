#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "Constraint.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"

namespace entities{

    inline std::vector<int>ConstraintEntity{
        middle::getTypeId<components::Constraint>(),
        middle::getTypeId<components::MouseSelectable>(),
        middle::getTypeId<components::MouseIntersectable>(),
    };

    inline void initConstraint(middle::GameState* gameState, int index, int indexA, int indexB, float targetDistance){
		auto& shape = gameState->shapes[index];
		++shape.id.generation;
		gameState->ids[index].generation = shape.id.generation;
		components::Constraint* constraint = middle::addComponent<components::Constraint>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		constraint->stiffness = middle::DEF_STIFFNESS;
		constraint->targetDistance = targetDistance;
		constraint->indexA = indexA;
		constraint->indexB = indexB;
    }
}
