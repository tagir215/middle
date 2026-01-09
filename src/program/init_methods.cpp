#include "init_methods.h"
#include "middle_gameplay_script_map.h"
#include "middle_component_table.h"

namespace middle {

	void initEntity(GameState* gameState, int index, Vector3 position, int offset) {
		index += offset;
		auto& shapes = gameState->shapes;
		++shapes[index].id.generation;
	}
	void initJoint(GameState* gameState, int index, Vector3 position, int offset)
	{
	}
	void initConstraint(GameState* gameState, int index, int indexA, int indexB, float targetDistance, int offset)
	{
	}
	void initLoop(GameState* gameState, int index, const std::vector<int>& loopIndexes, int offset)
	{
	}
	void initReference(GameState* gameState, int index, const std::vector<int>& loopIndexes, const std::string& sceneName, int offset)
	{
	}
	void initCamera(GameState* gameState, int index, const Vector3& position)
	{
	}
	void initScript(GameState* gameState, int index, const std::string& scriptName, const Vector3& position)
	{
	}
}
