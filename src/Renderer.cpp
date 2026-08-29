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

	void DrawCustomCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, int segments, Color color) {
		// Custom implementation using rlgl for variable segment counts
		rlPushMatrix();
		rlTranslatef(center.x, center.y, center.z);
		rlRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
		rlBegin(RL_LINES);
		for (int i = 0; i < 360; i += 360 / segments) {
			rlColor4ub(color.r, color.g, color.b, color.a);
			// Calculate vertices for line segments based on input 'segments'
			rlVertex3f(sinf(DEG2RAD * i) * radius, cosf(DEG2RAD * i) * radius, 0.0f);
			rlVertex3f(sinf(DEG2RAD * (i + 360 / segments)) * radius, cosf(DEG2RAD * (i + 360 / segments)) * radius, 0.0f);
		}
		rlEnd();
		rlPopMatrix();
	}

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

	void draw3D(middle::GameState* gameState, bool disabledDepthTest, int layerPass = 0) {

		if (gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE) {
			// center indicator
			DrawCube({ 0,5,0 }, 5, 5, 5, BLACK);
		}

		rlSetClipPlanes(gameState->nearPlaneDistance, gameState->farPlaneDistance);

		for (int i = 0; i < gameState->renderData.size(); ++i) {
			middle::RenderItem item = gameState->renderData[i];

			if (disabledDepthTest && !item.disableDepthTest) {
				continue;
			}
			if (!disabledDepthTest && item.disableDepthTest) {
				continue;
			}
			if (item.disableDepthTest && item.layer != layerPass) {
				continue;
			}

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
					DrawCube(item.center, item.width, item.length, item.height, item.backgroundColor);
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
				DrawCustomCircle3D(item.center, item.radius, { 1,0,0 }, 90, item.slices, item.color);
				if (item.backgroundColor.a != 0) {
					DrawCylinder(item.center, item.radius, item.radius, 0.000000001f, item.slices, item.backgroundColor);
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

			if (item.type == middle::BILLBOARD) {
				// billboard default angle is toward y,  
				item.transform.rotation = QuaternionFromVector3ToVector3({ 0,-1,0 }, { 0, 0, -1 });
				Matrix M = transformMatrix(item.transform, item.layer);
				rlPushMatrix();
				rlLoadIdentity();
				rlMultMatrixf(MatrixToFloatV(M).v);
				Vector3 pos = item.center;
				if (item.texture == nullptr) {
					DrawCube(pos, 4, 4, 4, BLACK);
				}
				else {
					if(item.shader)
						BeginShaderMode(*item.shader);
					DrawBillboard(gameState->activeCamera, *item.texture, pos, item.textureScale, item.color);
					if(item.shader)
						EndShaderMode();
				}
				rlPopMatrix();
			}

			if (item.type == middle::BACKGROUND) {
				// billboard default angle is toward y,  
				item.transform.rotation = QuaternionFromVector3ToVector3({ 0,-1,0 }, { 0, 0, -1 });
				Matrix M = transformMatrix(item.transform, item.layer);
				rlPushMatrix();
				rlLoadIdentity();
				rlMultMatrixf(MatrixToFloatV(M).v);
				Vector3 pos = item.center;
				Rectangle backgroundRect = { 0,0, item.width, item.height };
				Vector3 up = { 0,1,0 };
				Vector2 scale = { item.textureScale, item.textureScale };
				Vector2 origin = { scale.x * 0.5f, scale.y * 0.5f };
				float rotation = 0;
				if (item.texture == nullptr) {
					DrawCube(pos, 4, 4, 4, BLACK);
				}
				else {
					DrawBillboardPro(gameState->activeCamera, *item.texture, backgroundRect, pos, up, scale, origin, rotation, item.color);
				}
				rlPopMatrix();
			}

		}


		EndMode3D();
	}

	void drawText(middle::GameState* gameState, bool uiText) {
		for (int i = 0; i < gameState->renderData.size(); ++i) {
			middle::RenderItem item = gameState->renderData[i];
			if (item.disableDepthTest != uiText) {
				continue;
			}

			if (item.type == middle::RenderItemType::TEXT) {
				const int spacing = 1;
				float distFactor = 1 / Vector3Distance(gameState->activeCamera.position, item.transform.translation);
				float fontFactor = gameState->fontUnitFactor * distFactor;
				float scaledFontSize = item.fontSize * item.transform.scale.x * fontFactor;
				Vector2 rect = MeasureTextEx(gameState->globalFont, item.text.c_str(), scaledFontSize, spacing);
				Vector2 offset = { -rect.x * 0.5f, -rect.y * 0.5f };

				Vector2 pos = GetWorldToScreen(item.transform.translation, gameState->activeCamera);

				DrawTextEx(gameState->globalFont, item.text.c_str(), pos + offset, scaledFontSize, spacing, item.color);
			}

		}
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
			draw3D(gameState, false);
			EndMode3D();

			drawText(gameState, false);

			int maxLayers = 7;
			for (int i = -1; i < maxLayers; ++i) {
				BeginMode3D(camera);
				rlDisableDepthTest();
				draw3D(gameState, true, i);
				rlEnableDepthTest();
				EndMode3D();
			}

			drawText(gameState, true);

			Vector3 center = { 0,0,0 };
			Vector2 center2d = GetWorldToScreen(center, camera);
			if (gameState->sceneNames.size() > 0 && gameState->applicationMode == middle::ApplicationMode::EDITOR_MODE) {
				DrawText(gameState->activeSceneName.c_str(), center2d.x, center2d.y, 1, WHITE);
			}

			if (!gameState->releaseBuild) {
				rlImGuiBegin();

				while (gameState->uiSetups.size() > 0) {
					gameState->uiSetups.back()();
					gameState->uiSetups.pop_back();
				}

				rlImGuiEnd();
			}
			else {
				gameState->uiSetups.clear();
			}

			EndDrawing();

			gameState->renderData.clear();
		}
	};

	static middle::SystemRegistrar<RendererSystem> reg(scriptName);

}
