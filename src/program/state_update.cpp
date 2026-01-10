#include "state_update.h"
#include "middle_math.h"
#include "middle_shape_utils.h"
namespace middle {


	void reset(GameState* gameState) {
		if (gameState->reset) {
			gameState->reset = false;
			for (int i = 0; i < gameState->shapes.size(); ++i) {
				int nextGeneration = gameState->shapes[i].id.generation + 1;
				gameState->shapes[i] = Shape();
				gameState->shapes[i].id.generation = nextGeneration;
			}
		}
	}

	void updateInstances(GameState* gameState) {

		//for (int i = 0; i < gameState->shapes.size(); ++i) {
		//	Shape& shape = gameState->shapes[i];
		//	bool shouldSkip = isSlotFree(gameState, i);
		//	bool shouldMake = !isSlotFree(gameState, i) && !isShapeAlive(gameState, i);
		//	bool shouldUpdate = !isSlotFree(gameState, i) && isShapeAlive(gameState, i);
		//	bool shouldDelete = isSlotFree(gameState, i) && isShapeAlive(gameState, i);


		//	if (shouldSkip) {
		//		continue;
		//	}
		//	if (shouldMake) {
		//		addInstance(gameState, i, MakeShapeInstance(shape));
		//	}
		//	if (shouldUpdate) {
		//		auto& instance = getShapeInstance(gameState, i);
		//		// update shape for hot reload
		//		instance.shape = shape;

		//		// step backward
		//		if (gameState->editorState.doOneStep && instance.history.size() > 0 && gameState->editorState.stepDir == -1) {
		//			instance.pData = instance.history.front();
		//			instance.history.pop_front();
		//			instance.lifeTime -= gameState->frameTime;
		//		}
		//		// step forward
		//		else if ((!gameState->paused || gameState->editorState.doOneStep) && gameState->editorState.stepDir == 1) {
		//			auto copy = instance.pData;
		//			instance.history.push_front(copy);
		//			if (instance.history.size() > shape.historyMemoryLength) {
		//				instance.history.pop_back();
		//			}
		//			instance.lifeTime += gameState->frameTime;
		//		}

		//		// update loop centroids
		//		if (shape.type == ShapeType::LOOP) {
		//			Vec centroid = { 0,0,0 };
		//			for (int loopIndex = shape.loopArrayOffset; loopIndex < shape.loopArrayOffset + shape.loopSize; ++loopIndex) {
		//				// add member positions 
		//				centroid = descart::AddV(centroid, getShapeInstance(gameState, gameState->loopMembers[loopIndex]).pData.position);
		//			}
		//			instance.pData.position = descart::ScaleV(centroid, 1.0f / (float)shape.loopSize);
		//			shape.position = FromDescVec(instance.pData.position);
		//		}

		//		// reset state
		//		if (instance.lifeTime > shape.maxLifetime) {
		//			auto& pdat = instance.pData;
		//			instance.lifeTime = 0;
		//			TranslateInitShape(instance.shape, shape.position.x, shape.position.y, shape.position.z);
		//			instance.pData.transform = DescMat(instance.shape.initTransform);
		//			pdat.position = VecTransform({ 0,0,0 }, instance.pData.transform);
		//			pdat.linearAcc = DescVec(shape.linearAcceleration);
		//			pdat.linearVel = DescVec(shape.linearVelocity);
		//		}
		//	}
		//}

		//reset(gameState);
	}
}
