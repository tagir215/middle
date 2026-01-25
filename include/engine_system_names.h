#pragma once

namespace middle{
	inline std::vector<std::string>engineSystemNamesFrameStart
	{
		"EditorSystem",
		"MouseIntersectDetectionSystem",
		"MouseGrabbingSystem",
		"MouseSelectionSystem",
		"EnviromentalFileNavigationSystem",
		"EditorHotKeySystem",
		"CameraSystem",
	};

	inline std::vector<std::string>engineSystemNamesFrameEnd
	{
		"MiddlePhysicsSystem",
	};

	inline std::vector<std::string>engineRendererSystemNames{
		"EditorRenderSetupSystem",
		"EditorUiSystem",
		"ComponentUiSystem",
	};
}
