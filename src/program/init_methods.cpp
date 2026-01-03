#include "init_methods.h"

namespace middle {

	void sphere(GameState* gameState, int index, Vector3 position, int offset) {
		index += offset;
		auto& shapes = gameState->shapes;
		shapes[index].type = ShapeType::SPHERE;
		shapes[index].position = position;
		++shapes[index].id.generation;
	}
	void constraint(GameState* gameState, int index, int indexA, int indexB, float targetDistance, int offset) {
		index += offset;
		indexA += offset;
		indexB += offset;
		auto& shapes = gameState->shapes;
		shapes[index].type = ShapeType::CONSTRAINT;
		shapes[index].constraint.indexA = indexA;
		shapes[index].constraint.indexB = indexB;
		shapes[index].constraint.targetDistance = targetDistance;
		++shapes[index].id.generation;
	}
	void loop(GameState* gameState, int index, const std::vector<int>& loopIndexes, int offset) {
		index += offset;

		auto& members = gameState->loopMembers;
		auto& shape = gameState->shapes[index];
		shape.type = ShapeType::LOOP;
		shape.radius = DEF_RADIUS_LOOP_INDICATOR;
		shape.loopArrayOffset = gameState->loopIndex;
		shape.physicalShape = false;
		int loopSize = loopIndexes.size();
		shape.loopSize = loopSize;
		// set member loop, it should be empty at this point
		for (int i = 0; i < loopSize; ++i) {
			int memberIndex = loopIndexes[i] + offset;
			// initializing member loop
			members[shape.loopArrayOffset + i] = memberIndex;
			// initialize parent loop index to member
			auto& memberShape = gameState->shapes[memberIndex];
			memberShape.parentLoopIndex = index;
		}
		gameState->loopIndex += loopSize;
		++shape.id.generation;
	}
	void reference(GameState* gameState, int index, const std::vector<int>& loopIndexes, const std::string& sceneName, int offset) {
		loop(gameState, index, loopIndexes, offset);
		gameState->shapes[index].type = ShapeType::REFERENCE;
		gameState->shapes[index].radius = DEF_RADIUS_REFERENCE_INDICATOR;
		gameState->shapes[index].name = sceneName;
	}
	void camera(GameState* gameState, int index, const Vector3& position) {
		auto& shapes = gameState->shapes;
		shapes[index].type = ShapeType::CAMERA;
		shapes[index].position = position;
		shapes[index].radius = DEF_RADIUS_CAMERA;
		shapes[index].physicalShape = false;
		++shapes[index].id.generation;
	}
	void script(GameState* gameState, int index, const std::string& scriptName, const Vector3& position)
	{
		auto& shapes = gameState->shapes;
		shapes[index].type == ShapeType::SCRIPT;
		shapes[index].position = position;
		shapes[index].name = scriptName;
		shapes[index].radius = DEF_RADIUS_SCRIPT;
		shapes[index].physicalShape = false;
		++shapes[index].id.generation;
	}
}
