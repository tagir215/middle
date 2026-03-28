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
	const float layerGap = -1.0f;

	Matrix transformMatrix(Transform& transform, int layer) {
		Matrix S = MatrixScale(transform.scale.x, transform.scale.y, transform.scale.z);
		Matrix R = QuaternionToMatrix(transform.rotation);
		Matrix T = MatrixTranslate(transform.translation.x, transform.translation.y + layer * layerGap, transform.translation.z);
		Matrix M = MatrixMultiply(MatrixMultiply(S, R), T);
		return M;
	}

	std::vector<Vector3> getRectVertices(const Vector3& pos, const Vector3& scale, float width, float height)
	{
		Vector3 s = scale;
		std::vector<Vector3> vertices;
		vertices.resize(4);
		vertices[0] = { -width * 0.5f * s.x, 0, height * 0.5f * s.z };
		vertices[1] = { -width * 0.5f * s.x, 0, -height * 0.5f * s.z };
		vertices[2] = { width * 0.5f * s.x, 0, -height * 0.5f * s.z };
		vertices[3] = { width * 0.5f * s.x, 0, height * 0.5f * s.z };
		vertices[0] += pos;
		vertices[1] += pos;
		vertices[2] += pos;
		vertices[3] += pos;
		return vertices;
	}

	void drawRect(middle::GameState* gameState, const std::vector<Vector3>& vertices, const Color& color) {
		middle::RenderItem line1;
		line1.type = middle::RenderItemType::LINE;
		line1.linePointA = vertices[0];
		line1.linePointB = vertices[1];
		line1.color = color;
		DrawLine3D(line1.linePointA, line1.linePointB, line1.color);
		middle::RenderItem line2;
		line2.type = middle::RenderItemType::LINE;
		line2.linePointA = vertices[1];
		line2.linePointB = vertices[2];
		line2.color = color;
		DrawLine3D(line2.linePointA, line2.linePointB, line2.color);
		middle::RenderItem line3;
		line3.type = middle::RenderItemType::LINE;
		line3.linePointA = vertices[2];
		line3.linePointB = vertices[3];
		line3.color = color;
		DrawLine3D(line3.linePointA, line3.linePointB, line3.color);
		middle::RenderItem line4;
		line4.type = middle::RenderItemType::LINE;
		line4.linePointA = vertices[3];
		line4.linePointB = vertices[0];
		line4.color = color;
		DrawLine3D(line4.linePointA, line4.linePointB, line4.color);
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

			if (gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE) {
				// center indicator
				DrawCube({ 0,5,0 }, 5, 5, 5, BLACK);
			}

			rlSetClipPlanes(gameState->nearPlaneDistance, gameState->farPlaneDistance);


			for (int i = 0; i < gameState->renderData.size(); ++i) {
				middle::RenderItem item = gameState->renderData[i];


				if (item.type == middle::RenderItemType::SPHERE) {
					Vector3 pos = item.center;
					DrawSphereEx(item.center, item.radius, 5, 5, item.color);
				}


				if (item.type == middle::RenderItemType::RECTANGLE) {
					Matrix M = transformMatrix(item.transform, item.layer);
					rlPushMatrix();
					rlMultMatrixf(MatrixToFloatV(M).v);
					Vector3 pos = item.center;
					pos.y += item.layer * layerGap;
					if (item.color.a != 0) {
						drawRect(gameState, getRectVertices(item.center, item.scale, item.width, item.height), item.color);
					}
					if (item.backgroundColor.a != 0) {
						DrawCube(item.center, item.width, item.length, item.height, item.color);
					}
					rlPopMatrix();
				}

				if (item.type == middle::RenderItemType::CYLINDER) {
					Matrix M = transformMatrix(item.transform, item.layer);
					rlPushMatrix();
					rlMultMatrixf(MatrixToFloatV(M).v);
					Vector3 pos = item.center;
					DrawCylinder(pos, item.radius, item.ringRadius, item.length, 23, item.color);
					rlPopMatrix();
				}

				if (item.type == middle::RenderItemType::MODEL) {
					DrawModel(*item.model, item.center, 1, item.color);
				}

				if (item.type == middle::RenderItemType::VECTOR) {
					Matrix M = transformMatrix(item.transform, item.layer);
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
					M = transformMatrix(item.transform, item.layer);

					rlPushMatrix();
					rlMultMatrixf(MatrixToFloatV(M).v);
					DrawCylinder(item.center, 0, item.radius * 2, 10, 23, item.color);
					rlPopMatrix();

				}

				if (item.type == middle::RenderItemType::LINE) {
					Vector3 posA = item.linePointA + Vector3{ 0, item.layer * layerGap, 0 };
					Vector3 posB = item.linePointB + Vector3{ 0, item.layer * layerGap, 0 };
					DrawLine3D(posA, posB, item.color);
				}

				if (item.type == middle::CIRCLE) {
					Matrix M = transformMatrix(item.transform, item.layer);
					rlPushMatrix();
					rlLoadIdentity();
					rlMultMatrixf(MatrixToFloatV(M).v);
					DrawCircle3D(item.center, item.radius, { 1,0,0 }, 90, item.color);
					if (item.backgroundColor.a != 0) {
						DrawCylinder(item.center, item.radius, item.radius, 0.000000001f, 20, item.backgroundColor);
					}
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
								Matrix M = transformMatrix(transform, item.layer);
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
					Matrix M = transformMatrix(item.transform, item.layer);
					rlPushMatrix();
					rlLoadIdentity();
					rlMultMatrixf(MatrixToFloatV(M).v);
					Vector3 pos = item.center;
					DrawCube(pos, item.width, item.height, item.length, item.color);
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
			if (gameState->sceneNames.size() > 0 && gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE) {
				DrawText(gameState->activeSceneName.c_str(), center2d.x, center2d.y, 1, WHITE);
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
