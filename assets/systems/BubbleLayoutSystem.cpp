#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "component_utils.h"
#include "BubbleComponent.h"
#include "Circle.h"
#include "imgui.h"

class BubbleLayoutSystem : public middle::MiddleGameplaySystem {
public:
	BubbleLayoutSystem() {
		systemUpdateType = middle::SystemUpdateType::GAMEPLAY_POSTFRAME;
	}

	components::CompCache* bubbleCache;

	const float moveSpeed = 5;

	void init(middle::GameState* gameState) override {
		bubbleCache = middle::newCompCache(gameState, systemName);
		bubbleCache->addType<components::BubbleComponent>();
		bubbleCache->addType<components::Circle>();
	}

	typedef std::vector<Vector2> BubbleLayout;
    std::vector<BubbleLayout> layouts =
    {
        // 1
        {{0.5f, 0.5f}},

        // 2
        {{0.25f, 0.5f},
        {0.75f, 0.5f}},

        // 3
        {{0.5f, 0.666f},
        {0.333f, 0.333f},
        {0.666f, 0.333f}},

        // 4
        {{0.333f, 0.666f},
        {0.666f, 0.666f},
        {0.333f, 0.333f},
        {0.666f, 0.333f}},

        // 5 - pentagon
        {{0.5f, 0.75f},
        {0.738f, 0.577f},
        {0.647f, 0.298f},
        {0.353f, 0.298f},
        {0.262f, 0.577f}},

        // 6 - dice-style
		{{0.333f, 0.666f},
		 {0.666f, 0.666f},
		 {0.333f, 0.5f},
		 {0.666f, 0.5f},
		 {0.333f, 0.333f},
		 {0.666f, 0.333f}}, // layout 6

		{{0.2f, 0.666f},
		{0.4f, 0.666f},
		{0.6f, 0.666f},
		{0.8f, 0.666f},
		{0.3f, 0.333f},
		{0.5f, 0.333f},
		{0.7f, 0.333f}}, // layout 7

        // 8 - two rows of four
		{{0.2f, 0.666f},
		 {0.4f, 0.666f},
		 {0.6f, 0.666f},
		 {0.8f, 0.666f},
		 {0.2f, 0.333f},
		 {0.4f, 0.333f},
		 {0.6f, 0.333f},
		 {0.8f, 0.333f}}, // layout 8

         // 9 - 3x3 grid
        {{0.333f, 0.666f},
        {0.5f,   0.666f},
        {0.666f, 0.666f},
        {0.333f, 0.5f},
        {0.5f,   0.5f},
        {0.666f, 0.5f},
        {0.333f, 0.333f},
        {0.5f,   0.333f},
        {0.666f, 0.333f}},
    };
	void update(middle::GameState* gameState) override {

		auto circleIt = bubbleCache->begin<components::Circle>();
		for (middle::Id id : bubbleCache->relevantIdVector) {
			auto circle = *circleIt;
			std::vector<middle::Id>children;
			middle::getChildren(gameState, id, children);
			int childCount = children.size();
			if (childCount == 0) {
				continue;
			}

			float diameter = circle->radius * 2;
			Vector3 leftBottomCorner = Vector3{ -circle->radius, 0, -circle->radius };

			BubbleLayout layout;
			if (childCount < 10) {
				layout = layouts[childCount - 1];
			}
			else {
				int rowChildCount = std::sqrt(childCount);
				const float padding = 0.2f;
				float spacing = (1.0f - padding - padding) / rowChildCount;
				float currentRow = 0;
				float currentColumn = 0;
				Vector2 initPos = { padding, padding };
				for (int i = 0; i < childCount; ++i) {
					layout.push_back(initPos + Vector2{ spacing * currentColumn, spacing * currentRow });
					if (currentColumn >= rowChildCount) {
						currentColumn = 0;
						++currentRow;
					}
					else {
						++currentColumn;
					}
				}
			}

			for (int i = 0; i < childCount; ++i) {
				Vector3 layoutPos = { layout[i].x * diameter, 0, layout[i].y * diameter };
				Vector3 targetPosition = leftBottomCorner + layoutPos;
				middle::Id childId = children[i];
				Vector3 currentPos = middle::getLocalPosition(gameState, childId);
				Vector3 disp = (targetPosition - currentPos) * moveSpeed * gameState->frameTime;
				middle::setLocalPosition(gameState, childId, currentPos + disp);
			}
		}

	}
};

static middle::SystemRegistrar<BubbleLayoutSystem> reg("BubbleLayoutSystem");
