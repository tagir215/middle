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
	void notifyBubbleModification(middle::GameState* gameState, middle::Id resultId);


	class BubbleAction : public middle::EditorActionContainer {
	public:
		std::vector<middle::Id>inputs;
		std::vector<middle::Id>outputs;
	};

	class NotifyModificationAction : public middle::EditorActionContainer {
	public:
		std::shared_ptr<BubbleAction>action;
		NotifyModificationAction(std::shared_ptr<BubbleAction> bubbleAction) {
			this->action = bubbleAction;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Cancel : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_CANCEL
		};
		Cancel(middle::Id id) {
			this->inputs.push_back(id);
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Simplify : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_SIMPLIFY
		};
		Simplify(middle::Id id) {
			inputs.push_back(id);
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Bubblify : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_BUBBLIFY
		};
		Bubblify(middle::Id id) {
			inputs.push_back(id);
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class LinkMultiplicationTerm : public BubbleAction {
	public:
		enum InputRoles {
			ID_RECIEVER,
			ID_LINKING
		};
		enum OutputRoles {
			ID_RESULT_MULTIPLICATION
		};
		
		LinkMultiplicationTerm(middle::Id reciverShapeId, middle::Id linkikngShapeId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class UnlinkMultiplicationTerm : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_UNLINK
		};
		enum OutputRoles {
			ID_UNLINKED_MUL,
			ID_UNLINKED_SHAPE
		};
		UnlinkMultiplicationTerm(middle::Id unlinkingShapeId) {
			inputs.push_back(unlinkingShapeId);
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class MulOne : public BubbleAction {
	public:
		enum InputRoles {
			ID_RECIEVER
		};
		enum OutputRoles {
			ID_RESULT_MULTIPLICATION
		};
		MulOne(middle::Id recieverShapeId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class MulNegativeOne : public BubbleAction {
	public:
		enum InputRoles {
			ID_RECIEVER
		};
		enum OutputRoles {
			ID_RESULT_MULTIPLICATION
		};
		MulNegativeOne(middle::Id recieverShapeId) {
			inputs.push_back(recieverShapeId);
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
		enum InputRoles {
			ID_MUL
		};
		bool removedMulComp = false;
		UpdateBubblesMultiplicationIdentity(middle::Id mulId) {
			inputs.push_back(mulId);

		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class ExecuteMultiplication : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_COPY,
			ID_TO_COPY_INTO
		};
		enum OutputRoles {
			ID_RESULT_ADDITION
		};
		ExecuteMultiplication(middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class ExpandSummation : public BubbleAction {
	public:
		enum InputRoles {
			ID_SUMMATION
		};
		enum OutputRoles {
			ID_RESULT_ADDITION
		};
		ExpandSummation(middle::Id summationId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class ExecuteAddition : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_ADD,
			ID_TO_ADD_INTO
		};
		enum OutputRoles {
			ID_RESULT_ADDITION
		};
		ExecuteAddition(middle::Id shapeToAddId, middle::Id shapeToAddIntoId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class ExecutePower : public BubbleAction {
	public:
		enum InputRoles {
			ID_A,
			ID_B
		};
		enum OutputRoles {
			ID_RESULT_MULTIPLICATION
		};
		ExecutePower(middle::Id idA, middle::Id idB) {
			inputs.push_back(idA);
			inputs.push_back(idB);
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};


	class Pop : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_POP
		};
		enum OutputRoles {
			ID_RESULT_PARENT_OF_POPPED
		};
		Pop(middle::Id id);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Replace : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_REPLACE,
			ID_REPLACING
		};
		enum OutputRoles {
			ID_RESULT_REPLACEMENT
		};
		Replace(middle::Id shapeToReplace, middle::Id replacingShape);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class CompressCommonFactor : public BubbleAction {
	public:
		enum InputRoles {
			ID_COMMON_FACTOR
		};
		enum OutputRoles {
			ID_RESULT_COMPRESSED
		};
		CompressCommonFactor(middle::Id containerShape);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class CompressPowers : public BubbleAction {
	public:
		enum InputRoles {
			ID_COMMON_FACTOR
		};
		enum OutputRoles {
			ID_RESULT_COMPRESSED
		};
		CompressPowers(middle::Id commonExponentId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class NewAdditionTerm : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_ADD_INTO,
			ID_NEW_TERM
		};
		enum OutputRoles {
			ID_RESULT_CONTAINER
		};
		Vector3 targetPosition;
		NewAdditionTerm(middle::Id& shapeToAddIntoId, middle::Id& newTermId, const Vector3& targetPosition) {
			inputs.push_back(shapeToAddIntoId);
			inputs.push_back(newTermId);
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class NewMultiplicationTerm : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_ADD_INTO,
			ID_NEW_TERM
		};
		enum OutputRoles {
			ID_RESULT_CONTAINER
		};
		Vector3 targetPosition;
		NewMultiplicationTerm(middle::Id& shapeToAddIntoId, middle::Id& newTermId, const Vector3& targetPosition) {
			inputs.push_back(shapeToAddIntoId);
			inputs.push_back(newTermId);
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class NewPowerTerm : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_ADD_INTO,
			ID_NEW_TERM
		};
		enum OutputRoles {
			ID_RESULT_CONTAINER
		};
		Vector3 targetPosition;
		NewPowerTerm(middle::Id& shapeToAddIntoId, middle::Id& newTermId, const Vector3& targetPosition) {
			inputs.push_back(shapeToAddIntoId);
			inputs.push_back(newTermId);
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class InsertAsXOverX : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_ADD_INTO,
			ID_NEW_TERM
		};
		enum OutputRoles {
			ID_RESULT_CONTAINER
		};
		Vector3 targetPos;
		InsertAsXOverX(middle::Id shapeToAddIntoId, middle::Id newTermId, const Vector3& targetPosition) {
			inputs.push_back(shapeToAddIntoId);
			inputs.push_back(newTermId);
			this->targetPos = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class InsertAsXMinusX : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_ADD_INTO,
			ID_NEW_TERM
		};
		enum OutputRoles {
			ID_RESULT_CONTAINER
		};
		Vector3 targetPos;
		InsertAsXMinusX(middle::Id shapeToAddIntoId, middle::Id newTermId, const Vector3& targetPosition) {
			inputs.push_back(shapeToAddIntoId);
			inputs.push_back(newTermId);
			this->targetPos = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Substitute : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_SUBSTITUTE,
			ID_SUBSTITUTE
		};
		enum OutputRoles {
			ID_RESULT_SUBSTITUTE
		};
		Substitute(middle::Id shapeToReplaceId, middle::Id shapeToInsertId) {
			inputs.push_back(shapeToReplaceId);
			inputs.push_back(shapeToInsertId);
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class SubstituteFunction : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_SUBSTITUTE,
			ID_FUNCTION_BODY
		};
		enum OutputRoles {
			ID_RESULT_SUBSTITUTE
		};
		SubstituteFunction(middle::Id functionToReplaceId, middle::Id functionBodyId) {
			inputs.push_back(functionToReplaceId);
			inputs.push_back(functionBodyId);
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class CopyToInventory : public BubbleAction {
	public:
		enum InputRoles {
			ID_INVENTORY,
			ID_TO_COPY
		};
		enum OutputRoles {
			ID_RESULT_COPY
		};
		CopyToInventory(middle::Id inventoryId, middle::Id id) {
			inputs.push_back(inventoryId);
			inputs.push_back(id);
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};


	class CopyAsHelper : public BubbleAction {
	public:
		enum InputRoles {
			ID_TO_COPY,
		};
		enum OutputRoles {
			ID_RESULT_COPY_HELPER
		};
		Vector3 targetPosition;
		middle::Id copyShapeId;
		CopyAsHelper(middle::Id shapeToCopyId, const Vector3& targetPosition) {
			inputs.push_back(shapeToCopyId);
			this->targetPosition = targetPosition;
		}
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

}
