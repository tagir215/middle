#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "imgui.h"

class SystemProfilerUiSystem : public middle::MiddleGameplaySystem {
public:
	SystemProfilerUiSystem() {
		systemUpdateType = middle::SystemUpdateType::RENDERING;
		systemModeType = middle::SystemModeType::ENGINE;
	}

	void init(middle::GameState* gameState) override {
	}

	void drawText(const MiddleGameplaySystem* sys) {
		if (!sys) {
			return;
		}
		auto count = sys->updateTime.count();
		if (count <= 0) {
			return;
		}
		std::string text = sys->systemName + ": " + std::to_string(count) + "ms";
		ImGui::Text(text.c_str());
	}

	void update(middle::GameState* gameState) override {
		auto ui = [gameState, this] {
			ImGui::Begin("profiler");
			for (auto& sys : gameState->engineRendererSystems) {
				drawText(sys.get());
			}

			for (auto& sys : gameState->engineSystemsFrameStart) {
				drawText(sys.get());
			}

			for (auto& pair : gameState->gameplaySystems) {
				drawText(pair.second.get());
			}

			for (auto& pair : gameState->gameplaySystemsPostFrame) {
				drawText(pair.second.get());
			}

			for (auto sys : gameState->externalPreFrameSystems) {
				drawText(sys.get());
			}

			for (auto sys : gameState->externalPostFrameSystems) {
				drawText(sys.get());
			}

			ImGui::End();
			};
		gameState->uiSetups.push_back(ui);
	}
};

static middle::SystemRegistrar<SystemProfilerUiSystem> reg("SystemProfilerUiSystem");
