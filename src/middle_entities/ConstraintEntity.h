#pragma once
#include "middle_component_table.h"
#include "game_state.h"
#include "Constraint.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"

namespace entities{

    inline void initConstraint(middle::GameState* gameState, int index, int indexA, int indexB, float targetDistance){
		middle::Shape shape;
		components::Constraint* constraint = middle::addComponent<components::Constraint>(shape);
		middle::addComponent<components::MouseSelectable>(shape);
		middle::addComponent<components::MouseIntersectable>(shape);
		middle::registerShape(gameState, shape);
		constraint->stiffness = middle::DEF_STIFFNESS;
		constraint->targetDistance = targetDistance;
		middle::Shape& shapeA = middle::getShape(gameState, indexA);
		middle::Shape& shapeB = middle::getShape(gameState, indexB);
		constraint->idA = shapeA.id;
		constraint->idB = shapeB.id;
    }
}
