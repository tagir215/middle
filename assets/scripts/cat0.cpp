#include "middle_script_registry.h"
static std::string name = "cat0";

static void print(middle::GameState* gameState) {

}

static bool registered = []() {
	middle::registerScript(name, &print);
	return true;
}();
