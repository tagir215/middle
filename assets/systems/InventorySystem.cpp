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
#include "InventoryItem.h"
#include "InventorySlot.h"
#include "SnapRef.h"

class InventorySystem : public middle::MiddleGameplaySystem {

public:
	//components::CompCache* inventoryItemRefCache;
	components::CompCache* inventoryRectCache;

	InventorySystem() {
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
	}

	void init(middle::GameState* gameState) {
		inventoryRectCache = middle::newCompCache(gameState, systemName);
		inventoryRectCache->addType<components::Inventory>();
		inventoryRectCache->addType<components::LoopSociety>();
		inventoryRectCache->addType<components::Rectangle>();
		//inventoryItemRefCache = middle::newCompCache(gameState);
		//inventoryItemRefCache->addType<components::InventoryItem>();
		//inventoryItemRefCache->addType<components::SnapRef>();
	}


	const float margin = 30.0f;

	void update(middle::GameState* gameState) override {

		//auto snapRefIt = inventoryItemRefCache->begin<components::SnapRef>();
		//for (int i = 0; i < inventoryItemRefCache->getSize(); ++i) {
		//	auto snapRef = *snapRefIt;
		//	middle::Id id = inventoryItemRefCache->relevantIdVector[i];
		//	assert(middle::isValidId(gameState, snapRef->snapTargetId));
		//	Vector3 pos = middle::getShapePosition(gameState, snapRef->snapTargetId.index);
		//	Vector3 currPos = middle::getShapePosition(gameState, id.index);
		//	middle::moveShape(gameState, id.index, pos - currPos);
		//}

		auto inventoryRectIt = inventoryRectCache->begin<components::Inventory>();
		auto rectangleIt = inventoryRectCache->begin<components::Rectangle>();

		for (int i = 0; i < inventoryRectCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, inventoryRectCache->relevantIdVector[i].index);
			auto inventory = *inventoryRectIt;
			auto inventoryRect = *rectangleIt;


			if (inventory->freeLayout) {
				continue;
			}

			std::vector<middle::Id>items;
			middle::getChildren(gameState, shape.id, items);

			Vector3 inventoryPosition = middle::getGlobalPosition(gameState, shape.id.index);

			float totalWidth = inventoryRect->width;
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

			float rowSpacing = 0;
			if (inventory->rows > 1) {
				spacing *= inventory->rows;
				rowSpacing = totalHeight / inventory->rows;
			}

			Vector3 initialPos = referencePos;
			for (int j = 0; j < items.size(); ++j) {
				middle::Id childId = items[j];
				middle::Shape& child = middle::getShape(gameState, childId.index);

				Vector3 displacement = referencePos - middle::getGlobalPosition(gameState, child.id.index);
				middle::moveShape(gameState, child.id.index, displacement);
				if (inventory->horizontal) {
					referencePos.x += spacing;
				}
				else {
					referencePos.z -= spacing;
				}

				if (inventory->horizontal) {
					if (referencePos.x >= initialPos.x + totalWidth - 0.01f) {
						referencePos.x = initialPos.x;
						referencePos.z -= rowSpacing;
					}
				}
			}

		}
	}
};

static middle::SystemRegistrar<InventorySystem> reg("InventorySystem");
