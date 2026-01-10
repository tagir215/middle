#include "init_methods.h"
#include "middle_gameplay_script_map.h"
#include "middle_component_table.h"
#include "Sphere.h"
#include "Color.h"
#include "Position.h"

namespace middle {

	void initEntity(GameState* gameState, int index, Vector3 position, int offset) {
		index += offset;
		auto& shapes = gameState->shapes;
		++shapes[index].id.generation;
	}
	void initJoint(GameState* gameState, int index, Vector3 position, int offset)
	{
		auto& shapes = gameState->shapes;
		++shapes[index].id.generation;
		gameState->ids[index].generation = shapes[index].id.generation;
		components::Sphere* sphere = addComponent<components::Sphere>(shapes[index]);
		components::Color* color = addComponent<components::Color>(shapes[index]);
		components::Position* pos = addComponent<components::Position>(shapes[index]);
		sphere->radius = DEF_RADIUS;
		color->colorR = UGLY_PINK.r;
		color->colorG = UGLY_PINK.g;
		color->colorB = UGLY_PINK.b;
		color->colorA = UGLY_PINK.a;
		pos->posX = position.x;
		pos->posY = position.y;
		pos->posZ = position.z;
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
