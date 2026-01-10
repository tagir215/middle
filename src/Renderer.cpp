#pragma once
#include "game_state.h"
#include "middle_imgui.h"
#include "registrars.h"
#include "rlImGui.h"

namespace RendererSystem {

	static std::string scriptName = "RendererSystem";

	class RendererSystem : public middle::MiddleGameplaySystem {
		void update(middle::GameState* gameState) override {

			BeginDrawing();

			// 89, 135, 168
			ClearBackground(middle::BACKGROUND_COLOR);

			BeginMode3D(gameState->editorState.initCamera);

			rlSetClipPlanes(gameState->nearPlaneDistance, gameState->farPlaneDistance);

			const float axisLength = 1000;
			const Color CartesianColor = WHITE;

			float mouseX = gameState->input.mouseXZ_PlanePos.x;
			float mouseZ = gameState->input.mouseXZ_PlanePos.z;
			// draw grid
			const float visibleGridRadius = 10;
			float visibleGridRadiusSq = visibleGridRadius * visibleGridRadius * gameState->gridSize * gameState->gridSize;
			float gridSphereRadius = 0.10f * gameState->gridSize;
			for (float x = -axisLength; x < axisLength; x += gameState->gridSize) {
				for (float z = -axisLength; z < axisLength; z += gameState->gridSize) {
					float deltaX = mouseX - x;
					float deltaZ = mouseZ - z;
					float distSq = deltaX * deltaX + deltaZ * deltaZ;
					float ratio = distSq / visibleGridRadiusSq;
					if (distSq < visibleGridRadiusSq) {
						DrawSphere({ x,0,z }, gridSphereRadius, ColorAlpha(CartesianColor, (1 - ratio)));
					}
				}
			}


			for(int i=0; i<gameState->renderData.size(); ++i){
				middle::RenderItem item = gameState->renderData[i];

				if (item.type == middle::RenderItemType::SPHERE) {
					DrawSphere(item.center, item.radius, item.color);
				}

				if (item.type == middle::RenderItemType::LINE) {
					DrawLine3D(item.linePointA, item.linePointB, item.color);
				}
			}


			EndMode3D();

			// imgui uis
			setupUI(gameState);

			EndDrawing();
		}
	};

	static middle::SystemRegistrar<RendererSystem> reg(scriptName);

}
