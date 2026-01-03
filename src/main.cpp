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

#include "raylib.h"
#include "game.h"
#include "platform.h"
#include <chrono>
#include <filesystem>
#include <thread>
#include "game_state.h"
#include <raymath.h>
#include <rlImGui.h>
#include "renderer.h"
#include "editor_actions.h"

void UpdateGame(GameState* gameState);
void ReloadGameDLL();
void UpdateInput(GameState* gameState, Matrix& screenOrientorM);


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
	const int screenWidth = 1600;
	const int screenHeight = 800;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

	SetWindowPosition(100, 150);
	//set_window_always_on_top(GetWindowHandle());
	//HideCursor();

	gameState = new GameState();
	gameState->worldM = MatrixIdentity();

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	rlImGuiSetup(true);

	const float fixedTimeStep = 1.0f / 60.0f;
	float timeAccumulator = 0;
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

		Matrix scalorM = MatrixScale(1, -1, 1);
		Matrix translatorM = MatrixTranslate(0, GetScreenHeight(), 0);
		gameState->screenOrientorM = MatrixMultiply(scalorM, translatorM);

		UpdateInput(gameState, gameState->screenOrientorM);


		timeAccumulator += GetFrameTime();
		if (timeAccumulator >= fixedTimeStep) 
		{
			UpdateGame(gameState);
			timeAccumulator -= fixedTimeStep;
		}


		middle::Draw(gameState);

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

void UpdateInput(GameState* gameState, Matrix& screenOrientorM) {
	// INPUTS
	Vector3 mousePos = { GetMouseX(), GetMouseY(), 0 };
	Vector3 invertedMouse = Vector3Transform(mousePos, screenOrientorM);
	gameState->input.mousePos.x = invertedMouse.x;
	gameState->input.mousePos.y = invertedMouse.y;
	gameState->input.zoomIn = GetMouseWheelMoveV().y > 0;
	gameState->input.zoomOut = GetMouseWheelMoveV().y < 0;
	if (gameState->inputBlockers.find(InputBlockers::KEYBOARD_BLOCK) == gameState->inputBlockers.end()) {
		gameState->input.mouseHeld = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
		gameState->input.mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
		gameState->input.mouseReleased = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
		gameState->input.w = IsKeyDown(KEY_W);
		gameState->input.s = IsKeyDown(KEY_S);
		gameState->input.a = IsKeyDown(KEY_A);
		gameState->input.d = IsKeyDown(KEY_D);
		gameState->input.q = IsKeyDown(KEY_Q);
		gameState->input.e = IsKeyDown(KEY_E);
		gameState->input.grabDown = IsKeyDown(KEY_G);
		gameState->input.grabReleased = IsKeyReleased(KEY_G);
		gameState->input.grabReleased = IsKeyReleased(KEY_G);
		gameState->input.infoClick = IsKeyPressed(KEY_I);
		gameState->input.loopClick = IsKeyPressed(KEY_L);
		gameState->input.selectModeClick = IsKeyPressed(KEY_ONE);
		gameState->input.sphereModeClick = IsKeyPressed(KEY_TWO);
		gameState->input.constraintModeClick = IsKeyPressed(KEY_THREE);
		gameState->input.cameraModeClick = IsKeyPressed(KEY_FOUR);
		gameState->input.scriptModeClick = IsKeyPressed(KEY_FIVE);
		gameState->input.deleteClick = IsKeyPressed(KEY_R);
		gameState->input.copyClick = IsKeyPressed(KEY_C);
		gameState->input.saveClick = IsKeyPressed(KEY_P);
		gameState->input.openScript = IsKeyPressed(KEY_SPACE);
		gameState->input.focus = IsKeyPressed(KEY_F);
		gameState->input.newThing = IsKeyPressed(KEY_N);
	}

	// auto clear so others can fight over keeping stuff blocked
	gameState->inputBlockers.clear();

	// CAMERA POSITION UPDATE
	int cameraPosX = gameState->screenWidth / 2;
	int cameraPosY = gameState->screenHeight / 2;

	// MOUSE POSITION UPDATE
	int relativeX = gameState->input.mousePos.x - cameraPosX;
	int relativeY = gameState->input.mousePos.y - cameraPosY;
	gameState->input.mouseNormalizedPos.x = (float)relativeX / (float)cameraPosX;
	gameState->input.mouseNormalizedPos.y = (float)relativeY / (float)cameraPosX;
	gameState->aspectRatio = gameState->screenWidth / gameState->screenHeight;
	float angle = gameState->editorState.camera.fovy * DEG2RAD * 0.5f;
	float nearAxisY = tan(angle) * gameState->nearPlaneDistance;
	float nearAxisX = nearAxisY * gameState->aspectRatio;
	float nearPlanePos2dX = nearAxisX * gameState->input.mouseNormalizedPos.x;
	float nearPlanePos2dY = nearAxisX * gameState->input.mouseNormalizedPos.y;

	Vector3 cameraDir = Vector3Normalize(gameState->editorState.camera.target - gameState->editorState.camera.position);
	Vector3 cameraRight = Vector3Normalize(Vector3CrossProduct(cameraDir, gameState->editorState.camera.up));
	Vector3 cameraUp = Vector3CrossProduct(cameraRight, cameraDir);
	gameState->input.mouseNearPlanePos = gameState->editorState.camera.position
		+ cameraDir * gameState->nearPlaneDistance
		+ cameraRight * nearPlanePos2dX
		+ cameraUp * nearPlanePos2dY;

	gameState->input.mouseDir = Vector3Normalize(gameState->input.mouseNearPlanePos - gameState->editorState.camera.position);

	Vector3 xzPlanePos = { 0,0,0 };
	Vector3 xzPlaneNormal = { 0,-1,0 };
	Vector3 previousXZ_PlanePos = gameState->input.mouseXZ_PlanePos;
	Vector3 nextXZ_PlanePos = RayCastLinePlane(xzPlanePos, xzPlaneNormal, gameState->input.mouseNearPlanePos, gameState->input.mouseDir);
	gameState->input.mouseXZ_PlanePos = nextXZ_PlanePos;
	gameState->input.mouseXZ_PlaneVelocity = (nextXZ_PlanePos - previousXZ_PlanePos) / gameState->frameTime;
}
