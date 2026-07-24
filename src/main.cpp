/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute 'raylib_compile_execute' script
*   Note that compiled executable is placed in the same folder as .c file
*
*   To test the examples on Web, press F6 and execute 'raylib_compile_execute_web' script
*   Web version of the program is generated in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   Example originally created with raylib 1.0, last time updated with raylib 1.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "platform.h"
#include "raylib.h"
#include <chrono>
#include <filesystem>
#include <thread>
#include "game_state.h"
#include <raymath.h>
#include <rlImGui.h>
#include "editor_actions.h"
#include "middle_gameplay_script_map.h"
#include "middle_math.h"
#include "game.h"
#include "sound_helper.h"

#if defined(_DEBUG)
static const char* DLL_PATH = "Debug/game.dll";
static const char* TEMP_PATH = "Debug/game.load.dll";
bool gameMode = false;
#else
static const char* DLL_PATH = "Release/game.dll";
static const char* TEMP_PATH = "Release/game.load.dll";
bool gameMode = true;
#endif

void UpdateGame(GameState* gameState);
void ReloadGameDLL();


typedef decltype(UpdateGame) UpdateGameType;
static UpdateGameType* updateGamePtr;

GameState* gameState;


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
	// Initialization
	//--------------------------------------------------------------------------------------
	// todo 
	const int screenWidth = 1800;
	const int screenHeight = 1200;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

	SetWindowPosition(500, 80);
	//set_window_always_on_top(GetWindowHandle());
	HideCursor();

	gameState = new GameState();
	gameState->worldM = MatrixIdentity();

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	rlImGuiSetup(true);

	const float fixedTimeStep = 1.0f / 60.0f;
	gameState->frameTime = fixedTimeStep;

	if (gameMode) {
		gameState->applicationMode = middle::ApplicationMode::GAME_MODE;
		gameState->releaseBuild = true;
	}

	gameState->editorState.camera = {
		{0,-100,0},
		{0,0,0},
		{0,0,1},
		45,
		CAMERA_PERSPECTIVE
	};

	ShowCursor();


	auto& systemMap = getSystemMap();

	// these are called in middle project
	auto inputSystem = std::move(systemMap["InputSystem"]);
	auto renderSystem = std::move(systemMap["RendererSystem"]);
	auto fileDropSystem = std::move(systemMap["FileDropSystem"]);

	gameState->workingDir = GetWorkingDirectory();

	gameState->startGame = true;

	// load font TODO move somewhere
	int codepoints[] = {
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
		64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
		80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
		96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
		196, 214, 228, 246 
	};
	gameState->globalFont = LoadFontEx("../assets/fonts/paduloki-personal-use-only/paludoki.ttf", gameState->fontUnitFactor, codepoints, 100);
	GenTextureMipmaps(&gameState->globalFont.texture);
	SetTextureFilter(gameState->globalFont.texture, TEXTURE_FILTER_TRILINEAR);

	InitAudioDevice();
	loadSoundEffects(gameState);


	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		if (gameState->quit) {
			break;
		}

		// Update
		//----------------------------------------------------------------------------------
		// TODO: Update your variables here
		//----------------------------------------------------------------------------------
		ReloadGameDLL();

		gameState->screenWidth = GetScreenWidth();
		gameState->screenHeight = GetScreenHeight();


		fileDropSystem->update(gameState);

		inputSystem->update(gameState);

		gameState->frameTimeAccumulator += GetFrameTime();
		UpdateGame(gameState);

		renderSystem->update(gameState);

		playSoundEffects(gameState);

		if (gameState->closeGame) {
			break;
		}

	}

	gameState->closeGame = true;
	UpdateGame(gameState);

	CloseAudioDevice();
	// De-Initialization
	//--------------------------------------------------------------------------------------
	CloseWindow();        // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	delete gameState;

	return 0;
}

void UpdateGame(GameState* gameState)
{
	updateGamePtr(gameState);
}

void ReloadGameDLL()
{
	static void* gameDLL;

	static std::filesystem::file_time_type lastWriteTime;
	auto writeTime = std::filesystem::last_write_time(DLL_PATH);

	if (writeTime != lastWriteTime) {

		if (gameDLL) {
			bool freeResult = platform_free_dynamic_library(gameDLL);
			if (!freeResult)
				return;

			gameDLL = nullptr;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (std::filesystem::exists(TEMP_PATH)) {
			std::filesystem::remove(TEMP_PATH);
		}

		while (!std::filesystem::copy_file(DLL_PATH, TEMP_PATH)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		gameDLL = platform_load_dynamic_library("game.load.dll");

		gameState->reload = true;
		gameState->systemsRegistered = false;

		updateGamePtr = (UpdateGameType*)platform_load_dynamic_function(gameDLL, "UpdateGame");
		lastWriteTime = writeTime;
	}
}

