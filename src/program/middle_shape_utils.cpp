#include "middle_shape_utils.h"
#include "middle_math.h"

namespace middle {


	std::vector<int>findConnectedConstraints(GameState* gameState, int id) {
		std::vector<int> result;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			Shape& shape = gameState->shapes[i];
			if (shape.type != ShapeType::CONSTRAINT)
				continue;

			if (shape.constraint.indexA == id || shape.constraint.indexB == id)
				result.push_back(i);
		}
		return result;
	}

	bool constraintAlreadyExists(GameState* gameState, int indexA, int indexB) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			Shape& shape = gameState->shapes[i];
			if (shape.type != ShapeType::CONSTRAINT)
				continue;

			if (shape.constraint.indexA == indexA && shape.constraint.indexB == indexB)
				return true;

			if (shape.constraint.indexB == indexA && shape.constraint.indexA == indexB)
				return true;
		}

		return false;
	}

	int findFreeIndex(GameState* gameState)
	{
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (gameState->isSlotFree(i))
				return i;
		}
		assert(true);
	}

	void updateLoop(GameState* gameState, int id) {
		Shape& loopShape = gameState->shapes[id];
		assert(loopShape.type == ShapeType::LOOP);
		std::vector<int>members;
		// store still valid members
		for (int i = loopShape.loopArrayOffset; i < loopShape.loopArrayOffset + loopShape.loopSize; ++i) {
			int memberIndex = gameState->loopMembers[i];
			Shape& shape = gameState->shapes[memberIndex];
			if (shape.type != ShapeType::NONE) {
				members.push_back(memberIndex);
			}
		}
		int newSize = members.size();
		// if there's less than two valid members mark as NONE to you know, for deletion process
		if (newSize < 2) {
			loopShape.type = ShapeType::NONE;
			return;
		}
		// otherwise we update the member array valid members to come sequentially, and update the loopsize
		loopShape.loopSize = newSize;
		for (int i = 0; i < newSize; ++i) {
			gameState->loopMembers[loopShape.loopArrayOffset + i] = members[i];
		}
	}

	void unselect(GameState* gameState) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!gameState->isShapeAlive(i))
				continue;
			gameState->getShapeInstance(i).selected = false;
		}
	}

	std::vector<int> getChildIndexes(GameState* gameState, int id) {
		auto& shape = gameState->shapes[id];
		assert(shape.type == ShapeType::LOOP);
		std::vector<int> result;
		for (int i = shape.loopArrayOffset; i < shape.loopArrayOffset + shape.loopSize; ++i) {
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
			if (shape.type != ShapeType::LOOP) {
				continue;
			}
			int loopIndex = gameState->loopIndex;
			for (int j = 0; j < shape.loopSize; ++j) {
				gameState->loopMembers[loopIndex + j] = oldMembers[shape.loopArrayOffset + j];
			}
			shape.loopArrayOffset = gameState->loopIndex;
			gameState->loopIndex += shape.loopSize;
		}
	}

	int findHighestLevelContainer(GameState* gameState, int index)
	{
		if(gameState->isSlotFree(index) || gameState->shapes[index].type == ShapeType::CONSTRAINT)
			return UNASSIGNED;
		Shape& shape = gameState->shapes[index];
		if (shape.parentLoopIndex == UNASSIGNED)
			return index;

		return findHighestLevelContainer(gameState, shape.parentLoopIndex);
	}

	int findHighestUsedIndex(GameState* gameState)
	{
		int highestI = 0;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (gameState->shapes[i].type != ShapeType::NONE) {
				highestI = i;
			}
		}
		return highestI;
	}

	bool isContainer(GameState* gameState, int index)
	{
		ShapeType type = gameState->shapes[index].type;
		return type == ShapeType::LOOP || type == ShapeType::REFERENCE;
	}

	void dragShape(GameState* gameState, int index, Vector3 linearVelocity) {
		ShapeInstance& instance = gameState->getShapeInstance(index);
		if (isContainer(gameState, index)) {
			for (int i = instance.shape.loopArrayOffset; i < instance.shape.loopArrayOffset + instance.shape.loopSize; ++i) {
				int memberIndex = gameState->loopMembers[i];
				dragShape(gameState, memberIndex, linearVelocity);
			}
		}

		// don't move loops, we can move references though
		if (instance.shape.type == ShapeType::LOOP)
			return;

		Vec linearVel = DescVec(linearVelocity);
		if (instance.shape.physicalShape && !gameState->paused) {
			instance.pData.linearVel = linearVel;
		}
		else {
			instance.pData.position = AddV(instance.pData.position, ScaleV(linearVel, gameState->frameTime));
		}
	}

	void moveShape(GameState* gameState, int index, const Vector3& displacement)
	{
		Shape& shape = gameState->shapes[index];
		if (isContainer(gameState, index)) {
			for (int i = shape.loopArrayOffset; i < shape.loopArrayOffset + shape.loopSize; ++i) {
				int memberIndex = gameState->loopMembers[i];
				assert(index != memberIndex);
				moveShape(gameState, memberIndex, displacement);
			}
		}
		shape.position += displacement;
	}

	bool isGhostShape(int index)
	{
		return index >= GHOST_INDEX_OFFSET;
	}

}
