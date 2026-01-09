#include "middle_shape_utils.h"
#include "middle_math.h"
#include "middle_component_table.h"
#include "LoopSociety.h"
#include "Sphere.h"
#include "Reference.h"
#include "Position.h"
#include "Constraint.h"
#include "PhysicsData.h"

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
			if (isSlotFree(gameState, i))
				return i;
		}
		assert(true);
	}

	void updateLoop(GameState* gameState, int id) {
		Shape& loopShape = gameState->shapes[id];
		auto loop = getComponent<components::LoopSociety>(loopShape);
		assert(loop != nullptr);

		std::vector<int>members;
		// store still valid members
		for (int i = loop->loopArrayOffset; i < loop->loopArrayOffset + loop->loopSize; ++i) {
			int memberIndex = gameState->loopMembers[i];
			Shape& shape = gameState->shapes[memberIndex];
			if (isShapeAlive(gameState, memberIndex)) {
				members.push_back(memberIndex);
			}
		}
		int newSize = members.size();
		// if there's less than two valid members mark as NONE to you know, for deletion process
		if (newSize < 2) {
			++loopShape.id.generation;
			return;
		}
		// otherwise we update the member array valid members to come sequentially, and update the loopsize
		loop->loopSize = newSize;
		for (int i = 0; i < newSize; ++i) {
			gameState->loopMembers[loop->loopArrayOffset + i] = members[i];
		}
	}

	void unselect(GameState* gameState) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isShapeAlive(gameState, i))
				continue;
			getShapeInstance(gameState, i).selected = false;
		}
	}

	std::vector<int> getChildIndexes(GameState* gameState, int id) {
		auto& shape = gameState->shapes[id];
		auto loop = getComponent<components::LoopSociety>(shape);
		std::vector<int> result;
		for (int i = loop->loopArrayOffset; i < loop->loopArrayOffset + loop->loopSize; ++i) {
			result.push_back(gameState->loopMembers[i]);
		}
		return result;
	}

	void reorderLoops(GameState* gameState) {
		gameState->loopIndex = 0;
		// copy old array
		std::array<int, MAX_LOOP_MEMBER_COUNT>oldMembers = gameState->loopMembers;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			Shape& shape = gameState->shapes[i];
			auto loop = getComponent<components::LoopSociety>(shape);
			if (loop == nullptr) {
				continue;
			}
			int loopIndex = gameState->loopIndex;
			for (int j = 0; j < loop->loopSize; ++j) {
				gameState->loopMembers[loopIndex + j] = oldMembers[loop->loopArrayOffset + j];
			}
			loop->loopArrayOffset = gameState->loopIndex;
			gameState->loopIndex += loop->loopSize;
		}
	}

	int findHighestLevelContainer(GameState* gameState, int index)
	{
		if(isSlotFree(gameState, index))
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
			if (!isShapeAlive(gameState, i)) {
				highestI = i;
			}
		}
		return highestI;
	}

	bool isSphere(GameState* gameState, int index)
	{
		Shape& shape = gameState->shapes[index];
		if (getComponent<components::Sphere>(shape)) {
			return true;
		}
		return false;
	}

	bool isContainer(GameState* gameState, int index)
	{
		Shape& shape = gameState->shapes[index];
		auto loop = getComponent<components::LoopSociety>(shape);
		return loop != nullptr && loop->loopSize > 0;
	}

	void dragShape(GameState* gameState, int index, Vector3 linearVelocity) {
		ShapeInstance& instance = getShapeInstance(gameState, index);
		auto loop = getComponent<components::LoopSociety>(instance.shape);
		if (loop != nullptr && loop->loopSize > 0) {
			for (int i = loop->loopArrayOffset; i < loop->loopArrayOffset + loop->loopSize; ++i) {
				int memberIndex = gameState->loopMembers[i];
				dragShape(gameState, memberIndex, linearVelocity);
			}
		}

		// don't move loops, we can move references though
		//if (instance.shape.type == ShapeType::LOOP)
		//	return;

		Vec linearVel = DescVec(linearVelocity);
		auto pData = getComponent<components::PhysicsData>(instance.shape);
		auto posData = getComponent<components::Position>(instance.shape);
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
			for (int i = loop->loopArrayOffset; i < loop->loopArrayOffset + loop->loopSize; ++i) {
				int memberIndex = gameState->loopMembers[i];
				assert(index != memberIndex);
				moveShape(gameState, memberIndex, displacement);
			}
		}
		auto pos = getComponent<components::Position>(shape);
		pos->posX += displacement.x;
		pos->posY += displacement.y;
		pos->posZ += displacement.z;
	}

	bool isGhostShape(int index)
	{
		return index >= GHOST_INDEX_OFFSET;
	}

	bool isShapeAlive(GameState* gameState, int index){
		return gameState->shapes[index].id == gameState->shapeInstances[index].id;
	}

	bool isSlotFree(GameState* gameState, int index) {
		return isShapeAlive(gameState, index);
	}


	ShapeInstance& getShapeInstance(GameState* gameState, int index) {
		auto& instance = gameState->shapeInstances[index];
		auto& shape = gameState->shapes[index];
		assert(instance.id == shape.id);
		return gameState->shapeInstances[index];
	}

	void deleteShape(GameState* gameState, int index) {
		int prevGeneration = gameState->shapes[index].id.generation;
		gameState->shapes[index] = Shape();
		gameState->shapes[index].id.generation = prevGeneration + 1;
	}

	void deleteShapeRecursive(GameState* gameState, int index) {
		Shape& shape = gameState->shapes[index];
		if (isContainer(gameState, index)) {
			std::vector<int> children = getChildIndexes(gameState, index);
			for (int childIndex : children) {
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
		gameState->shapes[index] = shape;
	}

	void addInstance(GameState* gameState, int index, ShapeInstance instance) {
		instance.id.generation = gameState->shapes[index].id.generation;
		gameState->shapeInstances[index] = instance;
	}
	void moveCameraXZ(Camera3D& initCamera, const Vector3& pos)
	{
		Vector3 displacement = pos - initCamera.position;
		initCamera.position += displacement;
		initCamera.target += displacement;
	}
}
