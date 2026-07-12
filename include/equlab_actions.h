#pragma once
#include "editor_actions.h"

namespace equlab {

	class AddBubble : public middle::EditorActionContainer {
	public:
		middle::Id parentId;
		middle::Id resultId;
		Vector3 targetPosition;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		AddBubble(middle::Id parentId, const Vector3& targetPosition) {
			this->parentId = parentId;
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class AddUnit : public middle::EditorActionContainer {
	public:
		middle::Id parentId;
		middle::Id resultId;
		Vector3 targetPosition;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		AddUnit(middle::Id parentId, const Vector3& targetPosition) {
			this->parentId = parentId;
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class AddVariable : public middle::EditorActionContainer {
	public:
		middle::Id parentId;
		middle::Id resultId;
		std::string label;
		Vector3 targetPosition;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		AddVariable(middle::Id parentId, const std::string& label, const Vector3& targetPosition) {
			this->parentId = parentId;
			this->targetPosition = targetPosition;
			this->label = label;

		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class Move : public middle::EditorActionContainer {
	public:
		middle::Id id;
		Vector3 targetPosition;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		Move(middle::Id id, const Vector3& targetPosition) {
			this->id = id;
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class Delete : public middle::EditorActionContainer {
	public:
		middle::Id id;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		Delete(middle::Id id) {
			this->id = id;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class ConnectEqualsLink : public middle::EditorActionContainer {
	public:
		middle::Id bubbleIdA;
		middle::Id bubbleIdB;
		middle::Id resultId;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		ConnectEqualsLink(middle::Id bubbleIdA, middle::Id bubbleIdB) {
			this->bubbleIdA = bubbleIdA;
			this->bubbleIdB = bubbleIdB;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class ConnectOperationLink : public middle::EditorActionContainer {
	public:
		middle::Id idA;
		middle::Id idB;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		middle::Id resultId;
		ConnectOperationLink(const middle::Id& idA, const middle::Id& idB) {
			this->idA = idA;
			this->idB = idB;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class ConnectMultiplicationLink : public middle::EditorActionContainer {
	public:
		middle::Id bubbleIdA;
		middle::Id bubbleIdB;
		middle::Id resultId;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		ConnectMultiplicationLink(middle::Id bubbleIdA, middle::Id bubbleIdB) {
			this->bubbleIdA = bubbleIdA;
			this->bubbleIdB = bubbleIdB;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class ConnectPowerLink : public middle::EditorActionContainer {
	public:
		middle::Id bubbleIdA;
		middle::Id bubbleIdB;
		middle::Id resultId;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		ConnectPowerLink(middle::Id bubbleIdA, middle::Id bubbleIdB) {
			this->bubbleIdA = bubbleIdA;
			this->bubbleIdB = bubbleIdB;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

}
