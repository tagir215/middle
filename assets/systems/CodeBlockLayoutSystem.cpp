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

	float calculateTotalHeight(middle::GameState* gameState, components::LoopSociety* loop, float zmargin, float minHeight) {
		float totalHeight = 0;
		for (middle::Id& childId : loop->loopMemberIds) {
			auto& childShape = middle::getShape(gameState, childId.index);
			auto childRect = middle::getComponent<components::Rectangle>(childShape);
			totalHeight += childRect->height + zmargin;
		}
		return totalHeight > minHeight ? totalHeight : minHeight;
	}

	const float zmargin = 4;
	const float minHeight = 40;
	const float ifmargin = 20;
	const float ifoffset = 70;

	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {

			// procedure block child position updates
			auto procedure = middle::getComponent<components::ProcedureComponent>(shape);
			if (procedure) {
				auto rectangle = middle::getComponent<components::Rectangle>(shape);
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				float totalHeight = calculateTotalHeight(gameState, loop, zmargin, minHeight);
				rectangle->height = totalHeight;

				Vector3 referencePos;
				int size = loop->loopMemberIds.size();
				if (size > 1) {
					referencePos = middle::getShapePosition(gameState, loop->loopMemberIds[0].index);
				}
				else {
					referencePos = middle::getShapePosition(gameState, shape.id.index);
				}
				// move code block inside procedure container
				for (middle::Id& childId : loop->loopMemberIds) {
					auto& childShape = middle::getShape(gameState, childId.index);
					auto position = middle::getComponent<components::Position>(childShape);
					auto childRect = middle::getComponent<components::Rectangle>(childShape);
					Vector3 currPos = { position->posX, position->posY, position->posZ };
					middle::moveShape(gameState, childId.index, referencePos - currPos);
					referencePos.z -= childRect->height + zmargin;
				}
				// move procedure container
				if (size > 1) {
					auto& child0Shape = middle::getShape(gameState, loop->loopMemberIds[0].index);
					Vector3 child0Pos = middle::getShapePosition(gameState, loop->loopMemberIds[0].index);
					auto child0Rect = middle::getComponent<components::Rectangle>(child0Shape);

					Vector3 top = child0Pos + Vector3{ 0,0,child0Rect->height * 0.5f };
					Vector3 toCenter = Vector3{ 0,0,-totalHeight * 0.5f + zmargin * 0.5f };
					Vector3 center = top + toCenter;

					auto procPosition = middle::getComponent<components::Position>(shape);
					procPosition->posX = center.x;
					procPosition->posY = center.y;
					procPosition->posZ = center.z;
				}
			}

			// moving procedurefunctions
			auto containerCodeBlock = middle::getComponent<components::CodeBlock>(shape);
			if (containerCodeBlock) {
				auto rectangle = middle::getComponent<components::Rectangle>(shape);
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				float totalHeight = 0;
				const float zmargin = 4;
				Vector3 targetPos = middle::getShapePosition(gameState, shape.id.index);

				if (loop->loopMemberIds.size() > 0) {
					assert(loop->loopMemberIds.size() == 1);
					auto& childShape = middle::getShape(gameState, loop->loopMemberIds[0].index);
					auto childRect = middle::getComponent<components::Rectangle>(childShape);
					Vector3 childPos = middle::getShapePosition(gameState, childShape.id.index);
					middle::moveShape(gameState, childShape.id.index, targetPos - childPos);
				}
			}

			// moving functions
			auto ifBlock = middle::getComponent<components::IfComponent>(shape);
			if (ifBlock) {
				auto loop = middle::getComponent<components::LoopSociety>(shape);
				assert(loop->loopMemberIds.size() == 2);
				middle::Id childA = loop->loopMemberIds[0];
				middle::Id childB = loop->loopMemberIds[1];
				float totalHeight = calculateTotalHeight(gameState, loop, ifmargin, minHeight);
				Vector3 ifPosition = middle::getShapePosition(gameState, shape.id.index);

				if (loop->parentLoopId.index != middle::UNASSIGNED) {
					middle::Shape& parentShape = middle::getShape(gameState, loop->parentLoopId.index);
					auto parentRect = middle::getComponent<components::Rectangle>(parentShape);
					parentRect->height = totalHeight;
				}

				Vector3 referencePos = ifPosition + Vector3{ ifoffset,0,totalHeight * 0.5f };
				for (middle::Id& id : loop->loopMemberIds) {
					auto& childShape = middle::getShape(gameState, id.index);
					auto childRect = middle::getComponent<components::Rectangle>(childShape);
					referencePos.z = referencePos.z - childRect->height * 0.5f - ifmargin * 0.5f;
					Vector3 targetPos = referencePos;
					Vector3 currentPos = middle::getShapePosition(gameState, id.index);
					middle::moveShape(gameState, id.index, targetPos - currentPos);
					referencePos.z = referencePos.z - childRect->height * 0.5f - ifmargin * 0.5f;
				}
			}

			});
	}
};

static middle::SystemRegistrar<CodeBlockLayoutSystem> reg("CodeBlockLayoutSystem");
