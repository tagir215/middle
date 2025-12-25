#include "middle_script_registry.h"
#include <iostream>
static std::string name = "cat12";

void print(middle::GameState* gameState) {
	//std::cout << "hello from " << name << std::endl;
}

static bool registered = []() {
	middle::registerScript(name, &print);
	return true;
}();
