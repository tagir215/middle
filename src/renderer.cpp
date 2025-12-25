#include "renderer.h"
#include <raylib.h>
#include <string>
#include <vector>
#include "middle_math.h"
#include "middle_imgui.h"
#include "rlgl.h"

namespace middle {



	void  Draw(GameState* gameState) {
		// Draw
		//----------------------------------------------------------------------------------

		BeginDrawing();

		// 89, 135, 168
		ClearBackground(BACKGROUND_COLOR);

		BeginMode3D(gameState->camera);

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


		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!gameState->isShapeAlive(i))
				continue;

			auto& shapeInstance = gameState->getShapeInstance(i);
			auto& shape = shapeInstance.shape;

			descart::PhysicsBody pData = shapeInstance.pData;

			Vector3 pos = FromDescVec(shapeInstance.pData.position);

			switch (shapeInstance.shape.type) {
			case ShapeType::SPHERE:
				DrawSphere(pos, shape.radius, shape.color);
				if (shapeInstance.grabDown) {
					float length = shape.radius * 4;
					DrawCube(pos, length, length, length, ColorAlpha(ORANGE, 0.3f));
				}
				else if (shapeInstance.selected) {
					float length = shape.radius * 4;
					DrawCube(pos, length, length, length, ColorAlpha(WHITE, 0.3f));
				}
				break;
			case ShapeType::CONSTRAINT: {
				descart::Constraint& constraint = shapeInstance.shape.constraint;
				descart::Vec posA = gameState->getShapeInstance(constraint.indexA).pData.position;
				descart::Vec posB = gameState->getShapeInstance(constraint.indexB).pData.position;
				DrawLine3D(FromDescVec(posA), FromDescVec(posB), UGLY_PINK);

				if (shapeInstance.selected) {
					auto posA = gameState->getShapeInstance(constraint.indexA).pData.position;
					auto posB = gameState->getShapeInstance(constraint.indexB).pData.position;
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
				break;
			}
			case ShapeType::LOOP:

				if(shapeInstance.grabDown) {
					float length = shape.radius * 4;
					DrawCube(pos, length, length, length, ColorAlpha(ORANGE, 0.3f));
				}
				else if (shapeInstance.selected) {
					float length = shape.radius * 4;
					DrawCube(pos, length, length, length, ColorAlpha(WHITE, 0.3f));
				}
				DrawSphere(FromDescVec(shapeInstance.pData.position), shapeInstance.shape.radius, WHITE);
				break;
			}

		}

		EndMode3D();

		// imgui uis
		setupUI(gameState);

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

}
