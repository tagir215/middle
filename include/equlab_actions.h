#pragma once
#include "editor_actions.h"
#include "bubequ.h"

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

	class Negate : public middle::EditorActionContainer {
	public:
		middle::Id id;
		middle::Id resultId;
		Vector3 targetPosition;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		Negate(middle::Id id) {
			this->id = id;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class Invert : public middle::EditorActionContainer {
	public:
		middle::Id id;
		middle::Id resultId;
		Vector3 targetPosition;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		Invert(middle::Id id) {
			this->id = id;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class AddLabelCharacterToVariable : public middle::EditorActionContainer {
	public:
		middle::Id id;
		middle::Id resultId;
		std::string label;
		Vector3 targetPosition;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		AddLabelCharacterToVariable(middle::Id id, const std::string& label) {
			this->id = id;
			this->label = label;
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

	class AddEquals : public middle::EditorActionContainer {
	public:
		middle::Id resultId;
		Vector3 targetPos;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		AddEquals(const Vector3& targetPos) {
			this->targetPos = targetPos;
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

	class ToggleEditable : public middle::EditorActionContainer {
	public:
		middle::Id id;
		middle::Id topLevelId;
		ToggleEditable(middle::Id id) {
			this->id = id;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
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

	class ConnectPower : public middle::EditorActionContainer {
	public:
		middle::Id baseId;
		middle::Id exponentId;
		middle::Id resultId;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		ConnectPower(middle::Id bubbleIdA, middle::Id bubbleIdB) {
			this->baseId = bubbleIdA;
			this->exponentId = bubbleIdB;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	middle::Id bubequToBubble(middle::GameState* gameState, const Vector3& targetPos, std::shared_ptr<bubequ::Scope>& bubequ);
	std::string bubbleToBubequ(middle::GameState* gameState, middle::Id id);
}
