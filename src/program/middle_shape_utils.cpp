#include "middle_shape_utils.h"
#include "middle_math.h"
#include "middle_component_table.h"
#include "LoopSociety.h"
#include "Sphere.h"
#include "Reference.h"
#include "Position.h"
#include "Constraint.h"
#include "PhysicsData.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "JointEntity.h"
#include "LoopEntity.h"

namespace middle {


	std::vector<int>findConnectedConstraints(GameState* gameState, int id) {
		std::vector<int> result;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			Shape& shape = gameState->shapes[i];
			auto constraint = getComponent<components::Constraint>(shape);
			if (constraint == nullptr)
				continue;

			if (constraint->indexA == id || constraint->indexB == id)
				result.push_back(i);
		}
		return result;
	}

	bool constraintAlreadyExists(GameState* gameState, int indexA, int indexB) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			Shape& shape = gameState->shapes[i];
			auto constraint = getComponent<components::Constraint>(shape);
			if (constraint == nullptr)
				continue;

			if (constraint->indexA == indexA && constraint->indexB == indexB)
				return true;

			if (constraint->indexB == indexA && constraint->indexA == indexB)
				return true;
		}

		return false;
	}

	int findFreeIndex(GameState* gameState)
	{
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isShapeAlive(gameState, i))
				return i;
		}
		assert(true);
	}

	void unselect(GameState* gameState) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			auto& shape = gameState->shapes[i];
			auto selectableComponent = getComponent<components::MouseSelectable>(shape);
			if (selectableComponent) {
				selectableComponent->selected = false;
			}
		}
	}

	int findHighestLevelContainer(GameState* gameState, int index)
	{
		if(!isShapeAlive(gameState, index))
			return UNASSIGNED;
		Shape& shape = gameState->shapes[index];
		auto loop = getComponent<components::LoopSociety>(shape);
		if (loop->parentLoopIndex == UNASSIGNED)
			return index;

		return findHighestLevelContainer(gameState, loop->parentLoopIndex);
	}

	int findHighestUsedIndex(GameState* gameState)
	{
		int highestI = 0;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (isShapeAlive(gameState, i)) {
				highestI = i;
			}
		}
		return highestI;
	}

	int findNextFreeGhostIndex(GameState* gameState)
	{
		int highestUsed = findHighestUsedIndex(gameState) + 1;
		return highestUsed > GHOST_INDEX_OFFSET ? highestUsed : GHOST_INDEX_OFFSET;
	}

	void dragShape(GameState* gameState, int index, Vector3 linearVelocity) {
		Shape& shape = getShape(gameState, index);
		auto loop = getComponent<components::LoopSociety>(shape);
		if (loop != nullptr) {
			for (int i = 0; i < loop->loopMemberIndexes.size(); ++i) {
				int memberIndex = loop->loopMemberIndexes[i];
				dragShape(gameState, memberIndex, linearVelocity);
			}
		}

		// don't move loops, we can move references though
		//if (instance.shape.type == ShapeType::LOOP)
		//	return;

		Vec linearVel = DescVec(linearVelocity);
		auto pData = getComponent<components::PhysicsData>(shape);
		auto posData = getComponent<components::Position>(shape);
		if(pData != nullptr){
			pData->velX = linearVel.x;
			pData->velY = linearVel.y;
			pData->velZ = linearVel.z;
		}
		else if(posData){
			Vec currPos = { posData->posX, posData->posY, posData->posZ };
			Vec newPos = AddV(currPos, ScaleV(linearVel, gameState->frameTime));
			posData->posX = newPos.x;
			posData->posY = newPos.y;
			posData->posZ = newPos.z;
		}
	}

	void moveShape(GameState* gameState, int index, const Vector3& displacement)
	{
		Shape& shape = gameState->shapes[index];
		auto loop = getComponent<components::LoopSociety>(shape);
		if (loop != nullptr) {
			for (int i = 0; i < loop->loopMemberIndexes.size(); ++i) {
				int memberIndex = loop->loopMemberIndexes[i];
				assert(index != memberIndex);
				moveShape(gameState, memberIndex, displacement);
			}
		}

		auto pos = getComponent<components::Position>(shape);
		if (pos) {
			pos->posX += displacement.x;
			pos->posY += displacement.y;
			pos->posZ += displacement.z;
		}
	}

	bool isGhostShape(int index)
	{
		return index >= GHOST_INDEX_OFFSET;
	}

	bool isEntityOfType(GameState* gameState, int index, const std::vector<int>& entity)
	{
		auto& shape = getShape(gameState, index);
		if (shape.componentMap.size() != entity.size()) {
			return false;
		}
		for (int componentTypeId : entity) {
			if (shape.componentMap.find(componentTypeId) == shape.componentMap.end()) {
				return false;
			}
		}
		return true;
	}

	bool isShapeSelected(GameState* gameState, int index) {
		auto& shape = gameState->shapes[index];
		auto selectedComponent = getComponent<components::MouseSelectable>(shape);
		if (selectedComponent) {
			return selectedComponent->selected;
		}
		return false;
	}

	bool isMouseIntersectingShape(GameState* gameState, int index)
	{
		auto& shape = gameState->shapes[index];
		auto intersectable = getComponent<components::MouseIntersectable>(shape);
		if (intersectable) {
			return intersectable->intersecting;
		}
		return false;
	}

	bool isShapeAlive(GameState* gameState, int index) {
		return gameState->shapes[index].id == gameState->ids[index] && gameState->shapes[index].id.generation >= 0;
	}

	Vector3 getShapePosition(GameState* gameState, int index)
	{
		auto& shape = getShape(gameState, index);
		auto position = getComponent<components::Position>(shape);
		assert(position);
		return { position->posX, position->posY, position->posZ };
	}

	Vector3 getLoopCentroid(GameState* gameState, int index)
	{
		auto& shape = getShape(gameState, index);
		auto loop = getComponent<components::LoopSociety>(shape);
		Vector3 centroid = { 0,0,0 };
		if (loop->loopMemberIndexes.size() < 1)
			return centroid;

		for (int childIndex : loop->loopMemberIndexes) {
			auto& child = getShape(gameState, childIndex);
			if (getComponent<components::LoopTag>(child)) {
				centroid += getLoopCentroid(gameState, childIndex);
			}
			else {
				centroid += getShapePosition(gameState, childIndex);
			}
		}

		centroid = centroid * (1.0f / loop->loopMemberIndexes.size());
		return centroid;
	}

	Shape& getShape(GameState* gameState, int index)
	{
		if (gameState->shapes[index].id == gameState->ids[index]) {
			return gameState->shapes[index];
		}
		assert(false);
	}

	void deleteShape(GameState* gameState, int index) {
		int prevGeneration = gameState->shapes[index].id.generation;
		gameState->shapes[index] = Shape();
		gameState->shapes[index].id.generation = prevGeneration + 1;
	}

	void deleteShapeRecursive(GameState* gameState, int index) {
		Shape& shape = gameState->shapes[index];
		auto loop = getComponent<components::LoopSociety>(shape);
		if (loop) {
			for (int childIndex : loop->loopMemberIndexes) {
				deleteShapeRecursive(gameState, childIndex);
			}
		}
		std::vector<int> connectedConstraints = findConnectedConstraints(gameState, index);
		for (int connectedIndex : connectedConstraints) {
			deleteShape(gameState, connectedIndex);
		}
		deleteShape(gameState, index);
	}

	void addShape(GameState* gameState, int index, Shape shape) {
		shape.id.generation = gameState->shapes[index].id.generation + 1;
		gameState->ids[index] = shape.id;
		gameState->shapes[index] = shape;
	}

	void moveCameraXZ(Camera3D& initCamera, const Vector3& pos)
	{
		Vector3 displacement = pos - initCamera.position;
		initCamera.position += displacement;
		initCamera.target += displacement;
	}
}
