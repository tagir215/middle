#pragma once
#include <string>
#include <vector>

namespace middle {
	static const int UNASSIGNED = -1;
	static const int MAX_SHAPE_COUNT = 612;
	static const int GHOST_INDEX_OFFSET = 306;
	static const int MAX_CONSTRAINT_COUNT = 612;
	static const int MAX_VERTEX_COUNT = 10000;

	inline std::vector<std::string>engineSystemNamesFrameStart
	{
		"InputSystem",
		"EditorSystem",
		"MouseIntersectDetectionSystem",
		"MouseGrabbingSystem",
		"MouseSelectionSystem",
		"EnviromentalFileNavigationSystem",
	};

	inline std::vector<std::string>engineSystemNamesFrameEnd
	{
		"MiddlePhysicsSystem",
	};

	inline std::vector<std::string>engineRendererSystemNames{
		"EditorRenderSetupSystem",
	};
}
