#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "Inventory.h"
#include "LoopSociety.h"
#include "middle_shape_utils.h"
#include "Rectangle.h"
#include "Position.h"

class InventorySystem : public middle::MiddleGameplaySystem {

public:
	InventorySystem() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto inventory = middle::getComponent<components::Inventory>(shape);
			if (!inventory)
				return;

			auto loop = middle::getComponent<components::LoopSociety>(shape);
			std::vector<middle::Id>items;
			middle::getChildren(gameState, shape.id, items);

			Vector3 inventoryPosition = middle::getShapePosition(gameState, shape.id.index);
			const float ymargin = 10;
			const float xmargin = 4;

			// calculate total height
			float totalHeight = 0;
			float totalWidth = 0;
			for (middle::Id& childId : items) {
				middle::Shape& child = middle::getShape(gameState, childId.index);
				auto rect = middle::getComponent<components::Rectangle>(child);
				totalHeight += rect->height + ymargin;
				totalWidth = rect->width + xmargin > totalWidth ? rect->width + xmargin : totalWidth;
			}

			Vector3 pos = inventoryPosition;
			pos.z += totalHeight * 0.5f;
			auto inventoryRect = middle::getComponent<components::Rectangle>(shape);
			inventoryRect->width = totalWidth;
			inventoryRect->height = totalHeight;

			for (middle::Id& childId : items) {
				middle::Shape& child = middle::getShape(gameState, childId.index);
				auto rect = middle::getComponent<components::Rectangle>(child);
				auto position = middle::getComponent<components::Position>(child);
				pos.z -= rect->height * 0.5f + ymargin * 0.5f;
				position->posX = pos.x;
				position->posY = pos.y;
				position->posZ = pos.z;
				pos.z -= rect->height * 0.5f + ymargin * 0.5f;
			}

			});
	}
};

static middle::SystemRegistrar<InventorySystem> reg("InventorySystem");
