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

	Matrix transformMatrix(Transform& transform) {
		Matrix S = MatrixScale(transform.scale.x, transform.scale.y, transform.scale.z);
		Matrix R = QuaternionToMatrix(transform.rotation);
		Matrix T = MatrixTranslate(transform.translation.x, transform.translation.y, transform.translation.z);
		Matrix M = MatrixMultiply(MatrixMultiply(S, R), T);
		return M;
	}

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
					Matrix M = transformMatrix(item.transform);
					rlPushMatrix();
					rlMultMatrixf(MatrixToFloatV(M).v);
					DrawCube(item.center, item.width, item.length, item.height, item.color);
					rlPopMatrix();
				}

				if (item.type == middle::RenderItemType::CYLINDER) {
					Matrix M = transformMatrix(item.transform);
					rlPushMatrix();
					rlMultMatrixf(MatrixToFloatV(M).v);
					DrawCylinder(item.center, item.radius, item.ringRadius, item.length, 23, item.color);
					rlPopMatrix();
				}

				if (item.type == middle::RenderItemType::MODEL) {
					DrawModel(*item.model, item.center, 1, item.color);
				}

				if (item.type == middle::RenderItemType::VECTOR) {
					Matrix M = transformMatrix(item.transform);
					rlPushMatrix();
					rlLoadIdentity();
					rlMultMatrixf(MatrixToFloatV(M).v);
					DrawCylinder(item.center, item.radius, item.radius, item.length, 23, item.color);
					rlPopMatrix();

					Vector3 rotationForward = { 0,1,0 };
					Vector3 dir = Vector3RotateByQuaternion(rotationForward, item.transform.rotation);
					//DrawSphere(conePos, 3, BLUE);
					Vector3 conePos = item.transform.translation + Vector3Scale(dir, item.length);
					item.transform.translation = conePos;
					M = transformMatrix(item.transform);

					rlPushMatrix();
					rlMultMatrixf(MatrixToFloatV(M).v);
					DrawCylinder(item.center, 0, item.radius * 2, 10, 23, item.color);
					rlPopMatrix();

				}

				if (item.type == middle::RenderItemType::LINE) {
					DrawLine3D(item.linePointA, item.linePointB, item.color);
				}

				if (item.type == middle::CIRCLE) {
					Matrix M = transformMatrix(item.transform);
					rlPushMatrix();
					rlLoadIdentity();
					rlMultMatrixf(MatrixToFloatV(M).v);
					DrawCircle3D(item.center, item.radius, { 1,0,0 }, 90, item.color);
					rlPopMatrix();
				}

				if (item.type == middle::CIRCLE_SECTOR) {
					if (item.segments > 0) {
						Vector3 lastPos;
						float deltaAngle = (item.endAngle - item.startAngle) / item.segments;
						for (int i = 0; i < item.segments + 1; ++i) {
							float angle = item.startAngle + deltaAngle * i;
							Vector3 v = { 1,0,0 };
							Vector3 vr = Vector3RotateByAxisAngle(v, { 0,-1,0 }, angle);
							Vector3 pos = item.center + Vector3Scale(vr, item.radius);
							if (i > 0) {
								Transform transform;
								Vector3 dir = Vector3Normalize(Vector3Subtract(lastPos, pos));
								transform.rotation = QuaternionFromVector3ToVector3({ 0,-1,0 }, dir);
								transform.translation = lastPos;
								transform.scale = { 1,1,1 };
								float length = Vector3Distance(lastPos, pos);
								Matrix M = transformMatrix(transform);
								DrawLine3D(lastPos, pos, item.color);
								rlPushMatrix();
								rlLoadIdentity();
								rlMultMatrixf(MatrixToFloatV(M).v);
								int slices = 10;
								DrawCylinder({ 0,0,0 }, item.ringRadius, item.ringRadius, length, slices, item.color);
								rlPopMatrix();
							}
							lastPos = pos;
						}
					}
				}

				if (item.type == middle::CUBOID) {
					Matrix M = transformMatrix(item.transform);
					rlPushMatrix();
					rlLoadIdentity();
					rlMultMatrixf(MatrixToFloatV(M).v);
					DrawCube(item.center, item.width, item.height, item.length, item.color);
					rlPopMatrix();
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
