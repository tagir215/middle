#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MidComp/IntersectingTag.h"
#include "MidComp/SystemReference.h"
#include "MidComp/ComponentReference.h"
#include "MidComp/MouseSelectable.h"
#include "MidComp/ComponentRefParent.h"
#include "MidComp/LoopTag.h"
#include "MidComp/Position.h"
#include "MidComp/Text.h"
#include "editor_actions.h"

class EnviromentalFileNavigationSystem : public middle::MiddleGameplaySystem {
public:
	EnviromentalFileNavigationSystem() {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
		systemModeType = middle::SystemModeType::EDITOR;
	}

	components::CompCache* systemRefCache;

	void init(middle::GameState* gameState) {
		systemRefCache = middle::newCompCache(gameState, systemName);
		systemRefCache->addType<components::SystemReference>();
		systemRefCache->addType<components::IntersectingTag>();
	}

	void update(middle::GameState* gameState) override {

		auto systemRefIt = systemRefCache->begin<components::SystemReference>();
		auto systemIntersectableIt = systemRefCache->begin<components::IntersectingTag>();
		if (gameState->middleState.input.navigateToFileClick) {
			for (int i = 0; i < systemRefCache->getSize(); ++i) {
				auto systemRef = *systemRefIt;
				auto intersectable = *systemIntersectableIt;
				middle::queueAction(gameState, std::make_shared<middle::EditorActionOpenSystem>(systemRef->systemName));
			}
		}

	}
};

static middle::SystemRegistrar<EnviromentalFileNavigationSystem> reg("EnviromentalFileNavigationSystem");
