#pragma once
#include "game_state.h"
#include "middle_imgui.h"
#include "middle_constants.h"
#include <iostream>
#include "registrars.h"
#include "Sphere.h"
#include "Position.h"
#include "Constraint.h"
#include "Color.h"

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


			middle::loopInstances(gameState, [gameState](int i, middle::ShapeInstance& shapeInstance) {
				auto& shape = shapeInstance.shape;
				auto colorComponent = middle::getComponent<components::Color>(shape);
				if (colorComponent == nullptr)
					return;
				auto posComponent = middle::getComponent<components::Position>(shape);
				auto sphereComponent = middle::getComponent<components::Sphere>(shape);
				auto constraintComponent = middle::getComponent<components::Constraint>(shape);

				Color color = { colorComponent->colorA, colorComponent->colorG, colorComponent->colorB };

				if (sphereComponent) {
					Vector3 pos = { posComponent->posX, posComponent->posY, posComponent->posZ };
					float radius = sphereComponent->radius;
					DrawSphere(pos, radius, color);
					if (shapeInstance.grabDown) {
						float length = radius * 4;
						DrawCube(pos, length, length, length, ColorAlpha(ORANGE, 0.3f));
					}
					else if (shapeInstance.selected) {
						float length = radius * 4;
						DrawCube(pos, length, length, length, ColorAlpha(WHITE, 0.3f));
					}
				}


				if (constraintComponent) {
					auto& a = middle::getShapeInstance(gameState, constraintComponent->indexA);
					auto& b = middle::getShapeInstance(gameState, constraintComponent->indexB);
					auto posComA = middle::getComponent<components::Position>(a.shape);
					auto posComB = middle::getComponent<components::Position>(b.shape);
					Vec posA = { posComA->posX, posComA->posY, posComA->posZ };
					Vec posB = { posComB->posX, posComB->posY, posComB->posZ };

					if (shapeInstance.selected) {
						auto center = descart::ScaleV(descart::AddV(posA, posB), 0.5f);
						float length = descart::DistV(posA, posB);
						auto dir = descart::SubV(posB, posA);
						float angle = std::atan2(dir.x, dir.z);
						rlPushMatrix();
						rlTranslatef(center.x, center.y, center.z);
						rlRotatef(RAD2DEG * angle, 0, 1, 0);
						DrawCube({ 0,0,0 }, 2, 2, length, ColorAlpha(WHITE, 0.3f));
						rlPopMatrix();
					}
				}
				});


			EndMode3D();

			// imgui uis
			setupUI(gameState);

			EndDrawing();
		}
	};

	static middle::SystemRegistrar<RendererSystem> reg(scriptName);

}
