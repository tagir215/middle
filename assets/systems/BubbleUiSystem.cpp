#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "UiComponent.h"
#include "middle_component_table.h"
#include "middle_shape_utils.h"
#include "Constraint.h"
#include "LoopSociety.h"
#include "Position.h"
#include "bubble_algebra_buttons.h"

class BubbleUiSystem : public middle::MiddleGameplaySystem {
public:
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto ui = middle::getComponent<components::UiComponent>(shape);
			if (!ui) {
				return true;
			}

			auto loop = middle::getComponent<components::LoopSociety>(shape);
			std::vector<Vector3>positions;
			int size = loop->loopMemberIds.size();
			positions.resize(size);
			for (int index = 0; index < size; ++index) {
				auto& id = loop->loopMemberIds[index];
				auto& member = middle::getShape(gameState, id.index);
				auto position = middle::getComponent<components::Position>(member);
				positions[index] = { position->posX, position->posY, position->posZ };
			}

			for (int index = 0; index < size; ++index) {
				int indexA = index - 1;
				int indexB = index;
				if (indexA < 0)
					indexA = size - 1;
				middle::RenderItem renderItem;
				renderItem.type = middle::RenderItemType::LINE;
				renderItem.linePointA = positions[indexA];
				renderItem.linePointB = positions[indexB];
				renderItem.color = middle::UGLY_PINK;
				gameState->renderData.push_back(renderItem);
			}

			return true;
			});
	}
};

static middle::SystemRegistrar<BubbleUiSystem> reg("BubbleUiSystem");
