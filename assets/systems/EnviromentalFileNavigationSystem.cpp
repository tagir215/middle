#pragma once
#include "game_state.h"
#include "registrars.h"
#include "middle_shape_utils.h"
#include "MouseIntersectable.h"
#include "SystemReference.h"
#include "ComponentReference.h"
#include "MouseSelectable.h"
#include "ComponentRefParent.h"
#include "LoopTag.h"
#include "Position.h"
#include "Text.h"
#include "editor_actions.h"

class EnviromentalFileNavigationSystem : public middle::MiddleGameplaySystem {
	void update(middle::GameState* gameState) override {
		middle::loopInstances(gameState, [gameState](int i, middle::Shape& shape) {
			auto system = middle::getComponent<components::SystemReference>(shape);
			auto componentRef = middle::getComponent<components::ComponentReference>(shape);

			if (system || componentRef) {
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);
				if (intersectable->intersecting) {
					if (gameState->input.navigateToFileClick) {
						if (system) {
							gameState->editorState.editorActions.push_back(
								std::make_unique<middle::EditorActionOpenSystem>(system->systemName)
							);
						}
						if (componentRef) {
							gameState->editorState.editorActions.push_back(
								std::make_unique<middle::EditorActionOpenComponent>(componentRef->componentName)
							);
						}
					}
				}
			}



			auto selectable = middle::getComponent<components::MouseSelectable>(shape);

			if (!selectable) {
				return;
			}

			auto componentRefParent = middle::getComponent<components::ComponentRefParent>(shape);

			// if already has componentRef while selected skip TODO
			if (selectable->selected && componentRefParent) {
				return;
			}

			// if unselecteced while having a component skip and delete if theres componentrefparent
			if (!selectable->selected) {
				if (componentRefParent) {
					// delete child shapes
					for (int memberI : componentRefParent->indicatorChildren) {
						middle::deleteShape(gameState, memberI);
					}
					middle::deleteComponent<components::ComponentRefParent>(shape);
				}
				return;
			}

			if(!gameState->input.navigateToFileClick) {
				return;
			}

			// else create component ref
			auto newComponentRefParent = middle::addComponent<components::ComponentRefParent>(shape);

			// get position
			auto position = middle::getComponent<components::Position>(shape);
			auto loop = middle::getComponent<components::LoopTag>(shape);
			Vector3 entpos;
			if (position) {
				entpos = { position->posX, position->posY, position->posZ };
			}
			if (loop) {
				entpos = middle::getLoopCentroid(gameState, i);
			}

			// component Refs setup
			int componentCount = shape.componentMap.size();
			float angleBetween = 2 * PI / componentCount;
			std::vector<Vector3> positions;
			const float r = 40;
			const float compR = 3;
			float initAngle = PI / 10;

			positions.resize(componentCount);
			newComponentRefParent->indicatorChildren.resize(componentCount);

			int index = 0;
			for (auto pair : shape.componentMap) {
				int componentTypeId = pair.first;
				float angle = initAngle + angleBetween * index;
				float x = std::cosf(angle) * r;
				float z = std::sinf(angle) * r;
				Vector3 pos = { x,0,z };
				pos += entpos;
				std::string componentName = middle::componentNameMap[componentTypeId];

				// create entity
				int nextFreeIndex = middle::findNextFreeGhostIndex(gameState);
				middle::addShape(gameState, nextFreeIndex, middle::Shape());
				auto& newShape = middle::getShape(gameState, nextFreeIndex);
				middle::addComponent<components::MouseIntersectable>(newShape);
				auto newRef = middle::addComponent<components::ComponentReference>(newShape);
				auto newPos = middle::addComponent<components::Position>(newShape);
				auto newText = middle::addComponent<components::Text>(newShape);
				newRef->componentName = componentName;
				newPos->posX = pos.x;
				newPos->posY = pos.y;
				newPos->posZ = pos.z;
				newText->fontColorR = WHITE.r;
				newText->fontColorG = WHITE.g;
				newText->fontColorB = WHITE.b;
				newText->fontColorA = WHITE.a;
				newText->offsetX = 2;
				newText->offsetY = 0;
				newText->offsetZ = 10;
				newText->fontSize = middle::REF_TEXT_SIZE;
				newText->text = componentName;

				newComponentRefParent->indicatorChildren[index] = nextFreeIndex;
				++index;
			}

			
			});
	}
};

static middle::SystemRegistrar<EnviromentalFileNavigationSystem> reg("EnviromentalFileNavigationSystem");
