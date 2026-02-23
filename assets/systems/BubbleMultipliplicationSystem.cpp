#pragma once
#include "game_state.h"
#include "middle_system_registrar.h"
#include "BubbleComponent.h"
#include "BubbleMultiplyComponent.h"
#include "MouseGrabbable.h"
#include "middle_component_table.h"
#include "TimerComponent.h"
#include "editor_actions.h"
#include "InventoryItem.h"
#include "bubble_actions.h"


class BubbleMultipliplicationSystem : public middle::MiddleGameplaySystem {
public:

	bool isIntersecting(middle::GameState* gameState, middle::Shape& shape) {
		auto fraction = middle::getComponent<components::FractionalComponent>(shape);
		auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);

		if (fraction) {
			auto loop = middle::getComponent<components::LoopSociety>(shape);
			for (middle::Id id : loop->loopMemberIds) {
				middle::Shape& shape = middle::getShape(gameState, id.index);
				if (isIntersecting(gameState, shape)) {
					return true;
				}
			}
			return false;
		}

		return intersectable->intersectingTop;
	}

	const float timerTime = 0.2f;
	// MULTIPLICATIONS

	Vector3 closestPointOnOutlineToPoint(middle::GameState* gameState,
		const Vector3& point, const std::vector<middle::Id>& outline) {
		float minDistSq = std::numeric_limits<float>::max();
		Vector3 outlinePos;
		for (const middle::Id& outlineId : outline) {
			Vector3 nodePos = middle::getShapePosition(gameState, outlineId.index);
			float distSq = Vector3DistanceSqr(point, nodePos);
			if (distSq < minDistSq) {
				minDistSq = distSq;
				outlinePos = nodePos;
			}
		}
		return outlinePos;
	}


	void update(middle::GameState* gameState) override {
		return;

		// multiplications
		std::vector<middle::Id> multiplications;
		middle::loopInstances(gameState, [gameState, this, &multiplications](int i, middle::Shape& shape) {
			auto multiplication = middle::getComponent<components::BubbleMultiplyComponent>(shape);
			if (multiplication) {
				multiplications.push_back(shape.id);
			}
			return true;
			});

		// init and render TODO separate
		for (middle::Id& mulId : multiplications) {
			auto& mulShape = middle::getShape(gameState, mulId.index);
			auto multiplication = middle::getComponent<components::BubbleMultiplyComponent>(mulShape);
			auto loop = middle::getComponent<components::LoopSociety>(mulShape);

			// create mul pairs
			std::vector<bubbleActions::MultiplyPair>mulPairs;
			int mulCount = loop->loopMemberIds.size();
			for (int x = 0; x < mulCount; ++x) {
				for (int y = x + 1; y < mulCount; ++y) {
					bubbleActions::MultiplyPair pair;
					pair.parentId = mulId;
					pair.idA = loop->loopMemberIds[x];
					pair.idB = loop->loopMemberIds[y];
					mulPairs.push_back(pair);
				}
			}

			// calc multiplication for each mulpair
			for (auto& mulPair : mulPairs) {
				auto shapeA = middle::getShape(gameState, mulPair.idA.index);
				auto shapeB = middle::getShape(gameState, mulPair.idB.index);
				auto bubbleA = middle::getComponent < components::BubbleComponent>(shapeA);
				auto bubbleB = middle::getComponent < components::BubbleComponent>(shapeB);
				// set render item
				Vector3 posA = middle::getShapePosition(gameState, shapeA.id.index);
				Vector3 posB = middle::getShapePosition(gameState, shapeB.id.index);
				Vector3 connectingLinePosA = closestPointOnOutlineToPoint(gameState, posB, bubbleA->outlineNodes);
				Vector3 connectingLinePosB = closestPointOnOutlineToPoint(gameState, posA, bubbleB->outlineNodes);
				Vector3 connectionCenter = Vector3Scale(connectingLinePosA + connectingLinePosB, 0.5f);
				middle::RenderItem connectingLine;
				connectingLine.type = middle::RenderItemType::LINE;
				connectingLine.linePointA = connectingLinePosA;
				connectingLine.linePointB = connectingLinePosB;
				gameState->renderData.push_back(connectingLine);
				auto mulpos = middle::getComponent<components::Position>(mulShape);
				mulpos->posX = connectionCenter.x;
				mulpos->posY = connectionCenter.y;
				mulpos->posZ = connectionCenter.z;


				// bubble multiplication
				if (gameState->bubbleAlgebraState.mulAction != nullptr)
					continue;

				auto grabbableA = middle::getComponent<components::MouseGrabbable>(shapeA);
				auto grabbableB = middle::getComponent<components::MouseGrabbable>(shapeB);


				if (!grabbableA->grabbing && !grabbableB->grabbing) {
					continue;
				}

				auto& shapeToCopyId = grabbableA->grabbing ? shapeA.id : shapeB.id;
				auto& shapeToCopyIntoId = grabbableA->grabbing ? shapeB.id : shapeA.id;
				auto bubbleToCopy = grabbableA->grabbing ? bubbleA : bubbleB;
				auto bubbleToCopyInto = grabbableA->grabbing ? bubbleB : bubbleA;

				auto& shapeToCopyInto = middle::getShape(gameState, shapeToCopyIntoId.index);
				auto intersectable = middle::getComponent<components::MouseIntersectable>(shapeToCopyInto);

				if (
					gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED
					&& intersectable->intersectingTop
					) {

					// turn on infinite mass 
					bubbleToCopyInto->infiniteMass = true;

					// multiply action
					auto& mulAction = gameState->bubbleAlgebraState.mulAction;
					mulAction = std::make_unique<bubbleActions::ExecuteMultiplication>(mulShape.id, shapeToCopyId, shapeToCopyIntoId);
					mulAction->execute(gameState);

					bubbleActions::setBubbleHidden(gameState, shapeToCopyId, true);

					auto& shapeToCopyInto = middle::getShape(gameState, shapeToCopyIntoId.index);
					auto time = middle::addComponent<components::TimerComponent>(shapeToCopyInto);
					time->timeLeft = timerTime;
				}

			}
		}

		// inventory additions
		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			auto& grabbedShape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
			auto inventoryItem = middle::getComponent<components::InventoryItem>(grabbedShape);
			bool intersecting = false;
			// find top bubble
			middle::Id topBubbleId;
			const int addType = 1;
			const int mulType = 2;
			if (inventoryItem && (inventoryItem->itemType == mulType || inventoryItem->itemType == addType)) {

				middle::Id topBubbleId = bubbleActions::topLevelBubble(gameState);

				bool intersecting = false;
				auto& topBubbleShape = middle::getShape(gameState, topBubbleId.index);
				// if is multiplication check interesct from the children
				auto mul = middle::getComponent<components::BubbleMultiplyComponent>(topBubbleShape);
				if (mul) {
					auto loop = middle::getComponent<components::LoopSociety>(topBubbleShape);
					assert(loop);
					for (middle::Id& childId : loop->loopMemberIds) {
						bool childIntersecting = middle::isMouseIntersectingShape(gameState, childId.index);
						if (childIntersecting) {
							intersecting = true;
						}
					}
				}
				else {
					auto intersectable = middle::getComponent<components::MouseIntersectable>(topBubbleShape);
					intersecting = intersectable->intersectingTop;
				}
				if (intersecting) {
					middle::deleteComponent<components::InventoryItem>(grabbedShape);
					if (inventoryItem->itemType == mulType) {
						auto newMul = bubbleActions::CreateMulitiplicationReplacementShape(topBubbleId, grabbedShape.id);
						newMul.execute(gameState);
						middle::deleteShapeRecursive(gameState, grabbedShape.id.index);
						middle::deleteShapeRecursive(gameState, topBubbleId.index);
					}
					else if (inventoryItem->itemType == addType) {
						auto reparent = middle::EditorActionReparent(topBubbleId.index, grabbedShape.id.index);
						reparent.execute(gameState);
					}
					gameState->bubbleAlgebraState.grabbedId = middle::Id();

				}
			}
		}


		// undo multiplication if moving the bubble out of intersection
		if (gameState->bubbleAlgebraState.mulAction != nullptr) {
			auto mulAction = static_cast<bubbleActions::ExecuteMultiplication*>(gameState->bubbleAlgebraState.mulAction.get());
			auto& containerShape = middle::getShape(gameState, mulAction->shapeToCopyIntoId.index);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(containerShape);
			auto containerBubble = middle::getComponent<components::BubbleComponent>(containerShape);
			auto timer = middle::getComponent<components::TimerComponent>(containerShape);
			if (!intersectable->intersecting && !timer) {
				mulAction->undo(gameState);
				gameState->bubbleAlgebraState.mulAction.release();
			}
		}

		// mouse release after multiplication, this will finallize the multiplication
		if (gameState->bubbleAlgebraState.mulAction != nullptr
			&& gameState->input.mouseReleased) {
			auto mulAction = static_cast<bubbleActions::ExecuteMultiplication*>(gameState->bubbleAlgebraState.mulAction.get());
			mulAction->finalize(gameState);
			gameState->bubbleAlgebraState.mulAction.release();
			gameState->bubbleAlgebraState.grabbedId = middle::Id();
		}



		// ADDITIONS

		// get grabbable if somethign is grabbed
		components::MouseGrabbable* grabbable = nullptr;
		middle::Id grabbedParentId;

		if (gameState->bubbleAlgebraState.grabbedId.index != middle::UNASSIGNED) {
			auto& grabbedShape = middle::getShape(gameState, gameState->bubbleAlgebraState.grabbedId.index);
			auto grabbableLoop = middle::getComponent<components::LoopSociety>(grabbedShape);
			if (grabbableLoop->parentLoopId.index == middle::UNASSIGNED) {
				return;
			}

			auto& parentShape = middle::getShape(gameState, grabbableLoop->parentLoopId.index);
			auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);

			// if not multiplciation component we can probably add them together
			if (!mulComp) {
				grabbable = middle::getComponent<components::MouseGrabbable>(grabbedShape);
			}
			else {
				return;
			}
			// set parent.. if fraction we use parents parent, since that is then the same level as the units we would be intersecting
			auto fraction = middle::getComponent<components::FractionalComponent>(parentShape);
			if (fraction) {
				auto parentLoop = middle::getComponent<components::LoopSociety>(parentShape);
				grabbedParentId = parentLoop->parentLoopId;
			}
			else {
				grabbedParentId = parentShape.id;
			}
		}

		middle::loopInstances(gameState, [gameState, grabbable, this, &grabbedParentId](int i, middle::Shape& shape) {

			auto loop = middle::getComponent<components::LoopSociety>(shape);
			if (!loop) {
				return true;
			}

			auto bubble = middle::getComponent<components::BubbleComponent>(shape);
			auto unit = middle::getComponent<components::BubbleUnit>(shape);
			auto fraction = middle::getComponent<components::FractionalComponent>(shape);
			if (!bubble && !unit && !fraction) {
				return true;
			}

			auto intersectable = middle::getComponent<components::MouseIntersectable>(shape);

			if (bubble && intersectable->intersectingTop && gameState->gameInput.pop) {
				auto popAction = std::make_unique<bubbleActions::Pop>(shape.id);
				popAction->execute(gameState);
			}

			if (loop->parentLoopId.index != middle::UNASSIGNED) {
				auto& parentShape = middle::getShape(gameState, loop->parentLoopId.index);
				// if grabbing units parent is same as grabbed one we can skip
				if (parentShape.id == gameState->bubbleAlgebraState.grabbedId) {
					return true;
				}
				auto mulComp = middle::getComponent<components::BubbleMultiplyComponent>(parentShape);
				if (mulComp) {
					return true;
				}
				return true;
			}


			// if not grabbing anything can continue
			if (!grabbable) {
				return true;
			}
			// if already adding can continue
			if (gameState->bubbleAlgebraState.addAction != nullptr) {
				return true;
			}
			// if not grabbing something from same parents can continue
			if (loop->parentLoopId != grabbedParentId) {
				return true;
			}
			// if the grabbed one is the same one as in this iteration we can continue
			if (shape.id == gameState->bubbleAlgebraState.grabbedId) {
				return true;
			}

			intersectable = middle::getComponent<components::MouseIntersectable>(shape);

			bool isIntersecting = this->isIntersecting(gameState, shape);

			// if intersecting while grabbing do addition
			if (isIntersecting) {
				auto& addAction = gameState->bubbleAlgebraState.addAction;
				addAction = std::make_unique<bubbleActions::ExecuteAddition>(gameState->bubbleAlgebraState.grabbedId, shape.id);
				addAction->execute(gameState);

				// infinite mass
				auto bubbleToAddInto = middle::getComponent<components::BubbleComponent>(shape);
				if (bubbleToAddInto) {
					bubbleToAddInto->infiniteMass = true;
				}

				auto time = middle::addComponent<components::TimerComponent>(shape);
				time->timeLeft = timerTime;
			}

			return true;
			});


		// undo if moved to add shape out
		if (gameState->bubbleAlgebraState.addAction != nullptr) {
			auto addAction = static_cast<bubbleActions::ExecuteAddition*>(gameState->bubbleAlgebraState.addAction.get());
			auto& shapeToAddInto = middle::getShape(gameState, addAction->shapeToAddIntoId.index);
			auto intersectable = middle::getComponent<components::MouseIntersectable>(shapeToAddInto);
			auto bubbleToAddInto = middle::getComponent<components::BubbleComponent>(shapeToAddInto);
			auto timer = middle::getComponent<components::TimerComponent>(shapeToAddInto);
			if (!intersectable->intersecting && !timer) {
				addAction->undo(gameState);
				gameState->bubbleAlgebraState.addAction.release();
			}
		}

		// finialize addition if releasing mouse
		if (gameState->bubbleAlgebraState.addAction != nullptr) {
			if (gameState->input.mouseReleased) {
				auto addAction = static_cast<bubbleActions::ExecuteAddition*>(gameState->bubbleAlgebraState.addAction.get());
				auto& shapeToAddInto = middle::getShape(gameState, addAction->shapeToAddIntoId.index);
				auto bubbleToAddInto = middle::getComponent<components::BubbleComponent>(shapeToAddInto);
				addAction->finalize(gameState);
				gameState->bubbleAlgebraState.addAction.release();
			}
		}

	}
};

static middle::SystemRegistrar<BubbleMultipliplicationSystem> reg("BubbleMultipliplicationSystem");
