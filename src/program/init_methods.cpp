#include "init_methods.h"

namespace middle {
	GameState* gameStateRef = nullptr;

	void sphere(int index, Vector3 position, int offset) {
		index += offset;
		auto& shapes = gameStateRef->shapes;
		shapes[index].type = ShapeType::SPHERE;
		shapes[index].position = position;
		++shapes[index].id.generation;
	}
	void constraint(int index, int indexA, int indexB, float targetDistance, int offset) {
		index += offset;
		indexA += offset;
		indexB += offset;
		auto& shapes = gameStateRef->shapes;
		shapes[index].type = ShapeType::CONSTRAINT;
		shapes[index].constraint.indexA = indexA;
		shapes[index].constraint.indexB = indexB;
		shapes[index].constraint.targetDistance = targetDistance;
		++shapes[index].id.generation;
	}
	void loop(int index, const std::vector<int>& loopIndexes, int offset) {
		index += offset;

		auto& members = gameStateRef->loopMembers;
		auto& shape = gameStateRef->shapes[index];
		shape.type = ShapeType::LOOP;
		shape.radius = DEF_RADIUS_LOOP_INDICATOR;
		shape.loopArrayOffset = gameStateRef->loopIndex;
		shape.physicalShape = false;
		int loopSize = loopIndexes.size();
		shape.loopSize = loopSize;
		// set member loop, it should be empty at this point
		for (int i = 0; i < loopSize; ++i) {
			int memberIndex = loopIndexes[i] + offset;
			// initializing member loop
			members[shape.loopArrayOffset + i] = memberIndex;
			// initialize parent loop index to member
			auto& memberShape = gameStateRef->shapes[memberIndex];
			memberShape.parentLoopIndex = index;
		}
		gameStateRef->loopIndex += loopSize;
		++shape.id.generation;
	}
	void reference(int index, const std::vector<int>& loopIndexes, const std::string& sceneName, int offset) {
		loop(index, loopIndexes, offset);
		gameStateRef->shapes[index].type = ShapeType::REFERENCE;
		gameStateRef->shapes[index].radius = DEF_RADIUS_REFERENCE_INDICATOR;
		gameStateRef->shapes[index].name = sceneName;
	}
}
