#pragma once
#include <raylib.h>
#include <chrono>
#include <thread>
#include <rlImGui.h>
#include "game_state.h"
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

std::unique_ptr<middle::GameState> gameState;
struct RayState{
	Font globalFont;
	std::unordered_map<std::string, Sound>soundMap;
};

void UpdateGame(middle::GameState* gameState);
void ReloadGameDLL();

typedef decltype(UpdateGame) UpdateGameType;
static UpdateGameType* updateGamePtr;

class MiddleRaylibEngine {

public:
	RayState rayState;

	void init() {
		const int fps = 60;
		SetTargetFPS(fps);               
		rlImGuiSetup(true);

		gameState = std::make_unique<middle::GameState>();
		bubbleAssets::loadAssets(gameState.get());
		const float fixedTimeStep = 1.0f / (float)fps;
		gameState->middleState.frameTime = fixedTimeStep;
		if (gameMode) {
			gameState->middleState.applicationMode = middle::ApplicationMode::GAME_MODE;
			gameState->middleState.releaseBuild = true;
		}
		gameState->middleState.startGame = true;


		// load font TODO move somewhere
		int codepoints[] = {
			// Basic ASCII (32 to 126) for standard numbers and text
			32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
			50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
			68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
			86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
			103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
			117, 118, 119, 120, 121, 122, 123, 124, 125, 126,

			// Custom Math Codepoints
			0x00D7,   // Multiplication sign (×)
			0x22C5,   // Dot operator (⋅)
			0x2211,    // Summation operator (∑)
		};
		int codepointCount = sizeof(codepoints) / sizeof(codepoints[0]);
		const int fontUnitFactor = 1024;
		std::string fontPath = std::string(middlePaths::FONTS_FOLDER) + "/math-sans/NotoSansMath-Regular.ttf";
		rayState.globalFont = LoadFontEx(fontPath.c_str(), fontUnitFactor, codepoints, codepointCount);
		GenTextureMipmaps(&rayState.globalFont.texture);
		SetTextureFilter(rayState.globalFont.texture, TEXTURE_FILTER_TRILINEAR);

		std::unordered_map<std::string, Sound>soundMap;
		middleSoundHelpers::loadSoundEffects(soundMap, gameState.get());
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

			gameState->middleState.screenWidth = GetScreenWidth();
			gameState->middleState.screenHeight = GetScreenHeight();

			InputSystem::update(&gameState->middleState);

			gameState->middleState.frameTimeAccumulator += GetFrameTime();
			UpdateGame(gameState.get());

			renderer::RendererSystem::update(&gameState->middleState, rayState.globalFont, false);

			gameState->debugInfo.clear();

			if (gameState->middleState.closeGame) {
				break;
			}
		}

		gameState->middleState.closeGame = true;
		UpdateGame(gameState.get());

	}

};




void UpdateGame(middle::GameState* gameState)
{
	updateGamePtr(gameState);
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

		gameState->middleState.reload = true;
		gameState->systemsRegistered = false;

		updateGamePtr = (UpdateGameType*)platform_load_dynamic_function(gameDLL, "UpdateGame");
		lastWriteTime = writeTime;
	}
}

