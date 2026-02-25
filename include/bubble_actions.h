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

	void deleteBubble(middle::GameState* gameState, middle::Id& id);
	void setBubbleHidden(middle::GameState* gameState, middle::Id& id, bool hidden);
	void updateVariable(middle::GameState* gameState, middle::Id& newUnitRef, const std::string& label);
	middle::Id inverseBubble(middle::GameState* gameState, middle::Id& id);
	middle::Id topLevelBubble(middle::GameState* gameState);
	middle::Shape& newBubble(middle::GameState* gameState, const Vector3& targetPos);
	middle::Shape& newUnit(middle::GameState* gameState, const Vector3& targetPos);
	middle::Shape& newFraction(middle::GameState* gameState, const Vector3& targetPos, int dividend);
	middle::Shape& newMultiplication(middle::GameState* gameState, middle::Id& idA, middle::Id& idB);
	bool isIntersecting(middle::GameState* gameState, middle::Shape& shape);
	bool equals(middle::GameState* gameState, middle::Id& bubbleA, middle::Id& bubbleB);
	float unitValue(middle::GameState* gameState, middle::Id& containerId);

	class CreateMulitiplicationReplacementShape : public middle::GameplayAction {
	public:
		middle::Id shapeToReplaceId;
		middle::Id replacingShapeId;
		middle::Id resultShapeId;
		CreateMulitiplicationReplacementShape(middle::Id shapeToReplace, middle::Id replacingShape);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};


	class CreateAdditionReplacementShape : public middle::GameplayAction {
	public:
		middle::Id idA;
		middle::Id idB;
		middle::Id resultId;
		CreateAdditionReplacementShape(middle::Id idA, middle::Id idB);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class LinkMultiplicationTerm : public middle::GameplayAction {
	public:
		middle::Id recieverShapeId;
		middle::Id linkingShapeId;
		middle::Id resultShapeId;
		LinkMultiplicationTerm(middle::Id reciverShapeId, middle::Id linkikngShapeId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class MulOne : public middle::GameplayAction {
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

	class ExecuteMultiplication : public middle::GameplayAction {
	public:
		middle::Id shapeToCopyId;
		middle::Id shapeToCopyIntoId;
		middle::Id resultShapeId;
		ExecuteMultiplication(middle::Id shapeToCopyId, middle::Id shapeToCopyIntoId);
		void execute(middle::GameState* gameState);
		void undo(middle::GameState* gameState) override;
	};


	class ExecuteAddition : public middle::GameplayAction {
	public:
		middle::Id shapeToAddId;
		middle::Id shapeToAddIntoId;
		middle::Id resultShapeId;
		ExecuteAddition(middle::Id shapeToAddId, middle::Id shapeToAddIntoId);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Pop : public middle::GameplayAction {
	public:
		middle::Id id;
		Pop(middle::Id id);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Replace : public middle::GameplayAction {
	public:
		middle::Id shapeToReplaceId;
		middle::Id replacingShapeId;
		Replace(middle::Id shapeToReplace, middle::Id replacingShape);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Break : public middle::GameplayAction {
	public:
		middle::Id containerShapeId;
		middle::Id resultShapeId;
		int dividend;
		Break(middle::Id containerShape, int dividend);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

	class Compress : public middle::GameplayAction {
	public:
		middle::Id containerShapeId;
		middle::Id resultShapeId;
		Compress(middle::Id containerShape);
		void execute(middle::GameState* gameState) override;
		void undo(middle::GameState* gameState) override;
	};

}
