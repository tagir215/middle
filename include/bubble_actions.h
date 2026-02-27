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


namespace bubbleActions{

	void setBubbleHidden(middle::GameState* gameState, middle::Id& id, bool hidden);
	middle::Id inverseBubble(middle::GameState* gameState, middle::Id& id);
	middle::Id topLevelBubble(middle::GameState* gameState);
	middle::Shape& newBubble(middle::GameState* gameState, const Vector3& targetPos);
	middle::Shape& newUnit(middle::GameState* gameState, const Vector3& targetPos);
	middle::Shape& newFraction(middle::GameState* gameState, const Vector3& targetPos, int dividend);
	middle::Shape& shapeToFraction(middle::GameState* gameState, middle::Id shpaeId, const Vector3& targetPos, int dividend);
	middle::Shape& newMultiplication(middle::GameState* gameState, middle::Id& idA, middle::Id& idB);
	bool isIntersecting(middle::GameState* gameState, middle::Shape& shape);
	bool equals(middle::GameState* gameState, middle::Id& bubbleA, middle::Id& bubbleB);
	float unitValue(middle::GameState* gameState, middle::Id& containerId);
	int fractionUnitCount(middle::GameState* gameState, middle::Id& fractionId);

	class UpdateVariable : public middle::EditorActionContainer {
	public:
		std::string label;
		middle::Id newUnitRef;
		middle::Id oldUnitRef;
		UpdateVariable(std::string& label, middle::Id& newUnitRef);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class CreateMulitiplicationReplacementShape : public middle::EditorActionContainer {
	public:
		middle::Id shapeToReplaceId;
		middle::Id replacingShapeId;
		middle::Id resultShapeId;
		CreateMulitiplicationReplacementShape(middle::Id shapeToReplace, middle::Id replacingShape);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};


	class CreateAdditionReplacementShape : public middle::EditorActionContainer {
	public:
		middle::Id idA;
		middle::Id idB;
		middle::Id resultId;
		CreateAdditionReplacementShape(middle::Id idA, middle::Id idB);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class LinkMultiplicationTerm : public middle::EditorActionContainer {
	public:
		middle::Id recieverShapeId;
		middle::Id linkingShapeId;
		middle::Id resultShapeId;
		LinkMultiplicationTerm(middle::Id reciverShapeId, middle::Id linkikngShapeId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class MulOne : public middle::EditorActionContainer {
	public:
		middle::Id recieverShapeId;
		middle::Id resultShapeId;
		MulOne(middle::Id recieverShapeId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};


	struct MultiplyPair {
		middle::Id parentId;
		middle::Id idA;
		middle::Id idB;
	};

	class ExecuteMultiplication : public middle::EditorActionContainer {
	public:
		middle::Id mulShapeId;
		middle::Id shapeToCopyId;
		middle::Id shapeToCopyIntoId;
		middle::Id resultShapeId;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		ExecuteMultiplication(middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId);
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState) override;
	};


	class ExecuteAddition : public middle::EditorActionContainer {
	public:
		middle::Id shapeToAddId;
		middle::Id shapeToAddIntoId;
		middle::Id resultShapeId;
		ExecuteAddition(middle::Id shapeToAddId, middle::Id shapeToAddIntoId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Pop : public middle::EditorActionContainer {
	public:
		middle::Id id;
		Pop(middle::Id id);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Replace : public middle::EditorActionContainer {
	public:
		middle::Id shapeToReplaceId;
		middle::Id replacingShapeId;
		std::vector<std::unique_ptr<middle::EditorActionContainer>> actions;
		Replace(middle::Id shapeToReplace, middle::Id replacingShape);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Break : public middle::EditorActionContainer {
	public:
		middle::Id containerShapeId;
		middle::Id resultShapeId;
		int dividend;
		Break(middle::Id containerShape, int dividend);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Compress : public middle::EditorActionContainer {
	public:
		middle::Id containerShapeId;
		middle::Id resultCompressedBubbleId;
		middle::Id resultCountBubbleId;
		Compress(middle::Id containerShape);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

}
