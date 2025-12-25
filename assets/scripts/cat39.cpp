#include "middle_script_registry.h"
#include "middle_shape_utils.h"
static std::string name = "cat39";
static int index = 39;
static int frames = 100;
static int dir = -1;

void print(middle::GameState* gameState) {
	if (frames-- < 0) {
		dir *= -1;
		frames = 100;
	}
	middle::ShapeInstance& instance = gameState->getShapeInstance(index);
	//instance.pData.linearVel = { 10.0f * dir,0,0 };
	moveShape(gameState, instance, { 10.0f * dir, 0,0 });
}

static bool registered = []() {
	middle::registerScript(name, &print);
	return true;
}();
