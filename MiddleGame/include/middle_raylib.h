#pragma once
#include <raylib.h>
#include <chrono>
#include <thread>
#include <rlImGui.h>
#include "game.h"
#include "sound_helper.h"
#include "init_external_systems.h"
#include <iostream>
#include "assets_loading.h"
#include "profiler_helpers.h"
#include "InputSystem.cpp"
#include "Renderer.cpp"
#include "midconfig.h"

#if defined(_DEBUG)
static const char* DLL_PATH = "Debug/game.dll";
static const char* TEMP_DLL_NAME = "Debug/game.load";
bool gameMode = false;
#else
static const char* DLL_PATH = "Release/game.dll";
static const char* TEMP_DLL_NAME = "Release/game.load";
bool gameMode = true;
#endif

struct RayState{
	Font globalFont;
	std::vector<Sound>sounds;
	std::vector<Texture>textures;
	std::vector<Shader>shaders;
};

void UpdateGame(const middle::MiddleInputState& inputState, middle::MiddleOutputState** outputState);
void ReloadGameDLL();

typedef decltype(UpdateGame) UpdateGameType;
static UpdateGameType* updateGamePtr;

class MiddleRaylibEngine {

const int fps = 60;

public:
	RayState rayState;
	float frameTimeAccumulator = 0;
	middle::MiddleOutputState* outputState = nullptr;

	void init() {
		SetTargetFPS(fps);               
		rlImGuiSetup(true);

		bubbleAssets::loadAssets(rayState.shaders, rayState.textures);
		bubbleAssets::loadGlobalFont(rayState.globalFont);
	}

	middle::MiddleInputState updateInputState() {
		middle::MiddleInputState inputState;
		const float fixedTimeStep = 1.0f / (float)fps;
		inputState.frameTime = fixedTimeStep;
		inputState.frameTimeAccumulator = frameTimeAccumulator + GetFrameTime();
		inputState.screenWidth = GetScreenWidth();
		inputState.screenHeight = GetScreenHeight();
		return inputState;
	}

	void start() {
		// Main game loop
		while (!WindowShouldClose())    // Detect window close button or ESC key
		{
			// Update
			//----------------------------------------------------------------------------------
			// TODO: Update your variables here
			//----------------------------------------------------------------------------------
			ReloadGameDLL();

			auto inputState = updateInputState();
			InputSystem::update(&inputState, outputState);

			UpdateGame(inputState, &outputState);

			renderer::RendererSystem::update(outputState, rayState.globalFont, rayState.shaders, rayState.textures, false);

			if (outputState->closeGame) {
				break;
			}
		}

		auto inputState = updateInputState();
		inputState.closeGame = true;
		UpdateGame(inputState, &outputState);
	}

};




void UpdateGame(const middle::MiddleInputState& inputState, middle::MiddleOutputState** outputState)
{
	updateGamePtr(inputState, outputState);
}

void ReloadGameDLL()
{
	static void* gameDLL;

	static std::filesystem::file_time_type lastWriteTime;
	auto writeTime = std::filesystem::last_write_time(DLL_PATH);

	static int loadIndex = 0;

	if (writeTime != lastWriteTime) {

		if (gameDLL) {
			bool freeResult = platform_free_dynamic_library(gameDLL);
			if (!freeResult)
				return;

			gameDLL = nullptr;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		std::string loadPath = std::string(TEMP_DLL_NAME) + std::to_string(loadIndex) + ".dll";
		++loadIndex;

		if (std::filesystem::exists(loadPath)) {
			std::filesystem::remove(loadPath);
		}

		std::error_code ec;
		bool copied = std::filesystem::copy_file(
			DLL_PATH,
			loadPath,
			std::filesystem::copy_options::overwrite_existing,
			ec
		);

		if (!copied) {
			std::cout << "Failed to copy DLL\n";
			std::cout << "source: " << DLL_PATH << '\n';
			std::cout << "dest:   " << loadPath << '\n';
			std::cout << "error:  " << ec.message() << '\n';
			std::cout << "code:   " << ec.value() << '\n';
			return;
		}
		gameDLL = platform_load_dynamic_library(loadPath.data());

		updateGamePtr = (UpdateGameType*)platform_load_dynamic_function(gameDLL, "UpdateGame");
		lastWriteTime = writeTime;
	}
}

