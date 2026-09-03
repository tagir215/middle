#pragma once
#include "editor_actions.h"
#include "bubequ.h"

namespace equlab {

	class AddBubble : public middle::EditorActionContainer {
	public:
		middle::Id parentId;
		middle::Id resultId;
		Vector3 targetPosition;
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
		AddLabelCharacterToVariable(middle::Id id, const std::string& label) {
			this->id = id;
			this->label = label;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class AddLabelToFunction : public middle::EditorActionContainer {
	public:
		middle::Id id;
		middle::Id resultId;
		std::string label;
		AddLabelToFunction(middle::Id id, const std::string& label) {
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
		middle::Id parentId;
		AddEquals(middle::Id parentId, const Vector3& targetPos) {
			this->parentId = parentId;
			this->targetPos = targetPos;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class AddInequals : public middle::EditorActionContainer {
	public:
		middle::Id resultId;
		bool equalOr;
		middle::Id parentId;
		Vector3 targetPos;
		AddInequals(middle::Id parentId, const Vector3& targetPos, bool equalOr) {
			this->parentId = parentId;
			this->targetPos = targetPos;
			this->equalOr = equalOr;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class AddSummation : public middle::EditorActionContainer {
	public:
		middle::Id resultId;
		middle::Id parentId;
		Vector3 targetPos;
		AddSummation(middle::Id parentId, const Vector3& targetPos) {
			this->parentId = parentId;
			this->targetPos = targetPos;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class Move : public middle::EditorActionContainer {
	public:
		middle::Id id;
		Vector3 targetPosition;
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
		ConnectPower(middle::Id bubbleIdA, middle::Id bubbleIdB) {
			this->baseId = bubbleIdA;
			this->exponentId = bubbleIdB;
		}
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState);
	};

	class FreeParent : public middle::EditorActionContainer {
	public:
		middle::Id id;
		FreeParent(middle::Id id) {
			this->id = id;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class LoadParent : public middle::EditorActionContainer {
	public:
		middle::Id id;
		LoadParent(middle::Id id) {
			this->id = id;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};
}
