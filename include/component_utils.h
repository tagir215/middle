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

	template<typename T>
	inline T* getComp(middle::GameState* gameState, middle::Id id) {
		auto& shape = middle::getShape(gameState, id.index);
		return middle::getComponent<T>(shape);
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

	template<typename T>
	class AttachComponentAction : public middle::EditorActionContainer {
	public:
		middle::Id id;
		T* resultComp;
		AttachComponentAction(middle::Id id) {
			this->id = id;
		}
		void execute(middle::GameState* gameState) override {
			auto newComp = middle::attachComponent<T>(gameState, id);
			resultComp = newComp;
		}
		void undo(middle::GameState* gameState) override {
			middle::queueComponentDeletion<T>(gameState, id);
		}
	};
}
