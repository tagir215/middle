#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "Inventory.h"
#include "LoopSociety.h"
#include "middle_shape_utils.h"
#include "Rectangle.h"
#include "Position.h"
#include "Offset.h"

class InventorySystem : public middle::MiddleGameplaySystem {

public:

	const float margin = 30.0f;

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			auto inventory = middle::getComponent<components::Inventory>(shape);
			if (!inventory)
				return true;

			auto loop = middle::getComponent<components::LoopSociety>(shape);
			std::vector<middle::Id>items = loop->loopMemberIds;

			auto offset = middle::getComponent<components::Offset>(shape);
			assert(offset);

			Vector3 inventoryPosition = middle::getShapePosition(gameState, shape.id.index);
			Vector3 cameraPos = gameState->activeCamera.position;
			Vector3 offsetVec = { offset->offsetX, offset->offsetY, offset->offsetZ };
			Vector3 displacement = (cameraPos + offsetVec) - inventoryPosition;

			middle::moveShape(gameState, shape.id.index, displacement);

			inventoryPosition = middle::getShapePosition(gameState, shape.id.index);


			auto inventoryRect = middle::getComponent<components::Rectangle>(shape);

			float totalWidth =  inventoryRect->width;
			float totalHeight = inventoryRect->height;

			float spacing = 0;

			Vector3 referencePos = inventoryPosition;
			if (inventory->horizontal) {
				spacing = totalWidth / (items.size() + 1);
				referencePos.x -= totalWidth * 0.5f - spacing;
			}
			else {
				spacing = totalHeight / (items.size() + 1);
				referencePos.z += totalHeight * 0.5f - spacing;
			}


			for (middle::Id& childId : items) {
				middle::Shape& child = middle::getShape(gameState, childId.index);

				Vector3 displacement = referencePos - middle::getShapePosition(gameState, child.id.index);
				middle::moveShape(gameState, child.id.index, displacement);
				if (inventory->horizontal) {
					referencePos.x += spacing;
				}
				else {
					referencePos.z -= spacing;
				}
			}


			return true;
			});
	}
};

static middle::SystemRegistrar<InventorySystem> reg("InventorySystem");
