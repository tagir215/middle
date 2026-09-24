#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Inventory.h"
#include "BubbleComponent.h"
#include "MouseGrabbable.h"
#include "editor_actions.h"
#include "LoopSociety.h"
#include "InventoryItem.h"
#include "Position.h"
#include "DeleteComponent.h"
#include "IdRef.h"
#include "component_utils.h"
#include "PlacementComponent.h"
#include "CodeBlock.h"
#include "CodeFunction.h"
#include "UiComponent.h"
#include "Button.h"
#include "ActiveCheckBoxTag.h"
#include "MouseClickComponent.h"
#include "InventorySlot.h"
#include "SnapRef.h"
#include "Layer.h"
#include "InsertableBubble.h"
#include "imgui.h"
#include "IntersectingTag.h"
#include "bubble_utils.h"
#include "LocalPosition.h"
#include "bubble_colors.h"


class BubbleInventorySystem : public middle::MiddleGameplaySystem {
public:
	components::CompCache* cache;

	void init(middle::GameState* gameState) {
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
		cache = middle::newCompCache(gameState, systemName);
		cache->addType<components::Inventory>();
		cache->addType<components::LoopSociety>();
		cache->addType<components::LocalPosition>();
	}


	void update(middle::GameState* gameState) override {
		const float bubbleScaleRatioWithScreenHeight = 0.1f;
		const float distanceFromNearPlane = 900;
		const float screenAxisY = gameState->nearPlaneAxisY / gameState->middleState.nearPlaneDistance * distanceFromNearPlane;
		const float spacing = bubble::bubbleAxis * 0.4f;
		const float scale = 1;
		const midMath::Vector3 itemScale = {scale,scale,scale};

		auto invIt = cache->begin<components::Inventory>();
		auto invPos = cache->begin<components::LocalPosition>();
		auto loopIt = cache->begin<components::LoopSociety>();
		for (middle::Id inventoryId : cache->relevantIdVector) {
			auto localPos = *invPos;
			localPos->pos = { 0,0,0 };

			auto inv = *invIt;
			auto loop = *loopIt;
			const float itemWidth = bubble::bubbleAxis * itemScale.x * 2;
			const float inventoryWidth = itemWidth * inv->maxSize + (inv->maxSize -1) * spacing * itemScale.x;

			midMath::Vector3 cameraPos = gameState->middleState.activeCamera.position;
			midMath::Vector3 center = { cameraPos.x, cameraPos.y + distanceFromNearPlane, cameraPos.z -screenAxisY * 0.8f };

			midMath::Vector3 left = center - midMath::Vector3{inventoryWidth * 0.5f - itemWidth * 0.5f, 0, 0};
			midMath::Vector3 advance = midMath::Vector3{ inventoryWidth / inv->maxSize, 0,0 };
			midMath::Vector3 currentPos = left;
			
			for (int i = 0; i < inv->maxSize; ++i) {
				if (loop->loopMemberIds.size() > i && loop->loopMemberIds[i].index != middle::UNASSIGNED) {
					middle::Id childId = loop->loopMemberIds[i];
					middle::setLocalPosition(gameState, childId, currentPos);
					middle::setLocalScale(gameState, childId, itemScale);
				}

				middle::RenderItem item;
				const float marginScalar = 1.1f;
				item.type = middle::RenderItemType::RECTANGLE;
				item.transform.translation = currentPos;
				item.transform.scale = itemScale;
				item.transform.rotation = { 0,0,0,0 };
				item.layer = 0;
				item.width = bubble::bubbleAxis * 2 * marginScalar;
				item.height = item.width;
				item.length = 0;
				item.color = bubbleColors::DUMMY_GATE;
				if (i == inv->activeIndex) {
					item.color = { 0,0,255,255 };
				}
				gameState->middleState.renderData.push_back(item);

				currentPos += advance;
			}

			auto ui = [gameState, inv]() {
				int size = inv->maxSize;
				ImGui::Begin("inventory");
				if (size > 0) {
					ImGui::SliderInt("active item", &inv->activeIndex, 0, size - 1);
				}
				ImGui::Separator();
				ImGui::Checkbox("invert", &inv->invert);
				ImGui::SameLine();
				ImGui::Checkbox("negate", &inv->negate);

				ImGui::Separator();
				if (ImGui::RadioButton("Add", inv->insertType == components::INSERT_ADD))
					inv->insertType = components::INSERT_ADD;
				ImGui::SameLine();
				if(ImGui::RadioButton("Multiply", inv->insertType == components::INSERT_MULTIPLY))
					inv->insertType = components::INSERT_MULTIPLY;
				ImGui::SameLine();
				if(ImGui::RadioButton("Power", inv->insertType == components::INSERT_POWER))
					inv->insertType = components::INSERT_POWER;

				ImGui::Separator();
				if (ImGui::RadioButton("X/X", inv->invariantType == components::X_OVER_X))
					inv->invariantType = components::X_OVER_X;
				ImGui::SameLine();
				if(ImGui::RadioButton("X-X", inv->invariantType == components::X_MINUS_X))
					inv->invariantType = components::X_MINUS_X;

				if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered()) {
					gameState->inputBlockers.insert(middle::InputBlockers::MOUSE_BLOCK);
				}
				ImGui::End();
				};
			middle::queueUi(gameState, ui);
		}
	}


};

static middle::SystemRegistrar<BubbleInventorySystem> reg("BubbleInventorySystem");
