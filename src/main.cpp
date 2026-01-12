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
	const int screenWidth = 800;
	const int screenHeight = 400;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

	SetWindowPosition(400, 600);
	set_window_always_on_top(GetWindowHandle());
	HideCursor();

	gameState = new GameState();
	gameState->worldM = MatrixIdentity();

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	rlImGuiSetup(true);

	const float fixedTimeStep = 1.0f / 60.0f;
	gameState->frameTime = fixedTimeStep;


	gameState->editorState.camera = {
		{0,-100,0},
		{0,0,0},
		{0,0,1},
		45,
		CAMERA_PERSPECTIVE
	};

	gameState->startGame = true;

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


		gameState->frameTimeAccumulator += GetFrameTime();
		UpdateGame(gameState);

		systemMap["RendererSystem"]->update(gameState);

		if (gameState->closeGame) {
			break;
		}
	}

	gameState->closeGame = true;
	UpdateGame(gameState);

	// De-Initialization
	//--------------------------------------------------------------------------------------
	CloseWindow();        // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

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

	auto writeTime = std::filesystem::last_write_time("Debug/game.dll");
	if (writeTime != lastWriteTime) {

		if (gameDLL) {
			bool freeResult = platform_free_dynamic_library(gameDLL);
			if (!freeResult)
				return;
			gameDLL = nullptr;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (std::filesystem::exists("Debug/game.load.dll")) {
			std::filesystem::remove("Debug/game.load.dll");
		}

		while (!std::filesystem::copy_file("Debug/game.dll", "Debug/game.load.dll")) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		gameDLL = platform_load_dynamic_library("game.load.dll");

		gameState->reload = true;

		updateGamePtr = (UpdateGameType*)platform_load_dynamic_function(gameDLL, "UpdateGame");
		lastWriteTime = writeTime;
	}
}

