#pragma once
#include "game_state.h"
#include "middle_shape_utils.h"
#include "BubbleComponent.h"
#include "LoopSociety.h"
#include "MouseIntersectable.h"
#include "Position.h"
#include "BubbleMultiplyComponent.h"
#include "editor_actions.h"
#include "FractionalComponent.h"
#include "MouseGrabbable.h"
#include "BubbleUnit.h"
#include "LoopTag.h"
#include "Sphere.h"
#include "Text.h"
#include "InputVariable.h"
#include "AlgebraNode.h"
#include "component_utils.h"
#include "ModifiedBubbleTag.h"

namespace bubbleActions{

	middle::Id createNegatedReplacementShape(middle::GameState* gameState, middle::Id id);
	middle::Id createInverseReplacementShape(middle::GameState* gameState, middle::Id id);
	middle::Id createMultiplicationReplacementShape(middle::GameState* gameState, middle::Id shapeToReplace, middle::Id replacingShape);
	middle::Id createAdditionReplacementShape(middle::GameState* gameState, middle::Id shapeToReplace, middle::Id replacingShape);
	middle::Id createMultiplicationIntoPowerReplacementShape(middle::GameState* gameState, middle::Id shapeToReplace, middle::Id powerBubbleId);

	class BubbleAction : public middle::EditorActionContainer {
	public:
		middle::Id resultId;

		void notifyBubbleModification(middle::GameState* gameState, middle::Id id)
		{
			assert(middle::isValidId(gameState, id));
			middle::attachComponent<components::ModifiedBubbleTag>(gameState, id);
		}
	};

	class Cancel : public BubbleAction {
	public:
		middle::Id id;
		middle::Id resultId;
		Cancel(middle::Id id) {
			this->id = id;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Simplify : public BubbleAction {
	public:
		middle::Id id;
		middle::Id resultId;
		Simplify(middle::Id id) {
			this->id = id;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Bubblify : public BubbleAction {
	public:
		middle::Id id;
		middle::Id resultId;
		Bubblify(middle::Id id) {
			this->id = id;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class UpdateVariable : public BubbleAction {
	public:
		std::string label;
		std::function<middle::Id()>newUnitRefProvider;
		middle::Id oldUnitRef;
		UpdateVariable(std::string label, std::function<middle::Id()> newUnitrefProvider);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class LinkMultiplicationTerm : public BubbleAction {
	public:
		middle::Id recieverShapeId;
		middle::Id linkingShapeId;
		middle::Id resultShapeId;
		LinkMultiplicationTerm(middle::Id reciverShapeId, middle::Id linkikngShapeId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class UnlinkMultiplicationTerm : public BubbleAction {
	public:
		middle::Id unlinkingShapeId;
		middle::Id resultUnlinkedMulId;
		middle::Id resultUnlinkedId;
		UnlinkMultiplicationTerm(middle::Id unlinkingShapeId) {
			this->unlinkingShapeId = unlinkingShapeId;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class MulOne : public BubbleAction {
	public:
		middle::Id recieverShapeId;
		middle::Id resultShapeId;
		MulOne(middle::Id recieverShapeId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class MulNegativeOne : public BubbleAction {
	public:
		middle::Id recieverShapeId;
		middle::Id resultShapeId;
		MulNegativeOne(middle::Id recieverShapeId) {
			this->recieverShapeId = recieverShapeId;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};


	struct MultiplyPair {
		middle::Id parentId;
		middle::Id idA;
		middle::Id idB;
	};

	class UpdateBubblesMultiplicationIdentity : public BubbleAction {
	public:
		middle::Id mulId;
		bool removedMulComp = false;
		UpdateBubblesMultiplicationIdentity(middle::Id mulId) {
			this->mulId = mulId;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class ExecuteMultiplication : public BubbleAction {
	public:
		middle::Id shapeToCopyId;
		middle::Id shapeToCopyIntoId;
		middle::Id resultShapeId;
		ExecuteMultiplication(middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class ExpandSummation : public BubbleAction {
	public:
		middle::Id summationId;
		middle::Id resultShapeId;
		ExpandSummation(middle::Id summationId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class ExecuteAddition : public BubbleAction {
	public:
		middle::Id shapeToAddId;
		middle::Id shapeToAddIntoId;
		middle::Id resultShapeId;
		ExecuteAddition(middle::Id shapeToAddId, middle::Id shapeToAddIntoId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class ExecutePower : public BubbleAction {
	public:
		middle::Id idA;
		middle::Id idB;
		middle::Id resultShapeId;
		ExecutePower(middle::Id idA, middle::Id idB) {
			this->idA = idA;
			this->idB = idB;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};


	class Pop : public BubbleAction {
	public:
		middle::Id id;
		Pop(middle::Id id);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Replace : public BubbleAction {
	public:
		middle::Id shapeToReplaceId;
		middle::Id replacingShapeId;
		Replace(middle::Id shapeToReplace, middle::Id replacingShape);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Break : public BubbleAction {
	public:
		middle::Id unitShapeId;
		middle::Id resultShapeId;
		int dividend;
		Break(middle::Id containerShape, int dividend);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};


	class CompressCommonFactor : public BubbleAction {
	public:
		middle::Id commonFactorId;
		middle::Id resultShapeId;
		CompressCommonFactor(middle::Id containerShape);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class CompressPowers : public BubbleAction {
	public:
		middle::Id commonFactorId;
		middle::Id resultShapeId;
		CompressPowers(middle::Id commonExponentId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class NewAdditionTerm : public BubbleAction {
	public:
		middle::Id shapeToAddIntoId;
		middle::Id newTermId;
		Vector3 targetPosition;
		NewAdditionTerm(middle::Id& shapeToAddIntoId, middle::Id& newTermId, const Vector3& targetPosition) {
			this->shapeToAddIntoId = shapeToAddIntoId;
			this->newTermId = newTermId;
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class NewMultiplicationTerm : public BubbleAction {
	public:
		middle::Id shapeToAddIntoId;
		middle::Id newTermId;
		Vector3 targetPosition;
		NewMultiplicationTerm(middle::Id& shapeToAddIntoId, middle::Id& newTermId, const Vector3& targetPosition) {
			this->shapeToAddIntoId = shapeToAddIntoId;
			this->newTermId = newTermId;
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class NewPowerTerm : public BubbleAction {
	public:
		middle::Id shapeToAddIntoId;
		middle::Id newTermId;
		Vector3 targetPosition;
		NewPowerTerm(middle::Id& shapeToAddIntoId, middle::Id& newTermId, const Vector3& targetPosition) {
			this->shapeToAddIntoId = shapeToAddIntoId;
			this->newTermId = newTermId;
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class InsertAsXOverX : public BubbleAction {
	public:
		middle::Id shapeToAddIntoId;
		middle::Id newTermId;
		Vector3 targetPos;
		InsertAsXOverX(middle::Id shapeToAddIntoId, middle::Id newTermId, const Vector3& targetPosition) {
			this->shapeToAddIntoId = shapeToAddIntoId;
			this->newTermId = newTermId;
			this->targetPos = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class InsertAsXMinusX : public BubbleAction {
	public:
		middle::Id shapeToAddIntoId;
		middle::Id newTermId;
		Vector3 targetPos;
		InsertAsXMinusX(middle::Id shapeToAddIntoId, middle::Id newTermId, const Vector3& targetPosition) {
			this->shapeToAddIntoId = shapeToAddIntoId;
			this->newTermId = newTermId;
			this->targetPos = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class StartProcedure : public BubbleAction {
	public:
		middle::Id procContainer;
		middle::Id input;
		StartProcedure(middle::Id procContainerId, middle::Id inputId) {
			this->procContainer = procContainerId;
			this->input = inputId;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};


	class Substitute : public BubbleAction {
	public:
		middle::Id shapeToReplaceId;
		middle::Id shapeToInsertId;
		Substitute(middle::Id shapeToReplaceId, middle::Id shapeToInsertId) {
			this->shapeToReplaceId = shapeToReplaceId;
			this->shapeToInsertId = shapeToInsertId;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class SubstituteFunction : public BubbleAction {
	public:
		middle::Id functionToReplaceId;
		middle::Id functionBodyId;
		SubstituteFunction(middle::Id functionToReplaceId, middle::Id functionBodyId) {
			this->functionToReplaceId = functionToReplaceId;
			this->functionBodyId = functionBodyId;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class CopyToInventory : public BubbleAction {
	public:
		middle::Id inventoryId;
		middle::Id id;
		middle::Id resultId;
		CopyToInventory(middle::Id inventoryId, middle::Id id) {
			this->inventoryId = inventoryId;
			this->id = id;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};


	class CopyAsHelper : public BubbleAction {
	public:
		middle::Id shapeToCopyId;
		Vector3 targetPosition;
		middle::Id copyShapeId;
		CopyAsHelper(middle::Id shapeToCopyId, const Vector3& targetPosition) {
			this->shapeToCopyId = shapeToCopyId;
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

}
