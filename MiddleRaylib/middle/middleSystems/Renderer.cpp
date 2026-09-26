#pragma once
#include "raylib.h"
#include <raymath.h>
#include "middle_system_registrar.h"
#include "rlImGui.h"
#include "imgui.h"
#include <cstdlib>
#include <thread>
#include "rlgl.h"
#include "middle_math.h"
#include "middle_state.h"
#include "middle_math_maping_helper.h"

const int fontUnitFactor = 1024;

namespace renderer {

	const float layerGap = -1.0f;

	static void DrawCustomCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, int segments, Color color) {
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

	static Matrix transformMatrix(Transform& transform, int layer) {
		Matrix S = MatrixScale(transform.scale.x, transform.scale.y, transform.scale.z);
		Matrix R = QuaternionToMatrix(transform.rotation);
		Matrix T = MatrixTranslate(transform.translation.x, transform.translation.y + layer * layerGap, transform.translation.z);
		Matrix M = MatrixMultiply(MatrixMultiply(S, R), T);
		return M;
	}

	static std::vector<Vector3> getRectVertices(const midMath::Vector3& pos, const midMath::Vector3& scale, float width, float height)
	{
		Vector3 s = toRVec(scale);
		std::vector<Vector3> vertices;
		vertices.resize(4);
		vertices[0] = { -width * 0.5f * s.x, 0, height * 0.5f * s.z };
		vertices[1] = { -width * 0.5f * s.x, 0, -height * 0.5f * s.z };
		vertices[2] = { width * 0.5f * s.x, 0, -height * 0.5f * s.z };
		vertices[3] = { width * 0.5f * s.x, 0, height * 0.5f * s.z };
		Vector3 rpos = toRVec(pos);
		vertices[0] += rpos;
		vertices[1] += rpos;
		vertices[2] += rpos;
		vertices[3] += rpos;
		return vertices;
	}

	static void drawRect(const std::vector<Vector3>& vertices, const midPrimitive::Color& color) {
		Color rcolor = toRColor(color);
		DrawLine3D(vertices[0], vertices[1], rcolor);
		DrawLine3D(vertices[1], vertices[2], rcolor);
		DrawLine3D(vertices[2], vertices[3], rcolor);
		DrawLine3D(vertices[3], vertices[0], rcolor);
	}

	static void draw3D(const middle::MiddleOutputState* const middleState, bool disabledDepthTest, const Camera& const camera, const std::vector<Shader>& shaders, const std::vector<Texture>& textures, int layerPass = 0) {


		rlSetClipPlanes(middleState->nearPlaneDistance, middleState->farPlaneDistance);

		for (int i = 0; i < middleState->renderData.size(); ++i) {
			middle::RenderItem item = middleState->renderData[i];

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
				Vector3 pos = toRVec(item.center);
				DrawSphereEx(pos, item.radius, 5, 5, toRColor(item.color));
			}


			if (item.type == middle::RenderItemType::RECTANGLE) {
				Matrix M = transformMatrix(toRTransform(item.transform), item.layer);
				rlPushMatrix();
				rlMultMatrixf(MatrixToFloatV(M).v);
				Vector3 pos = toRVec(item.center);
				pos.y += item.layer * layerGap;
				if (item.color.a != 0) {
					std::vector<Vector3>vertices = getRectVertices(item.center, item.scale, item.width, item.height);
					drawRect(vertices, item.color);
				}
				if (item.backgroundColor.a != 0) {
					DrawCube(toRVec(item.center), item.width, item.length, item.height, toRColor(item.backgroundColor));
				}
				rlPopMatrix();
			}

			if (item.type == middle::RenderItemType::CYLINDER) {
				Matrix M = transformMatrix(toRTransform(item.transform), item.layer);
				rlPushMatrix();
				rlMultMatrixf(MatrixToFloatV(M).v);
				Vector3 pos = toRVec(item.center);
				DrawCylinder(pos, item.radius, item.ringRadius, item.length, 23, toRColor(item.color));
				rlPopMatrix();
			}

			if (item.type == middle::RenderItemType::LINE) {
				Vector3 posA = toRVec(item.linePointA) + Vector3{ 0, item.layer * layerGap, 0 };
				Vector3 posB = toRVec(item.linePointB) + Vector3{ 0, item.layer * layerGap, 0 };
				DrawLine3D(posA, posB, toRColor(item.color));
			}

			if (item.type == middle::CIRCLE) {
				Matrix M = transformMatrix(toRTransform(item.transform), item.layer);
				rlPushMatrix();
				rlLoadIdentity();
				rlMultMatrixf(MatrixToFloatV(M).v);
				DrawCustomCircle3D(toRVec(item.center), item.radius, { 1,0,0 }, 90, item.slices, toRColor(item.color));
				if (item.backgroundColor.a != 0) {
					DrawCylinder(toRVec(item.center), item.radius, item.radius, 0.000000001f, item.slices, toRColor(item.backgroundColor));
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
						Vector3 pos = toRVec(item.center) + Vector3Scale(vr, item.radius);
						if (i > 0) {
							Transform transform;
							Vector3 dir = Vector3Normalize(Vector3Subtract(lastPos, pos));
							transform.rotation = QuaternionFromVector3ToVector3({ 0,-1,0 }, dir);
							transform.translation = lastPos;
							transform.scale = { 1,1,1 };
							float length = Vector3Distance(lastPos, pos);
							Matrix M = transformMatrix(transform, item.layer);
							DrawLine3D(lastPos, pos, toRColor(item.color));
							rlPushMatrix();
							rlLoadIdentity();
							rlMultMatrixf(MatrixToFloatV(M).v);
							int slices = 10;
							DrawCylinder({ 0,0,0 }, item.ringRadius, item.ringRadius, length, slices, toRColor(item.color));
							rlPopMatrix();
						}
						lastPos = pos;
					}
				}
			}

			if (item.type == middle::CUBOID) {
				Matrix M = transformMatrix(toRTransform(item.transform), item.layer);
				rlPushMatrix();
				rlLoadIdentity();
				rlMultMatrixf(MatrixToFloatV(M).v);
				Vector3 pos = toRVec(item.center);
				DrawCube(pos, item.width, item.height, item.length, toRColor(item.color));
				rlPopMatrix();
			}

			if (item.type == middle::BILLBOARD) {
				// billboard default angle is toward y,  
				Quaternion rotation = QuaternionFromVector3ToVector3({ 0,-1,0 }, { 0, 0, -1 });
				item.transform.rotation = { rotation.x, rotation.y, rotation.z, rotation.w };
				Matrix M = transformMatrix(toRTransform(item.transform), item.layer);
				rlPushMatrix();
				rlLoadIdentity();
				rlMultMatrixf(MatrixToFloatV(M).v);
				Vector3 pos = toRVec(item.center);
				if (item.texture == middleAssets::TEXTURE::TEXTURE_NONE) {
					DrawCube(pos, 4, 4, 4, BLACK);
				}
				else {
					if (item.shader != middleAssets::SHADER::SHADER_NONE)
						BeginShaderMode(shaders[item.shader]);

					DrawBillboard(camera, textures[item.texture], pos, item.textureScale, toRColor(item.color));

					if (item.shader != middleAssets::SHADER::SHADER_NONE)
						EndShaderMode();
				}
				rlPopMatrix();
			}

			if (item.type == middle::BACKGROUND) {
				// billboard default angle is toward y,  
				Quaternion rot = QuaternionFromVector3ToVector3({ 0,-1,0 }, { 0, 0, -1 });
				item.transform.rotation = { rot.x, rot.y, rot.z, rot.w };
				Matrix M = transformMatrix(toRTransform(item.transform), item.layer);
				rlPushMatrix();
				rlLoadIdentity();
				rlMultMatrixf(MatrixToFloatV(M).v);
				Vector3 pos = toRVec(item.center);
				Rectangle backgroundRect = { 0,0, item.width, item.height };
				Vector3 up = { 0,1,0 };
				Vector2 scale = { item.textureScale, item.textureScale };
				Vector2 origin = { scale.x * 0.5f, scale.y * 0.5f };
				float rotation = 0;
				if (item.texture == middleAssets::TEXTURE_NONE) {
					DrawCube(pos, 4, 4, 4, BLACK);
				}
				else {
					DrawBillboardPro(camera, textures[item.texture], backgroundRect, pos, up, scale, origin, rotation, toRColor(item.color));
				}
				rlPopMatrix();
			}

		}

		EndMode3D();
	}

	static void drawText(const middle::MiddleOutputState* const middleState, bool uiText, const Font& font, const Camera& camera) {
		for (int i = 0; i < middleState->renderData.size(); ++i) {
			const middle::RenderItem& item = middleState->renderData[i];
			if (item.disableDepthTest != uiText) {
				continue;
			}

			if (item.type == middle::RenderItemType::TEXT) {
				const int spacing = 0;
				float yDistance = std::abs(middleState->activeCamera.position.y - item.transform.translation.y);
				//float distFactor = 1 / Vector3Distance(gameState->activeCamera.position, item.transform.translation);
				float distFactor = 1 / yDistance;
				float fontFactor = fontUnitFactor * distFactor;
				float scaledFontSize = item.fontSize * item.transform.scale.x * fontFactor;
				Vector2 rect = MeasureTextEx(font, item.text.c_str(), scaledFontSize, spacing);
				Vector2 offset = { -rect.x * 0.5f, -rect.y * 0.5f };

				Vector2 pos = GetWorldToScreen(toRVec(item.transform.translation), camera);

				DrawTextEx(font, item.text.c_str(), pos + offset, scaledFontSize, spacing, toRColor(item.color));
			}

		}
	}

	class RendererSystem {
	public:
		static void update(const middle::MiddleOutputState* const middleState, const Font& font, const std::vector<Shader>& shaders, const std::vector<Texture>& textures, bool releaseBuild)  {

			BeginDrawing();

			// 89, 135, 168
			ClearBackground(toRColor(middleState->backgroundColor));

			Camera camera = toRCam(middleState->activeCamera);

			BeginMode3D(camera);
			draw3D(middleState, false, camera, shaders, textures);
			EndMode3D();

			drawText(middleState, false, font, camera);

			int maxLayers = 7;
			for (int i = -1; i < maxLayers; ++i) {
				BeginMode3D(camera);
				rlDisableDepthTest();
				draw3D(middleState, true, camera, shaders, textures, i);
				rlEnableDepthTest();
				EndMode3D();
			}

			SetTextLineSpacing(0);

			drawText(middleState, true, font, camera);

			Vector3 center = { 0,0,0 };
			Vector2 center2d = GetWorldToScreen(center, camera);

			if (!releaseBuild) {
				rlImGuiBegin();

				for (const auto& ui : middleState->uiSetups) {
					ui();
				}

				rlImGuiEnd();
			}

			EndDrawing();
		}
	};

}
