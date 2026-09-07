#include "middle_debug_utils.h"
#include "imgui.h"

namespace middle {

	void DrawText() {

	}

	void drawImGuiInt(middle::GameState* gameState, const char* label, int i)
	{
		auto ui = [gameState, label, i]() {
			ImGui::Begin(label);
			ImGui::Text(std::to_string(i).c_str());
			ImGui::End();
			};
		gameState->uiSetups.push_back(ui);
	}

	void drawImGuiFloat(middle::GameState* gameState, const char* label, float f)
	{
		auto ui = [gameState, label, f]() {
			ImGui::Begin(label);
			ImGui::Text(std::to_string(f).c_str());
			ImGui::End();
			};
		gameState->uiSetups.push_back(ui);
	}

	void drawImGuiIntVector(middle::GameState* gameState, const char* label, const std::vector<int>& vector)
	{
		auto ui = [gameState, label, vector]() {
			ImGui::Begin(label);
			for (int v : vector) {
				ImGui::Text(std::to_string(v).c_str());
			}
			ImGui::End();
			};
		gameState->uiSetups.push_back(ui);
	}
}
