#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "MouseIntersectable.h"
#include "SystemReference.h"
#include "ComponentReference.h"
#include "MouseSelectable.h"
#include "ComponentRefParent.h"
#include "LoopTag.h"
#include "Position.h"
#include "Text.h"
#include "editor_actions.h"

class EnviromentalFileNavigationSystem : public middle::MiddleGameplaySystem {
public:
	EnviromentalFileNavigationSystem() {
		systemUpdateType = middle::SystemUpdateType::PREFRAME;
		systemModeType = middle::SystemModeType::EDITOR;
	}

	components::CompCache* systemRefCache;

	void init(middle::GameState* gameState) {
		systemRefCache = middle::newCompCache(gameState);
		systemRefCache->addType<components::SystemReference>();
		systemRefCache->addType<components::MouseIntersectable>();
	}

	void update(middle::GameState* gameState) override {

		auto systemRefIt = systemRefCache->begin<components::SystemReference>();
		auto systemIntersectableIt = systemRefCache->begin<components::MouseIntersectable>();
		if (gameState->input.navigateToFileClick) {
			for (int i = 0; i < systemRefCache->getSize(); ++i) {
				auto systemRef = *systemRefIt;
				auto intersectable = *systemIntersectableIt;
				if (intersectable->intersecting) {
					middle::queueAction(gameState, std::make_shared<middle::EditorActionOpenSystem>(systemRef->systemName));
				}
			}
		}

	}
};

static middle::SystemRegistrar<EnviromentalFileNavigationSystem> reg("EnviromentalFileNavigationSystem");
