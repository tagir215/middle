#pragma once

#include <iostream>
#include "game.h"
#include "state_update.h"
#include "descart_loop.h"
#include "middle_script_registry.h"

using namespace middle;



__declspec(dllexport) void UpdateGame(GameState* gameState)
{
	processEditorActions(gameState);

	// update
	if (gameState->reload) {
		//gameInitializer.update(gameState);
		reset(gameState);
		loadSceneNames(gameState);
		if (gameState->sceneNames.size() > 0) {
			loadScene(gameState, gameState->sceneNames[gameState->activeScene], false);
		}
	}
	updateInstances(gameState);


	// camera controls
	const float maxCameraSpeed = 20;
	float mouseCamRatio = gameState->input.mousePos.y / gameState->screenHeight;
	const float cameraSpeed = mouseCamRatio * mouseCamRatio * mouseCamRatio * maxCameraSpeed;
	Vector3 cameraMovementDir = { 0,0,0 };
	if (gameState->input.w)
		cameraMovementDir += Vector3Normalize(gameState->camera.target - gameState->camera.position);
	if (gameState->input.s)
		cameraMovementDir += Vector3Negate(Vector3Normalize(gameState->camera.target - gameState->camera.position));
	if (gameState->input.e)
		cameraMovementDir += { 0, 0, 1 };
	if (gameState->input.q)
		cameraMovementDir += { 0, 0, -1 };
	if (gameState->input.d)
		cameraMovementDir += Vector3Negate(Vector3Normalize(Vector3CrossProduct(gameState->camera.up, gameState->camera.target - gameState->camera.position)));
	if (gameState->input.a)
		cameraMovementDir += Vector3Normalize(Vector3CrossProduct(gameState->camera.up, gameState->camera.target - gameState->camera.position));

	gameState->camera.position += cameraMovementDir * cameraSpeed;
	gameState->camera.target += cameraMovementDir * cameraSpeed;


	// mouse intersects 
	loopInstances(gameState, [gameState](int i, ShapeInstance& instance) {
		Shape& shape = gameState->shapes[i];

		bool wasIntersecting = instance.mouseIntersects;
		bool container = isContainer(gameState, i);
		if (shape.type == ShapeType::SPHERE || container) {
			// when in constraint mode only can select shapes
			if (container && gameState->creationMode == CreationMode::CONSTRAINT_MODE)
				return;
			auto pos = instance.pData.position;
			bool mouseIntersect = RayCastLineSphere(FromDescVec(pos), instance.shape.radius, gameState->camera.position, gameState->camera.position + gameState->input.mouseDir);
			instance.mouseIntersects = mouseIntersect;
		}
		if (shape.type == ShapeType::CONSTRAINT) {
			// in constraint creation mode, can't select constraints, only spheres to create constraints to
			if (gameState->creationMode == CreationMode::CONSTRAINT_MODE)
				return;
			auto& instanceA = gameState->getShapeInstance(instance.shape.constraint.indexA);
			auto& instanceB = gameState->getShapeInstance(instance.shape.constraint.indexB);
			auto posA = instanceA.pData.position;
			auto posB = instanceB.pData.position;
			bool mouseIntersect = PointIntersectLineZX_Plane(gameState->input.mouseXZ_PlanePos, FromDescVec(posA), FromDescVec(posB), DEF_LINE_PADDING_H, DEF_LINE_PADDING_V);
			instance.mouseIntersects = mouseIntersect;
		}

		// when holding down, don't immediatedly toggle once when starting intersect
		if (!wasIntersecting && instance.mouseIntersects && gameState->input.mouseHeld) {
			instance.selected = !instance.selected;
		}

		// toggle selection when clicking
		if (instance.mouseIntersects && gameState->input.mouseClicked) {
			instance.selected = !instance.selected;
		}

		// grabbing activates selected if there's no selections yet, except can't grab constraints
		if (instance.mouseIntersects && gameState->input.grabDown && gameState->selectCount == 0 && shape.type != ShapeType::CONSTRAINT) {
			instance.selected = true;
		}

		});

	// count update
	gameState->intersectCount = 0;
	gameState->selectCount = 0;
	loopInstances(gameState, [gameState](int i, ShapeInstance& instance) {
		if (instance.selected)
			++gameState->selectCount;
		if (instance.mouseIntersects)
			++gameState->intersectCount;
		});

	// info on/off
	if (gameState->intersectCount == 0 && gameState->input.infoClick) {
		gameState->showAllInfo = !gameState->showAllInfo;
	}
	else {
		loopInstances(gameState, [gameState](int i, ShapeInstance& instance) {
			if (instance.selected && gameState->input.infoClick) {
				instance.infoVisible = !instance.infoVisible;
			}
			});
	}

	// unselect
	if (gameState->input.mouseClicked && gameState->intersectCount == 0) {
		unselect(gameState);
	}

	// open script
	if (gameState->input.openScript && gameState->intersectCount > 0) {
		loopInstances(gameState, [&](int i, ShapeInstance& instance) {
			if (instance.mouseIntersects) {
				gameState->nextEditorAction = EditorAction::OPEN_SCRIPT;
				gameState->nextEditorActionParams = { "", i };
			}
			});
	}

	// movement
	loopInstances(gameState, [gameState](int i, ShapeInstance& instance) {
		Shape& shape = gameState->shapes[i];

		if (instance.selected && gameState->input.grabDown) {
			instance.grabDown = true;
		}
		else {
			instance.grabDown = false;
		}

		if (instance.grabDown) {
			dragShape(gameState, i, gameState->input.mouseXZ_PlaneVelocity);
		}

		});

	if (gameState->input.grabReleased && gameState->selectCount == 1) {
		unselect(gameState);
	}

	bool paused = gameState->paused && !gameState->doOneStep;
	gameState->doOneStep = false;

	if (paused || gameState->stepDir == -1)
	{
		gameState->doOneStep = false;
		return;
	}

	// run scripts
	loopInstances(gameState, [gameState](int i, ShapeInstance& instance) {
		runScript(gameState, i);
		});


	// shape phsyics stuff
	// create pairs

	std::vector<Constraint> constraints;
	std::vector<BodyPair> pairs;
	std::vector<int> grounds;

	if (physicsBodies.size() < MAX_SHAPE_COUNT) {
		physicsBodies.resize(MAX_SHAPE_COUNT);
	}


	for (int i = 0; i < gameState->shapes.size(); ++i) {
		if (!gameState->isShapeAlive(i)) {
			physicsBodies[i] = nullptr;
			continue;
		}
		auto& instance = gameState->getShapeInstance(i);

		if (instance.shape.type == ShapeType::CONSTRAINT) {
			if (instance.shape.constraint.indexA != UNASSIGNED && instance.shape.constraint.indexB != UNASSIGNED)
				constraints.push_back(instance.shape.constraint);
		}
		if (instance.shape.type == ShapeType::SPHERE) {
			for (int j : grounds) {
				BodyPair pair;
				pair.indexA = i;
				pair.indexB = j;
				pairs.push_back(pair);
			}
			physicsBodies[i] = &instance.pData;
		}
	}

	DescLoop(gameState->frameTime, pairs, constraints, physicsBodies);

}


