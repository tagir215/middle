#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "Inventory.h"
#include "LoopSociety.h"
#include "middle_shape_utils.h"
#include "Rectangle.h"
#include "Position.h"
#include "Circle.h"
#include "Offset.h"

class InventorySystem : public middle::MiddleGameplaySystem {

public:
	InventorySystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	const float margin = 30.0f;

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			auto inventory = middle::getComponent<components::Inventory>(shape);
			if (!inventory)
				return;

			auto loop = middle::getComponent<components::LoopSociety>(shape);
			std::vector<middle::Id>items = loop->loopMemberIds;

			Vector3 inventoryPosition = middle::getShapePosition(gameState, shape.id.index);
			Vector3 cameraPos = gameState->activeCamera.position;

			auto offset = middle::getComponent<components::Offset>(shape);
			assert(offset);
			Vector3 inventoryTargetOffset = {offset->offsetX, offset->offsetY, offset->offsetZ};
			Vector3 targetPos = cameraPos + inventoryTargetOffset;
			middle::moveShape(gameState, shape.id.index, targetPos - inventoryPosition);

			const float zspacing = 10;
			const float xmargin = 4;

			auto inventoryRect = middle::getComponent<components::Rectangle>(shape);

			// calculate total height
			float leftX, rightX, bottomZ, topZ;
			middle::loopChildrenOnlyRectBoundingBox(gameState, shape.id, &leftX, &rightX, &bottomZ, &topZ);
			float totalWidth = rightX - leftX + margin;
			float totalHeight = topZ - bottomZ + margin;

			// TODO REMOVE
			if (totalWidth <= 0) {
				totalWidth = 10;
				totalHeight = 10;
			}

			inventoryRect->width = totalWidth;
			inventoryRect->height = totalHeight;

			Vector3 referencePos = inventoryPosition;
			referencePos.z += totalHeight * 0.5f + zspacing * 0.5f;
			referencePos.z -= margin * 0.5f;

			for (middle::Id& childId : items) {
				middle::Shape& child = middle::getShape(gameState, childId.index);
				auto rect = middle::getComponent<components::Rectangle>(child);
				auto circle = middle::getComponent<components::Circle>(child);
				auto position = middle::getComponent<components::Position>(child);
				if (rect) {
					referencePos.z -= rect->height * 0.5f + zspacing * 0.5f;
					position->posX = referencePos.x;
					position->posY = referencePos.y;
					position->posZ = referencePos.z;
					referencePos.z -= rect->height * 0.5f + zspacing * 0.5f;
				}
				if (circle) {
					referencePos.z -= circle->radius + zspacing * 0.5f;
					position->posX = referencePos.x;
					position->posY = referencePos.y;
					position->posZ = referencePos.z;
					referencePos.z -= circle->radius + zspacing * 0.5f;
				}
			}


			});
	}
};

static middle::SystemRegistrar<InventorySystem> reg("InventorySystem");
