#pragma once
#include "editor_actions.h"
#include "middle_shape_utils.h"

namespace middle {
	template<typename CompType>
	CompType* attachComponent(middle::GameState* gameState, middle::Id id) {
		middle::Shape& shape = middle::getShape(gameState, id.index);
		auto newComp = middle::addComponent<CompType>(shape);
		gameState->componentTypeIdSetWithStructuralChanges.insert(middle::getTypeId<CompType>());
		return newComp;
	}

	template<typename CompType>
	void queueComponentAttachment(middle::GameState* gameState, middle::Id id) {
		middle::queueAction(gameState, std::make_shared<middle::CustomAction>([id](middle::GameState* gameState) {
			middle::Shape& shape = middle::getShape(gameState, id.index);
			middle::addComponent<CompType>(shape);
			gameState->componentTypeIdSetWithStructuralChanges.insert(middle::getTypeId<CompType>());
			}));
	}

	template<typename CompType, typename Init>
	void queueComponentAttachment(middle::GameState* gameState, middle::Id id, Init init) {
		middle::queueAction(gameState, std::make_shared<middle::CustomAction>([id, &init](middle::GameState* gameState) {
			middle::Shape& shape = middle::getShape(gameState, id.index);
			auto newComp = middle::addComponent<CompType>(shape);
			init(newComp);
			gameState->componentTypeIdSetWithStructuralChanges.insert(middle::getTypeId<CompType>());
			}));
	}

	template<typename CompType>
	void queueComponentDeletion(middle::GameState* gameState, middle::Id id) {
		middle::queueAction(gameState, std::make_shared<middle::CustomAction>([id](middle::GameState* gameState) {
			middle::Shape& shape = middle::getShape(gameState, id.index);
			if (!middle::getComponent<CompType>(shape)) {
				return;
			}
			middle::deleteComponent<CompType>(shape);
			gameState->componentTypeIdSetWithStructuralChanges.insert(middle::getTypeId<CompType>());
			}));
	}

}
