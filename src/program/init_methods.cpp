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

namespace middle {

	void initEntity(GameState* gameState, int index, Vector3 position, int offset) {
		index += offset;
		auto& shapes = gameState->shapes;
		++shapes[index].id.generation;
	}
	void initJoint(GameState* gameState, int index, Vector3 position, int offset)
	{
		auto& shape = gameState->shapes[index];
		++shape.id.generation;
		gameState->ids[index].generation = shape.id.generation;
		components::Sphere* sphere = addComponent<components::Sphere>(shape);
		components::Color* color = addComponent<components::Color>(shape);
		components::Position* pos = addComponent<components::Position>(shape);
		components::PhysicsData* physics = addComponent<components::PhysicsData>(shape);
		addComponent<components::MouseSelectable>(shape);
		addComponent<components::MouseGrabbable>(shape);
		addComponent<components::MouseIntersectable>(shape);
		sphere->radius = DEF_RADIUS;
		color->colorR = UGLY_PINK.r;
		color->colorG = UGLY_PINK.g;
		color->colorB = UGLY_PINK.b;
		color->colorA = UGLY_PINK.a;
		pos->posX = position.x;
		pos->posY = position.y;
		pos->posZ = position.z;
		physics->mass = 1;
		physics->invMass = 1;
		physics->momentOfInertia = 1;
		physics->invMomentOfInertia = 1;
		physics->damX = DEF_DAMPING;
		physics->damY = DEF_DAMPING;
		physics->damZ = DEF_DAMPING;
	}
	void initConstraint(GameState* gameState, int index, int indexA, int indexB, float targetDistance, int offset)
	{
		auto& shape = gameState->shapes[index];
		++shape.id.generation;
		gameState->ids[index].generation = shape.id.generation;
		components::Color* color = addComponent<components::Color>(shape);
		components::Constraint* constraint = addComponent<components::Constraint>(shape);
		addComponent<components::MouseSelectable>(shape);
		addComponent<components::MouseIntersectable>(shape);
		constraint->stiffness = DEF_STIFFNESS;
		constraint->targetDistance = targetDistance;
		constraint->indexA = indexA;
		constraint->indexB = indexB;
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
