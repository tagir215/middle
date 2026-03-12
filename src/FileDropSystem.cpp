#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"

class FileDropSystem : public middle::MiddleGameplaySystem {


	void init(middle::GameState* gameState) override {
	}

	void update(middle::GameState* gameState) override {

        if (IsFileDropped())
        {
            FilePathList droppedFiles = LoadDroppedFiles();

            if (droppedFiles.count == 1) // Only support one file dropped
            {
                if (IsFileExtension(droppedFiles.paths[0], ".obj") ||
                    IsFileExtension(droppedFiles.paths[0], ".gltf") ||
                    IsFileExtension(droppedFiles.paths[0], ".glb") ||
                    IsFileExtension(droppedFiles.paths[0], ".vox") ||
                    IsFileExtension(droppedFiles.paths[0], ".iqm") ||
                    IsFileExtension(droppedFiles.paths[0], ".m3d"))       // Model file formats supported
                {
                    gameState->modelsToLoadQueue.push(droppedFiles.paths[0]);
                }
            }

            UnloadDroppedFiles(droppedFiles);    // Unload filepaths from memory

        }

        if (gameState->modelsToLoadQueue.size() > 0) {
            std::string path = gameState->modelsToLoadQueue.front();
            gameState->modelsToLoadQueue.pop();
            Model model = LoadModel(path.c_str());
            gameState->loadedModels.push_back({path, model});
        }

        //-------------------------
	}
};

static middle::SystemRegistrar<FileDropSystem> reg("FileDropSystem");
