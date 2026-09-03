#pragma once
#include <vector>
#include "game_state.h"

namespace bubble{
	typedef std::vector<Vector2> BubbleLayout;
    inline std::vector<BubbleLayout> layouts =
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

	inline BubbleLayout powerLayout =
	{
		{0.45, 0.45},
		{0.90f, 0.90f}
	};

	inline BubbleLayout summationLayout =
	{
		{0.25f, 0.333f},
		{0.25f, 0.666f},
		{0.75f, 0.5f},
	};


	void updateSummationLayout(middle::GameState* gameState, middle::Id id, float width, float moveSpeed);
	void updatePowerLayout(middle::GameState* gameState, middle::Id id, float width, float moveSpeed);
	void updateBubbleLayout(middle::GameState* gameState, middle::Id id, float width, float moveSpeed);
}
