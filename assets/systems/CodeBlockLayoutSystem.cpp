#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "ProcedureComponent.h"
#include "Rectangle.h"
#include "LoopSociety.h"
#include "Position.h"
#include "CodeBlock.h"
#include "CodeFunction.h"
#include "IfComponent.h"

class CodeBlockLayoutSystem : public middle::MiddleGameplaySystem {
public:
	CodeBlockLayoutSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	const float zmargin = 4;
	const float minProcedureHeight = 30;
	const float minProcedureWidth = 60;
	const float minCodeBlockHeight = 20;
	const float minCodeBlockWidth = 40;
	const float minFunctionHeight = 10;
	const float minFunctionWidth = 20;
	const float ifmargin = 20;
	const float ifoffset = 70;

	void updateRectSize(middle::GameState* gameState, middle::Shape& shape, float minW, float minH) {
		components::Rectangle* rect = middle::getComponent<components::Rectangle>(shape);
		rect->width = 0;
		rect->height = 0;
		float left = 100000; float right = -100000; float bottom = left; float top = right;
		middle::loopRectBoundingBox(gameState, shape.id, &left, &right, &bottom, &top);
		float newWidth = right - left + 5;
		float newHeight = top - bottom;
		if (newWidth < minW) {
			newWidth = minW;
		}
		if (newHeight < minH) {
			newHeight = minH;
		}
		rect->width = newWidth;
		rect->height = newHeight;

		auto pos = middle::getComponent<components::Position>(shape);
		pos->posX = (left + right) * 0.5f;
		pos->posY = 0;
		pos->posZ = (top + bottom) * 0.5f;
	}

	void updateRectSizeRecursive(middle::GameState* gameState, middle::Shape& shape) {
		auto procedure = middle::getComponent<components::ProcedureComponent>(shape);
		auto codeBlock = middle::getComponent<components::CodeBlock>(shape);
		auto function = middle::getComponent<components::CodeFunction>(shape);
		auto loop = middle::getComponent<components::LoopSociety>(shape);

		float minW = 0;
		float minH = 0;
		if (procedure) {
			minW = minProcedureWidth;
			minH = minProcedureHeight;
		}
		if (codeBlock) {
			minW = minCodeBlockWidth;
			minH = minCodeBlockHeight;
		}
		if (function) {
			return;
		}
		updateRectSize(gameState, shape, minW, minH);

		for (middle::Id& id : loop->loopMemberIds) {
			auto& child = middle::getShape(gameState, id.index);
			updateRectSizeRecursive(gameState, child);
		}
	}

	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {


			// procedure block child position and size updates
			auto procedure = middle::getComponent<components::ProcedureComponent>(shape);
			if (procedure) {

				updateRectSizeRecursive(gameState, shape);

				auto procRect = middle::getComponent<components::Rectangle>(shape);
				auto procLoop = middle::getComponent<components::LoopSociety>(shape);

				float totalHeight = procRect->height;
				float totalWidth = procRect->width;
				int size = procLoop->loopMemberIds.size();

				Vector3 referencePos;
				if (size > 1) {
					referencePos = middle::getShapePosition(gameState, procLoop->loopMemberIds[0].index);
				}
				else {
					referencePos = middle::getShapePosition(gameState, shape.id.index);
				}
				// move code block inside procedure container
				for (int index = 0; index < size; ++index) {
					middle::Id childId = procLoop->loopMemberIds[index];
					auto& childShape = middle::getShape(gameState, childId.index);
					auto position = middle::getComponent<components::Position>(childShape);
					auto childRect = middle::getComponent<components::Rectangle>(childShape);
					Vector3 currPos = { position->posX, position->posY, position->posZ };

					if(index > 0)
						referencePos.z -= childRect->height * 0.5f + zmargin * 0.5f;
					middle::moveShape(gameState, childId.index, referencePos - currPos);
					referencePos.z -= childRect->height * 0.5f + zmargin * 0.5f;
				}
				// move procedure container
				if (size > 1) {
					auto& child0Shape = middle::getShape(gameState, procLoop->loopMemberIds[0].index);
					Vector3 child0Pos = middle::getShapePosition(gameState, procLoop->loopMemberIds[0].index);
					auto child0Rect = middle::getComponent<components::Rectangle>(child0Shape);

					float z = child0Pos.z + child0Rect->height * 0.5f;
					float centerZ = z - totalHeight * 0.5f;

					Vector3 center = { child0Pos.x, child0Pos.y, centerZ };

					auto procPosition = middle::getComponent<components::Position>(shape);
					//procPosition->posX = center.x;
					procPosition->posY = center.y;
					procPosition->posZ = center.z;
				}
			}

			// code block size and position updates
			auto containerCodeBlock = middle::getComponent<components::CodeBlock>(shape);
			if (containerCodeBlock) {
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				auto codeBlockRect = middle::getComponent<components::Rectangle>(shape);
				const float zmargin = 4;
				Vector3 targetPos = middle::getShapePosition(gameState, shape.id.index);

				if (loop->loopMemberIds.size() > 0) {
					assert(loop->loopMemberIds.size() == 1);
					auto& childShape = middle::getShape(gameState, loop->loopMemberIds[0].index);
					Vector3 childPos = middle::getShapePosition(gameState, childShape.id.index);
					//middle::moveShape(gameState, childShape.id.index, targetPos - childPos);
				}
			}


			// if block layout
			auto ifBlock = middle::getComponent<components::IfComponent>(shape);
			if (ifBlock) {
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				assert(loop->loopMemberIds.size() == 2);
				middle::Id childA = loop->loopMemberIds[0];
				middle::Id childB = loop->loopMemberIds[1];

				Vector3 ifPosition = middle::getShapePosition(gameState, shape.id.index);

				float left = 100000; float right = -100000; float bottom = left; float top = right;
				middle::loopRectBoundingBox(gameState, shape.id, &left, &right, &bottom, &top);
				float totalHeight = top - bottom;

				Vector3 referencePos = ifPosition + Vector3{ ifoffset,0,totalHeight * 0.5f };
				for (middle::Id& id : loop->loopMemberIds) {
					auto& childShape = middle::getShape(gameState, id.index);
					auto childRect = middle::getComponent<components::Rectangle>(childShape);
					referencePos.z = referencePos.z - childRect->height * 0.5f;
					Vector3 targetPos = referencePos;
					Vector3 currentPos = middle::getShapePosition(gameState, id.index);
					middle::moveShape(gameState, id.index, targetPos - currentPos);
					referencePos.z = referencePos.z - childRect->height * 0.5f - ifmargin;
				}
			}

			});
	}
};

static middle::SystemRegistrar<CodeBlockLayoutSystem> reg("CodeBlockLayoutSystem");
