#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "Inventory.h"
#include "LoopSociety.h"
#include "middle_shape_utils.h"
#include "Rectangle.h"
#include "Position.h"
#include "Offset.h"
#include "editor_file_utils.h"

class InventorySystem : public middle::MiddleGameplaySystem {

public:
	components::CompCache* inventoryCache;

	void init(middle::GameState* gameState) {
		inventoryCache = middle::newCompCache(gameState);
		inventoryCache->addType<components::Inventory>();
		inventoryCache->addType<components::LoopSociety>();
		inventoryCache->addType<components::Rectangle>();

	}


	const float margin = 30.0f;

	void update(middle::GameState* gameState) override {

		auto inventoryIt = inventoryCache->begin<components::Inventory>();
		auto loopIt = inventoryCache->begin<components::LoopSociety>();
		auto rectangleIt = inventoryCache->begin<components::Rectangle>();

		for (int i = 0; i < inventoryCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, inventoryCache->relevantIdVector[i].index);
			auto inventory = *inventoryIt;
			auto loop = *loopIt;
			auto inventoryRect = *rectangleIt;

			std::vector<middle::Id>items = loop->loopMemberIds;

			Vector3 inventoryPosition = middle::getShapePosition(gameState, shape.id.index);

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

		}
	}
};

static middle::SystemRegistrar<InventorySystem> reg("InventorySystem");
