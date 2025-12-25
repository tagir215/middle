#include "state_update.h"
#include "middle_math.h"
namespace middle {

	void reset(GameState* gameState) {
		if (gameState->reset) {
			gameState->reset = false;
			for (int i = 0; i < gameState->shapes.size(); ++i) {
				++gameState->shapes[i].id.generation;
			}
		}

	}

	void updateInstances(GameState* gameState) {
		gameState->reload = false;

		for (int i = 0; i < gameState->shapes.size(); ++i) {
			Shape& shape = gameState->shapes[i];
			bool shouldSkip = gameState->isSlotFree(i);
			bool shouldMake = !gameState->isSlotFree(i) && !gameState->isShapeAlive(i);
			bool shouldUpdate = !gameState->isSlotFree(i) && gameState->isShapeAlive(i);
			bool shouldDelete = gameState->isSlotFree(i) && gameState->isShapeAlive(i);


			if (shouldSkip) {
				continue;
			}
			if (shouldMake) {
				gameState->addInstance(i, MakeShapeInstance(shape));
			}
			if (shouldUpdate) {
				auto& instance = gameState->getShapeInstance(i);
				// update shape for hot reload
				instance.shape = shape;

				// step backward
				if (gameState->doOneStep && instance.history.size() > 0 && gameState->stepDir == -1) {
					instance.pData = instance.history.front();
					instance.history.pop_front();
					instance.lifeTime -= gameState->frameTime;
				}
				// step forward
				else if ((!gameState->paused || gameState->doOneStep) && gameState->stepDir == 1) {
					auto copy = instance.pData;
					instance.history.push_front(copy);
					if (instance.history.size() > shape.historyMemoryLength) {
						instance.history.pop_back();
					}
					instance.lifeTime += gameState->frameTime;
				}

				// update loop centroids
				if (instance.shape.type == ShapeType::LOOP) {
					Vec centroid = { 0,0,0 };
					for (int loopIndex = shape.loopArrayOffset; loopIndex < shape.loopArrayOffset + shape.loopSize; ++loopIndex) {
						// add member positions 
						centroid = descart::AddV(centroid, gameState->getShapeInstance(gameState->loopMembers[loopIndex]).pData.position);
					}
					instance.pData.position = descart::ScaleV(centroid, 1.0f / (float)shape.loopSize);
				}

				// reset state
				if (instance.lifeTime > shape.maxLifetime) {
					auto& pdat = instance.pData;
					instance.lifeTime = 0;
					TranslateInitShape(instance.shape, shape.position.x, shape.position.y, shape.position.z);
					instance.pData.transform = DescMat(instance.shape.initTransform);
					pdat.position = VecTransform({ 0,0,0 }, instance.pData.transform);
					pdat.linearAcc = DescVec(shape.linearAcceleration);
					pdat.linearVel = DescVec(shape.linearVelocity);
				}
			}
		}

		reset(gameState);
	}
}
