#include "init_methods.h"
#include "middle_gameplay_script_map.h"
#include "middle_component_table.h"

namespace middle {

	void initJoint(GameState* gameState, int index, Vector3 position, int offset) {
		index += offset;
		auto& shapes = gameState->shapes;
		shapes[index].type = ShapeType::SPHERE;
		shapes[index].position = position;
		++shapes[index].id.generation;
	}
	void initConstraint(GameState* gameState, int index, int indexA, int indexB, float targetDistance, int offset) {
		index += offset;
		indexA += offset;
		indexB += offset;
		auto& shapes = gameState->shapes;
		shapes[index].type = ShapeType::CONSTRAINT;
		shapes[index].initConstraint.indexA = indexA;
		shapes[index].initConstraint.indexB = indexB;
		shapes[index].initConstraint.targetDistance = targetDistance;
		++shapes[index].id.generation;
	}
	void initLoop(GameState* gameState, int index, const std::vector<int>& loopIndexes, int offset) {
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
	void initReference(GameState* gameState, int index, const std::vector<int>& loopIndexes, const std::string& sceneName, int offset) {
		initLoop(gameState, index, loopIndexes, offset);
		gameState->shapes[index].type = ShapeType::REFERENCE;
		gameState->shapes[index].radius = DEF_RADIUS_REFERENCE_INDICATOR;
		gameState->shapes[index].name = sceneName;
	}
	void initCamera(GameState* gameState, int index, const Vector3& position) {
		auto& shapes = gameState->shapes;
		shapes[index].type = ShapeType::CAMERA;
		shapes[index].position = position;
		shapes[index].radius = DEF_RADIUS_CAMERA;
		shapes[index].physicalShape = false;
		++shapes[index].id.generation;
	}
	void initScript(GameState* gameState, int index, const std::string& scriptName, const Vector3& position)
	{
		auto& shapes = gameState->shapes;
		shapes[index].type = ShapeType::SYSTEM;
		shapes[index].position = position;
		shapes[index].name = scriptName;
		shapes[index].radius = DEF_RADIUS_SYSTEM;
		shapes[index].physicalShape = false;
		++shapes[index].id.generation;

		// register script 
		if (scriptMap[scriptName].get() != nullptr) {
			gameState->gameplayScripts[scriptName] = std::move(scriptMap[scriptName]);
		}
	}
	void initComponent(GameState* gameState, int index, int componentId, const std::string& componentName, const Vector3& position) {
		auto& shapes = gameState->shapes;
		shapes[index].type = ShapeType::COMPONENT;
		shapes[index].position = position;
		shapes[index].name = componentName;
		shapes[index].radius = DEF_RADIUS_COMPONENT;
		shapes[index].physicalShape = false;
		shapes[index].component.componentId = componentId;
		++shapes[index].id.generation;
	}
}
