#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Rectangle.h"
#include "LoopSociety.h"
#include "Position.h"
#include "CodeBlock.h"
#include "CodeFunction.h"
#include "IfComponent.h"
#include "Offset.h"
#include "InputVariable.h"
#include "OutputVariable.h"
#include "ScopeComponent.h"
#include "ProcedureComponent.h"
#include "Scale.h"

class CodeBlockLayoutSystem : public middle::MiddleGameplaySystem {
public:
	CodeBlockLayoutSystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	const float zmargin = 4;
	const float minScopeHeight = 40;
	const float minScopeWidth = 60;
	const float minCodeBlockHeight = 30;
	const float minCodeBlockWidth = 20;
	const float minFunctionHeight = 10;
	const float minFunctionWidth = 20;
	const float ifspacing = 20;
	const float blockMarginX = 20;
	const float blockMarginZ = 4;
	const float ifoffset = 70;
	const float variableSpacingZ = 10;
	const float variableSpacingX = 10;

	void updateRectSize(middle::GameState* gameState, middle::Shape& shape, float minW, float minH, float marginX, float marginY) {
		components::Rectangle* rect = middle::getComponent<components::Rectangle>(shape);
		components::Scale* scale = middle::getComponent<components::Scale>(shape);
		if (!rect) {
			return;
		}
		float left, right, bottom, top;
		middle::loopChildrenOnlyRectBoundingBox(gameState, shape.id, &left, &right, &bottom, &top);
		float newWidth = right - left;
		float newHeight = top - bottom;
		Vector3 s = { 1,1,1 };
		if (scale) {
			s = scale->scale;
		}
		if (newWidth < minW * s.x) {
			newWidth = minW;
		}
		if (newHeight < minH * s.z) {
			newHeight = minH;
		}
		rect->width = newWidth + marginX;
		rect->height = newHeight + marginY;
	}

	void updateRectSizeRecursive(middle::GameState* gameState, middle::Shape& shape) {
		auto scope = middle::getComponent<components::ScopeComponent>(shape);
		auto codeBlock = middle::getComponent<components::CodeBlock>(shape);
		auto function = middle::getComponent<components::CodeFunction>(shape);
		auto ifBlock = middle::getComponent<components::IfComponent>(shape);
		auto loop = middle::getComponent<components::LoopSociety>(shape);

		float marginX = 0;
		float marginZ = 0;
		if (scope) {
			marginX = minScopeWidth;
			marginZ = minScopeHeight;
		}
		if (codeBlock) {
			marginX = minCodeBlockWidth;
			marginZ = minCodeBlockHeight;
		}
		if (ifBlock) {
			return;
		}
		if (function) {
			return;
		}
		updateRectSize(gameState, shape, marginX, marginZ, blockMarginX * 2, blockMarginZ * 2);

		for (middle::Id& id : loop->loopMemberIds) {
			auto& child = middle::getShape(gameState, id.index);
			updateRectSizeRecursive(gameState, child);
		}
	}

	void update(middle::GameState* gameState) override {

		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			return;
		}

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {

			// move input variables to bubbles they reference
			auto inputVar = middle::getComponent<components::InputVariable>(shape);
			if (inputVar && inputVar->unitRef.index != middle::UNASSIGNED) {
				Vector3 currentPos = middle::getShapePosition(gameState, shape.id.index);
				Vector3 refShapePosition = middle::getShapePosition(gameState, inputVar->unitRef.index);
				middle::moveShape(gameState, shape.id.index, refShapePosition - currentPos);
			}

			// procedure layout
			auto procedure = middle::getComponent<components::ProcedureComponent>(shape);
			if (procedure) {
				Vector3 procPos = middle::getShapePosition(gameState, shape.id.index);
				auto procRect = middle::getComponent<components::Rectangle>(shape);
				Vector3 referencePos = procPos;
				referencePos.x -= procRect->width * 0.5f + variableSpacingX;
				referencePos.z += procRect->height * 0.5f - variableSpacingZ;

				auto loop = middle::getComponent<components::LoopSociety>(shape);
				for (middle::Id& childId : loop->loopMemberIds) {
					auto& childShape = middle::getShape(gameState, childId.index);
					auto inputVariable = middle::getComponent<components::InputVariable>(childShape);
					if (!inputVariable)
						continue;

					auto inputPos = middle::getComponent<components::Position>(childShape);
					inputPos->posX = referencePos.x;
					inputPos->posY = referencePos.y;
					inputPos->posZ = referencePos.z;
					referencePos.z -= variableSpacingZ;
				}
			}

			// scope block layout
			auto scope = middle::getComponent<components::ScopeComponent>(shape);
			if (scope) {

				updateRectSizeRecursive(gameState, shape);

				auto scopeRect = middle::getComponent<components::Rectangle>(shape);
				auto scopeLoop = middle::getComponent<components::LoopSociety>(shape);
				auto scopePosition = middle::getComponent<components::Position>(shape);

				float totalHeight = scopeRect->height;
				float totalWidth = scopeRect->width;
				int size = scopeLoop->loopMemberIds.size();

				Vector3 scopePos = { scopePosition->posX, scopePosition->posY, scopePosition->posZ };
				Vector3 referencePos = scopePos;
				Vector3 scopeScale = middle::getTotalScale(gameState, shape.id);

				// move code block inside procedure container
				for (int index = 0; index < size; ++index) {
					middle::Id childId = scopeLoop->loopMemberIds[index];
					auto& childShape = middle::getShape(gameState, childId.index);
					auto childRect = middle::getComponent<components::Rectangle>(childShape);
					if (!childRect) {
						continue;
					}
					auto position = middle::getComponent<components::Position>(childShape);
					Vector3 currPos = { position->posX, position->posY, position->posZ };

					if (index > 0)
						referencePos.z -= childRect->height * 0.5f + zmargin * 0.5f;
					position->posX = referencePos.x;
					position->posY = referencePos.y;
					position->posZ = referencePos.z;
					referencePos.z -= childRect->height * 0.5f + zmargin * 0.5f;

					auto offset = middle::getComponent<components::Offset>(childShape);
					if (offset) {
						offset->offsetX = childRect->width * 0.5f * scopeScale.x;
					}
				}
				// move scope container
				if (size > 0) {
					auto& child0Shape = middle::getShape(gameState, scopeLoop->loopMemberIds[0].index);
					Vector3 child0Pos = middle::getShapePosition(gameState, scopeLoop->loopMemberIds[0].index);
					auto child0Rect = middle::getComponent<components::Rectangle>(child0Shape);

					float left, right, bottom, top;
					middle::loopChildrenOnlyRectBoundingBox(gameState, shape.id, &left, &right, &bottom, &top);
					Vector3 center = { (left + right) * 0.5f, 0, (bottom + top) * 0.5f };

					auto scopeOffset = middle::getComponent<components::Offset>(shape);
					scopeOffset->offsetX = (center.x - scopePos.x) * scopeScale.x;
					scopeOffset->offsetY = (center.y - scopePos.y) * scopeScale.y;
					scopeOffset->offsetZ = (center.z - scopePos.z) * scopeScale.z;
				}
			}

			// code block layout
			auto codeBlock = middle::getComponent<components::CodeBlock>(shape);
			if (codeBlock) {
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				float left, right, bottom, top;

				auto codeBlockRect = middle::getComponent < components::Rectangle>(shape);
				Vector3 codeBlockPos = middle::getShapePosition(gameState, shape.id.index);
				float leftAlignmentX = codeBlockPos.x- codeBlockRect->width * 0.5f;
				float targetX = leftAlignmentX + blockMarginX;
				float targetZ = codeBlockPos.z;

				for (middle::Id& childId : loop->loopMemberIds) {
					auto& childShape = middle::getShape(gameState, childId.index);
					auto childRect = middle::getComponent<components::Rectangle>(childShape);
					auto pos = middle::getComponent<components::Position>(childShape);
					auto offset = middle::getComponent<components::Offset>(childShape);
					pos->posX = targetX + childRect->width * 0.5f;
					pos->posZ = targetZ;
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

				Vector3 targetScale = { 1,1,1 };
				if (loop->parentLoopId.index != middle::UNASSIGNED) {
					auto& parentShape = middle::getShape(gameState, loop->parentLoopId.index);
					auto parentCodeBlock = middle::getComponent<components::CodeBlock>(parentShape);
					if (!parentCodeBlock) {
						targetScale = { 0.1f,0.1f,0.1f };
					}
				}

				float spacingZ = ifspacing * targetScale.z;

				float totalHeight = 0;
				for (middle::Id& childId : loop->loopMemberIds) {
					auto& childShape = middle::getShape(gameState, childId.index);
					auto rect = middle::getComponent<components::Rectangle>(childShape);
					if (rect) {
						totalHeight += rect->height * targetScale.z;
					}
				}
				totalHeight += spacingZ;

				Vector3 referencePos = ifPosition + Vector3{ ifoffset * targetScale.x,0, totalHeight * 0.5f };

				for (middle::Id& id : loop->loopMemberIds) {
					auto& childShape = middle::getShape(gameState, id.index);
					auto childRect = middle::getComponent<components::Rectangle>(childShape);
					auto childScale = middle::getComponent<components::Scale>(childShape);
					childScale->scale = targetScale;
					referencePos.z -= childRect->height * 0.5f * targetScale.z;

					Vector3 currentPos = middle::getShapePosition(gameState, id.index);
					float sizeOffset = childRect->width * 0.5f * targetScale.x;

					Vector3 targetPos = referencePos;
					targetPos.x += sizeOffset;
					middle::moveShape(gameState, id.index, targetPos - currentPos);
					referencePos.z = referencePos.z - childRect->height * 0.5f * targetScale.z - spacingZ;
				}
			}

			// function and its variables layout
			auto function = middle::getComponent<components::CodeFunction>(shape);
			if (function) {
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				int inputChildCount = 0;
				for (middle::Id& childId : loop->loopMemberIds) {
					auto& childShape = middle::getShape(gameState, childId.index);
					auto input = middle::getComponent<components::InputVariable>(childShape);
					if (input) {
						++inputChildCount;
					}
				}

				if (inputChildCount > 0) {
					auto funcRect = middle::getComponent<components::Rectangle>(shape);
					Vector3 funcPosition = middle::getShapePosition(gameState, shape.id.index);

					Vector3 referencePosRight = funcPosition;
					referencePosRight.x += funcRect->width * 0.5f + variableSpacingX;

					Vector3 referencePosLeft = funcPosition;
					float totalHeight = (inputChildCount - 1) * variableSpacingZ;
					referencePosLeft.x -= funcRect->width * 0.5f + variableSpacingX;
					referencePosLeft.z += totalHeight * 0.5f;

					for (middle::Id& childId : loop->loopMemberIds) {
						auto& childShape = middle::getShape(gameState, childId.index);
						auto input = middle::getComponent<components::InputVariable>(childShape);
						auto output = middle::getComponent<components::OutputVariable>(childShape);
						auto pos = middle::getComponent<components::Position>(childShape);
						if (input) {
							pos->posX = referencePosLeft.x;
							pos->posY = referencePosLeft.y;
							pos->posZ = referencePosLeft.z;
							referencePosLeft.z -= variableSpacingZ;
						}
						if (output) {
							pos->posX = referencePosRight.x;
							pos->posY = referencePosRight.y;
							pos->posZ = referencePosRight.z;
						}
					}
				}

			}

			});
	}
};

static middle::SystemRegistrar<CodeBlockLayoutSystem> reg("CodeBlockLayoutSystem");
