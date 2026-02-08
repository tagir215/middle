#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "middle_shape_utils.h"
#include "Rectangle.h"

class UiRenderSetup : public middle::MiddleGameplaySystem {
public:
	UiRenderSetup() {
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void update(middle::GameState* gameState) override {

		middle::loopInstances(gameState, [gameState, this](int i, middle::Shape& shape) {
			auto rectangle = middle::getComponent<components::Rectangle>(shape);
			if (!rectangle)
				return;
			Vector3 position = middle::getShapePosition(gameState, shape.id.index);

			middle::RenderItem line1;
			line1.type = middle::RenderItemType::LINE;
			line1.linePointA = { position.x - rectangle->width * 0.5f, 0, position.z + rectangle->height * 0.5f };
			line1.linePointB = { position.x - rectangle->width * 0.5f, 0, position.z - rectangle->height * 0.5f };
			gameState->renderData.push_back(line1);
			middle::RenderItem line2;
			line2.type = middle::RenderItemType::LINE;
			line2.linePointA = { position.x + rectangle->width * 0.5f, 0, position.z + rectangle->height * 0.5f };
			line2.linePointB = { position.x + rectangle->width * 0.5f, 0, position.z - rectangle->height * 0.5f };
			gameState->renderData.push_back(line2);
			middle::RenderItem line3;
			line3.type = middle::RenderItemType::LINE;
			line3.linePointA = { position.x - rectangle->width * 0.5f, 0, position.z - rectangle->height * 0.5f };
			line3.linePointB = { position.x + rectangle->width * 0.5f, 0, position.z - rectangle->height * 0.5f };
			gameState->renderData.push_back(line3);
			middle::RenderItem line4;
			line4.type = middle::RenderItemType::LINE;
			line4.linePointA = { position.x - rectangle->width * 0.5f, 0, position.z + rectangle->height * 0.5f };
			line4.linePointB = { position.x + rectangle->width * 0.5f, 0, position.z + rectangle->height * 0.5f };
			gameState->renderData.push_back(line4);

			});
	}
};

static middle::SystemRegistrar<UiRenderSetup> reg("UiRenderSetup");
