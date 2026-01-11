#include "init_methods.h"
#include "middle_gameplay_script_map.h"
#include "middle_component_table.h"
#include "Sphere.h"
#include "Color.h"
#include "Position.h"
#include "MouseSelectable.h"
#include "MouseGrabbable.h"
#include "MouseIntersectable.h"
#include "Constraint.h"
#include "PhysicsData.h"
#include "LoopSociety.h"

namespace middle {

	void initEntity(GameState* gameState, int index, Vector3 position, int offset) {
		index += offset;
		auto& shapes = gameState->shapes;
		++shapes[index].id.generation;
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
