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

	components::CompCache* uiCache;

	void init(middle::GameState* gameState) {
		uiCache = middle::newCompCache(gameState);
		uiCache->addType<components::UiComponent>();
		uiCache->addType<components::LoopSociety>();
	}
	void update(middle::GameState* gameState) override {

		auto loopIt = uiCache->begin<components::LoopSociety>();

		for (int i = 0; i < uiCache->getSize(); ++i) {
			auto& shape = middle::getShape(gameState, uiCache->relevantIdVector[i].index);
			auto loop = *loopIt;
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

		}
	}
};

static middle::SystemRegistrar<BubbleUiSystem> reg("BubbleUiSystem");
