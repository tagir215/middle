#include "editor_update.h"
#include "Position.h"
#include "Constraint.h"
#include "Sphere.h"

namespace middle {
	void updateEditor(GameState* gameState)
	{
		processEditorActions(gameState);

		if (gameState->startGame) {
			if (gameState->applicationMode == ApplicationMode::EDITOR_MODE) {
				loadEditorState(gameState);
			}
			gameState->startGame = false;
		}

		// update
		if (gameState->reload) {
			reset(gameState);
			loadSceneNames(gameState);
			loadScriptNames(gameState);
			loadComponentNames(gameState);
			if (gameState->sceneNames.size() > 0) {
				loadScene(gameState, gameState->sceneNames[gameState->activeScene], false);
			}
			gameState->reload = false;
		}
		updateInstances(gameState);


		// camera controls
		const float maxCameraSpeed = 60;
		float mouseCamRatio = gameState->input.mousePos.y / gameState->screenHeight;
		const float cameraSpeed = mouseCamRatio * mouseCamRatio * mouseCamRatio * maxCameraSpeed;
		Vector3 cameraMovementDir = { 0,0,0 };
		if (gameState->input.w)
			cameraMovementDir += Vector3Normalize(gameState->editorState.initCamera.target - gameState->editorState.initCamera.position);
		if (gameState->input.s)
			cameraMovementDir += Vector3Negate(Vector3Normalize(gameState->editorState.initCamera.target - gameState->editorState.initCamera.position));
		if (gameState->input.e)
			cameraMovementDir += { 0, 0, 1 };
		if (gameState->input.q)
			cameraMovementDir += { 0, 0, -1 };
		if (gameState->input.d)
			cameraMovementDir += Vector3Negate(Vector3Normalize(Vector3CrossProduct(gameState->editorState.initCamera.up, gameState->editorState.initCamera.target - gameState->editorState.initCamera.position)));
		if (gameState->input.a)
			cameraMovementDir += Vector3Normalize(Vector3CrossProduct(gameState->editorState.initCamera.up, gameState->editorState.initCamera.target - gameState->editorState.initCamera.position));

		gameState->editorState.initCamera.position += cameraMovementDir * cameraSpeed;
		gameState->editorState.initCamera.target += cameraMovementDir * cameraSpeed;


		// mouse intersects 
		loopInstances(gameState, [gameState](int i, ShapeInstance& instance) {
			Shape& shape = gameState->shapes[i];

			bool wasIntersecting = instance.mouseIntersects;
			bool bContainer = isContainer(gameState, i);
			bool bSphere = isSphere(gameState, i);
			if (bSphere) {
				// when in constraint mode can only select actual spheres
				if (gameState->editorState.creationMode == CreationMode::CONSTRAINT_MODE && !isSphere(gameState, i))
					return;

				auto sphere = getComponent<components::Sphere>(shape);
				auto posC = getComponent<components::Position>(shape);
				Vec pos = {posC->posX, posC->posY, posC->posZ};
				Vector3 intersectPos;
				bool mouseIntersect = RayCastLineSphere(FromDescVec(pos), sphere->radius, gameState->editorState.initCamera.position, gameState->editorState.initCamera.position + gameState->input.mouseDir, intersectPos);
				instance.mouseIntersects = mouseIntersect;
			}
			auto constraint = getComponent<components::Constraint>(shape);
			if (constraint != nullptr) {
				// in constraint creation mode, can't select constraints, only spheres to create constraints to
				if (gameState->editorState.creationMode == CreationMode::CONSTRAINT_MODE)
					return;
				auto& instanceA = getShapeInstance(gameState, constraint->indexA);
				auto& instanceB = getShapeInstance(gameState, constraint->indexB);
				auto posCA = getComponent<components::Position>(instanceA.shape);
				auto posCB = getComponent<components::Position>(instanceB.shape);
				Vec posA = {posCA->posX, posCA->posY, posCA->posZ};
				Vec posB = {posCB->posX, posCB->posY, posCB->posZ};
				bool mouseIntersect = PointIntersectLineZX_Plane(gameState->input.mouseXZ_PlanePos, FromDescVec(posA), FromDescVec(posB), DEF_LINE_PADDING_H, DEF_LINE_PADDING_V);
				instance.mouseIntersects = mouseIntersect;
			}

			// ghost shapes can't be selected or edited
			if (isGhostShape(i)) {
				return;
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
			if (instance.mouseIntersects && gameState->input.grabDown && gameState->selectCount == 0 && constraint == nullptr) {
				instance.selected = true;
				++gameState->selectCount;
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
			gameState->editorState.showAllInfo = !gameState->editorState.showAllInfo;
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
					//if(instance.shape.type == ShapeType::SYSTEM)
					//	gameState->editorState.nextEditorAction = EditorAction::OPEN_SYSTEM;
					//if(instance.shape.type == ShapeType::COMPONENT)
					//	gameState->editorState.nextEditorAction = EditorAction::OPEN_COMPONENT;
					//gameState->editorState.nextEditorActionParams = { "", i };
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
				auto posC = getComponent<components::Position>(instance.shape);
				Vec pos = { posC->posX, posC->posY, posC->posZ };
				float objYDistance = std::abs(pos.y - pos.y);
				float yDistance = std::abs(gameState->editorState.initCamera.position.y);
				if (yDistance == 0)
					yDistance = 0.001f;
				Vector3 xzVel = Vector3Scale(gameState->input.mouseXZ_PlaneVelocity, objYDistance / yDistance);
				dragShape(gameState, i, xzVel);
			}
			});

		if (gameState->input.grabReleased && gameState->selectCount == 1) {
			unselect(gameState);
		}

		bool paused = gameState->paused && !gameState->editorState.doOneStep;
		gameState->editorState.doOneStep = false;

		if (paused || gameState->editorState.stepDir == -1)
		{
			gameState->editorState.doOneStep = false;
			return;
		}
	}
}