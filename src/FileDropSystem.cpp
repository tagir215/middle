#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "rlgl.h"

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

                if (IsFileExtension(droppedFiles.paths[0], ".png") ||
                    IsFileExtension(droppedFiles.paths[0], ".jpg"))
                {
                    gameState->texturesToLoadQueue.push(droppedFiles.paths[0]);
                }
            }

            UnloadDroppedFiles(droppedFiles);    // Unload filepaths from memory

        }

        if (gameState->modelsToLoadQueue.size() > 0) {
            std::string path = gameState->modelsToLoadQueue.front();
            gameState->modelsToLoadQueue.pop();
            Model model = LoadModel(path.c_str());
            gameState->loadedModels.push_back(middle::ModelContainer{path, model});
        }

        if (gameState->texturesToLoadQueue.size() > 0) {
            while (gameState->texturesToLoadQueue.size() > 0) {
				std::string path = gameState->texturesToLoadQueue.front();
				gameState->texturesToLoadQueue.pop();
				Texture2D texture = LoadTexture(path.c_str());
                GenTextureMipmaps(&texture);
                SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);

                SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
                //rlTextureParameters(texture.id, RL_TEXTURE_FILTER_ANISOTROPIC_4X, 16);
                rlTextureParameters(texture.id, RL_TEXTURE_MIPMAP_BIAS_RATIO, -0.5f);
                gameState->loadedTextureMap[path] = middle::TextureContainer{ path, texture };
			}
		}

		//-------------------------
	}
};

static middle::SystemRegistrar<FileDropSystem> reg("FileDropSystem");
