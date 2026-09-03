#include "bubble_layout.h"
#include "middle_shape_utils.h"
#include "BubbleSummationComponent.h"
#include "bubble_utils.h"
#include "BubblePowerComponent.h"
#include "GlobalRect.h"
#include "component_utils.h"
#include "LocalScale.h"

namespace bubble{

	void updateScale(middle::GameState* gameState, middle::Id id, float scalar, float smoothFactor) {
		auto childScale = middle::getComp<components::LocalScale>(gameState, id);
		Vector3 targetScale = Vector3{ scalar,scalar,scalar };
		if(gameState->bubbleAlgebraState.postUndoFrames == 0)
			childScale->scale += (targetScale - childScale->scale) * smoothFactor;
		else
			childScale->scale = targetScale;
	}

	void updatePowerLayoutScale(middle::GameState* gameState, middle::Id id, float smoothFactor)
	{
		middle::Id baseId, exponentId;
		bubble::getPowerBaseAndExponent(gameState, id, baseId, exponentId);
		bubble::updateScale(gameState, baseId, bubble::powerBaseRatio, smoothFactor);
		bubble::updateScale(gameState, exponentId, bubble::powerExponentRatio, smoothFactor);
	}

	void updateSummationLayoutScale(middle::GameState* gameState, middle::Id id, float smoothFactor)
	{
		middle::Id indexId, upperLimitId, summandId;
		bubble::getSummationIndexLimitSummand(gameState, id, indexId, upperLimitId, summandId);
		bubble::updateScale(gameState, indexId, bubble::summationIndexRatio, smoothFactor);
		bubble::updateScale(gameState, upperLimitId, bubble::summationUpperLimitRatio, smoothFactor);
		bubble::updateScale(gameState, summandId, bubble::summationSummandRatio, smoothFactor);
	}

	void updateBubbleLayoutScale(middle::GameState* gameState, middle::Id id, float smoothFactor)
	{
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		int childCount = children.size();

		if (childCount == 0) {
			return;
		}

		float ratio = bubble::scaleRatio;
		if (childCount == 1) {
			ratio = bubble::oneChildScaleRatio;
		}
		else {
			ratio = ratio / childCount;
		}

		for (middle::Id childId : children) {
			bubble::updateScale(gameState, childId, ratio, smoothFactor);
		}
	}


	const Vector3 getDisplacement(float moveSpeed, const Vector3& targetPos, const Vector3& currentPos, float deltaTime) {
		float speedDelta = moveSpeed * deltaTime;
		Vector3 disp = (targetPos - currentPos);
		float dispSq = Vector3LengthSqr(disp);
		if (dispSq > speedDelta * speedDelta) {
			disp = Vector3Normalize(disp) * speedDelta;
		}
		return disp;
	}

	void updateSummationLayout(middle::GameState* gameState, middle::Id id, float width, float moveSpeed) {
		middle::Id indexId, upperLimitId, summandId;
		bubble::getSummationIndexLimitSummand(gameState, id, indexId, upperLimitId, summandId);
		float diameter = width;
		Vector3 leftBottomCorner = Vector3{ -width * 0.5f, 0, -width * 0.5f };
		Vector2 posIndex = bubble::summationLayout[components::SummationRole::INDEX];
		Vector2 posUpperLimit = bubble::summationLayout[components::SummationRole::UPPER_LIMIT];
		Vector2 posSummand = bubble::summationLayout[components::SummationRole::SUMMAND];

		Vector3 targetPosIndex = leftBottomCorner + Vector3{ posIndex.x, 0, posIndex.y } *diameter;
		Vector3 targetPosUpperLimit = leftBottomCorner + Vector3{ posUpperLimit.x, 0, posUpperLimit.y } *diameter;
		Vector3 targetPosSummand = leftBottomCorner + Vector3{ posSummand.x, 0, posSummand.y } *diameter;

		Vector3 currentPosIndex = middle::getLocalPosition(gameState, indexId);
		Vector3 currentPosUpperLimit = middle::getLocalPosition(gameState, upperLimitId);
		Vector3 currentPosSummand = middle::getLocalPosition(gameState, summandId);

		Vector3 dispIndex = getDisplacement(moveSpeed, targetPosIndex, currentPosIndex, gameState->frameTime);
		Vector3 dispUpperLimit = getDisplacement(moveSpeed, targetPosUpperLimit, currentPosUpperLimit, gameState->frameTime);
		Vector3 dispSummand = getDisplacement(moveSpeed, targetPosSummand, currentPosSummand, gameState->frameTime);

		middle::setLocalPosition(gameState, indexId, currentPosIndex + dispIndex);
		middle::setLocalPosition(gameState, upperLimitId, currentPosUpperLimit + dispUpperLimit);
		middle::setLocalPosition(gameState, summandId, currentPosSummand + dispSummand);
	}



	void updatePowerLayout(middle::GameState* gameState, middle::Id id, float width, float moveSpeed) {
		middle::Id baseId, exponentId;
		bubble::getPowerBaseAndExponent(gameState, id, baseId, exponentId);
		float diameter = width;
		Vector3 leftBottomCorner = Vector3{ -diameter * 0.5f, 0, -diameter * 0.5f };
		Vector2 posBase = bubble::powerLayout[components::PowerRole::POWER_ROLE_BASE];
		Vector2 posExponent = bubble::powerLayout[components::PowerRole::POWER_ROLE_EXPONENT];

		Vector3 targetPosBase = leftBottomCorner + Vector3{ posBase.x, 0, posBase.y } *diameter;
		Vector3 targetPosExponent = leftBottomCorner + Vector3{ posExponent.x, 0, posExponent.y } *diameter;

		Vector3 currentBasePos = middle::getLocalPosition(gameState, baseId);
		Vector3 currentExponentPos = middle::getLocalPosition(gameState, exponentId);

		Vector3 dispBase = getDisplacement(moveSpeed, targetPosBase, currentBasePos, gameState->frameTime);
		Vector3 dispExponent = getDisplacement(moveSpeed, targetPosExponent, currentExponentPos, gameState->frameTime);

		middle::setLocalPosition(gameState, baseId, currentBasePos + dispBase);
		middle::setLocalPosition(gameState, exponentId, currentExponentPos + dispExponent);
	}


	void updateBubbleLayout(middle::GameState* gameState, middle::Id id, float width, float moveSpeed) {
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		int childCount = children.size();
		if (childCount == 0) {
			return;
		}

		float diameter = width;
		Vector3 leftBottomCorner = Vector3{ -diameter * 0.5f, 0, -diameter * 0.5f };

		bubble::BubbleLayout layout;
		if (childCount < 10) {
			layout = bubble::layouts[childCount - 1];
		}
		else {
			int rowChildCount = std::sqrt(childCount);
			const float padding = 0.2f;
			float spacing = (1.0f - padding - padding) / rowChildCount;
			float currentRow = 0;
			float currentColumn = 0;
			Vector2 initPos = { padding, padding };
			for (int i = 0; i < childCount; ++i) {
				layout.push_back(initPos + Vector2{ spacing * currentColumn, spacing * currentRow });
				if (currentColumn >= rowChildCount) {
					currentColumn = 0;
					++currentRow;
				}
				else {
					++currentColumn;
				}
			}
		}

		for (int i = 0; i < childCount; ++i) {
			Vector3 layoutPos = { layout[i].x * diameter, 0, layout[i].y * diameter };
			Vector3 targetPosition = leftBottomCorner + layoutPos;
			middle::Id childId = children[i];
			Vector3 currentPos = middle::getLocalPosition(gameState, childId);
			Vector3 disp = getDisplacement(moveSpeed, targetPosition, currentPos, gameState->frameTime);

			if (gameState->bubbleAlgebraState.postUndoFrames == 0)
				middle::setLocalPosition(gameState, childId, currentPos + disp);
			else
				middle::setLocalPosition(gameState, childId, targetPosition);
		}
	}
}
