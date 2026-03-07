#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "raymath.h"
#include "rlImGui.h"
#include "imgui.h"
#include <cstdlib>
#include <thread>
#include "rlgl.h"

namespace RendererSystem {

	static std::string scriptName = "RendererSystem";

	class RendererSystem : public middle::MiddleGameplaySystem {
	public:
		void init(middle::GameState* gameState) {

		}
		void update(middle::GameState* gameState) override {

			BeginDrawing();

			// 89, 135, 168
			ClearBackground(gameState->editorState.backgroundColor);

			Camera camera = gameState->activeCamera;

			BeginMode3D(camera);

			// center indicator
			DrawCube({ 0,5,0 }, 5, 5, 5, BLACK);

			rlSetClipPlanes(gameState->nearPlaneDistance, gameState->farPlaneDistance);


			for (int i = 0; i < gameState->renderData.size(); ++i) {
				middle::RenderItem item = gameState->renderData[i];

				if (item.type == middle::RenderItemType::SPHERE) {
					DrawSphereEx(item.center, item.radius, 5, 5, item.color);
				}

				if (item.type == middle::RenderItemType::RECTANGLE) {
					Matrix T = MatrixTranslate(item.transform.translation.x, item.transform.translation.y, item.transform.translation.z);
					Matrix R = QuaternionToMatrix(item.transform.rotation);
					Matrix S = MatrixScale(item.transform.scale.x, item.transform.scale.y, item.transform.scale.z);
					Matrix M = MatrixMultiply(MatrixMultiply(S, R), T);
					rlPushMatrix();
					rlMultMatrixf(MatrixToFloatV(M).v);
					DrawCube(item.center, item.width, item.length, item.height, item.color);
					rlPopMatrix();
				}

				if (item.type == middle::RenderItemType::LINE) {
					DrawLine3D(item.linePointA, item.linePointB, item.color);
				}

				if (item.type == middle::CIRCLE) {
					DrawCircle3D(item.center, item.radius, { 1,0,0 }, 90, item.color);
				}
			}


			EndMode3D();

			for (int i = 0; i < gameState->renderData.size(); ++i) {
				middle::RenderItem item = gameState->renderData[i];
				if (item.type == middle::RenderItemType::TEXT) {
					Vector2 pos = GetWorldToScreen(item.center, camera);
					DrawText(item.text.c_str(), pos.x, pos.y, item.fontSize, item.color);
				}
			}

			Vector3 center = { 0,0,0 };
			Vector2 center2d = GetWorldToScreen(center, camera);
			if (gameState->sceneNames.size() > 0) {
				DrawText(gameState->sceneNames[gameState->activeScene].c_str(), center2d.x, center2d.y, 1, WHITE);
			}

			rlImGuiBegin();

			while (gameState->uiSetups.size() > 0) {
				gameState->uiSetups.back()();
				gameState->uiSetups.pop_back();
			}

			rlImGuiEnd();

			EndDrawing();

			gameState->renderData.clear();
		}
	};

	static middle::SystemRegistrar<RendererSystem> reg(scriptName);

}
