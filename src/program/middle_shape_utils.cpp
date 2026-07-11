#include "middle_shape_utils.h"
#include "middle_math.h"
#include "middle_component_table.h"
#include "LoopSociety.h"
#include "Sphere.h"
#include "Reference.h"
#include "Position.h"
#include "Constraint.h"
#include "PhysicsData.h"
#include "MouseSelectable.h"
#include "MouseIntersectable.h"
#include "JointEntity.h"
#include "LoopEntity.h"
#include "ComponentRefParent.h"
#include "PlacementComponent.h"
#include "Rectangle.h"
#include "Offset.h"
#include "Scale.h"

namespace middle {


	std::vector<int>findConnectedConstraints(GameState* gameState, Id id) {
		std::vector<int> result;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			Shape& shape = gameState->shapes[i];
			auto constraint = getComponent<components::Constraint>(shape);
			if (constraint == nullptr)
				continue;

			if (constraint->idA == id || constraint->idB == id)
				result.push_back(i);
		}
		return result;
	}

	int constraintExistsAt(GameState* gameState, Id idA, Id idB) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			Shape& shape = gameState->shapes[i];
			if (!isValidId(gameState, shape.id))
				continue;
			auto constraint = getComponent<components::Constraint>(shape);
			if (constraint == nullptr)
				continue;

			if (constraint->idA == idA && constraint->idB == idB)
				return i;

			if (constraint->idB == idA && constraint->idA == idB)
				return i;
		}

		return UNASSIGNED;
	}

	int findFreeIndex(GameState* gameState)
	{
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			middle::Id id = gameState->ids[i];
			if ((id.index == middle::UNASSIGNED
				|| id != gameState->shapes[id.index].id
				|| id.generation < 0)) {
				return i;
			}
		}
		assert(true);
	}

	void unselect(GameState* gameState) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			auto& shape = gameState->shapes[i];
			auto selectableComponent = getComponent<components::MouseSelectable>(shape);
			if (selectableComponent) {
				selectableComponent->selected = false;
			}
		}
	}

	int findHighestLevelContainer(GameState* gameState, int index)
	{
		if (!isValidId(gameState, gameState->ids[index]))
			return UNASSIGNED;
		Shape& shape = gameState->shapes[index];
		middle::Id parentId = middle::getParent(gameState, shape.id);
		if (parentId.index == UNASSIGNED) {
			return index;
		}

		return findHighestLevelContainer(gameState, parentId.index);
	}

	int findHighestUsedIndex(GameState* gameState)
	{
		int highestI = 0;
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (isValidId(gameState, gameState->ids[i])) {
				highestI = i;
			}
		}
		return highestI;
	}

	int findNextFreeGhostIndex(GameState* gameState)
	{
		int highestUsed = findHighestUsedIndex(gameState) + 1;
		return highestUsed > GHOST_INDEX_OFFSET ? highestUsed : GHOST_INDEX_OFFSET;
	}

	void dragShape(GameState* gameState, int index, Vector3 linearVelocity) {
		Shape& shape = getShape(gameState, index);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, shape.id, children);
		for (int i = 0; i < children.size(); ++i) {
			Id memberId = children[i];
			dragShape(gameState, memberId.index, linearVelocity);
		}

		Vec linearVel = DescVec(linearVelocity);
		auto pData = getComponent<components::PhysicsData>(shape);
		auto posData = getComponent<components::Position>(shape);
		if (pData != nullptr) {
			pData->velX = linearVel.x;
			pData->velY = linearVel.y;
			pData->velZ = linearVel.z;
		}
		else if (posData) {
			Vec currPos = { posData->posX, posData->posY, posData->posZ };
			Vec newPos = AddV(currPos, ScaleV(linearVel, gameState->frameTime));
			posData->posX = newPos.x;
			posData->posY = newPos.y;
			posData->posZ = newPos.z;
		}
	}

	void moveShape(GameState* gameState, int index, const Vector3& displacement)
	{
		Shape& shape = gameState->shapes[index];
		std::vector<middle::Id>children;
		middle::getChildren(gameState, shape.id, children);
		for (int i = 0; i < children.size(); ++i) {
			Id memberId = children[i];
			assert(index != memberId.index);
			moveShape(gameState, memberId.index, displacement);
		}
		if (index == 1219) {
			int a = 0;
		}

		auto pos = getComponent<components::Position>(shape);
		if (pos) {
			pos->posX += displacement.x;
			pos->posY += displacement.y;
			pos->posZ += displacement.z;
		}
	}

	bool isGhostShape(int index)
	{
		return index >= GHOST_INDEX_OFFSET;
	}

	bool isRecursiveChildOf(GameState* gameState, int childIndex, int parentIndex)
	{
		auto& parent = getShape(gameState, parentIndex);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, parent.id, children);

		for (Id& id : children) {
			if (id.index == childIndex) {
				return true;
			}
			if (isRecursiveChildOf(gameState, childIndex, id.index)) {
				return true;
			}
		}
		return false;
	}

	bool isEntityOfType(GameState* gameState, int index, const std::vector<int>& entity)
	{
		auto& shape = getShape(gameState, index);
		if (shape.componentMap.size() != entity.size()) {
			return false;
		}
		for (int componentTypeId : entity) {
			if (shape.componentMap.find(componentTypeId) == shape.componentMap.end()) {
				return false;
			}
		}
		return true;
	}

	bool isShapeSelected(GameState* gameState, int index) {
		auto& shape = gameState->shapes[index];
		auto selectedComponent = getComponent<components::MouseSelectable>(shape);
		if (selectedComponent) {
			return selectedComponent->selected;
		}
		return false;
	}

	bool isMouseIntersectingShape(GameState* gameState, int index)
	{
		auto& shape = gameState->shapes[index];
		auto intersectable = getComponent<components::MouseIntersectable>(shape);
		if (intersectable) {
			return intersectable->intersecting;
		}
		return false;
	}

	bool isShapeAlive(GameState* gameState, int index) {
		return gameState->shapes[index].id == gameState->ids[index] && gameState->shapes[index].id.generation >= 0 && gameState->shapes[index].componentMap.size() > 0;
	}

	bool isValidId(GameState* gameState, middle::Id id)
	{
		return id.index != middle::UNASSIGNED
			&& gameState->ids[id.index] == id
			&& id == gameState->shapes[id.index].id
			&& gameState->shapes[id.index].componentMap.size() > 0
			&& id.generation >= 0;
	}


	Vector3 getShapePosition(GameState* gameState, int index)
	{
		auto& shape = getShape(gameState, index);
		auto position = getComponent<components::Position>(shape);
		if (!position) {
			assert(false);
		}
		Vector3 result = { position->posX, position->posY, position->posZ };
		auto offset = getComponent<components::Offset>(shape);
		if (offset) {
			result.x += offset->offsetX;
			result.y += offset->offsetY;
			result.z += offset->offsetZ;
		}
		return result;
	}

	Shape& getShape(GameState* gameState, int index)
	{
		if (gameState->shapes[index].id == gameState->ids[index]) {
			return gameState->shapes[index];
		}
		assert(false);
	}

	void deleteShape(GameState* gameState, int index, bool deleteComponentsOnly) {
		if (!isShapeAlive(gameState, index)) {
			return;
		}

		// remove parent indexes if deleting loops from children
		std::vector<middle::Id>children;
		middle::getChildren(gameState, gameState->shapes[index].id, children);

		// remove childs references to this shape
		for (middle::Id& childId : children) {
			if (isShapeAlive(gameState, childId.index)) {
				auto& childShape = gameState->shapes[childId.index];
				if (childShape.componentMap.size() == 0) {
					continue;
				}
				auto childLoop = getComponent<components::LoopSociety>(childShape);
				childLoop->parentLoopId.index = UNASSIGNED;
			}
		}

		middle::Id parentId = middle::getParent(gameState, gameState->shapes[index].id);
		// remove parent refernce to this shape
		if (parentId.index != middle::UNASSIGNED) {
			auto& parentShape = getShape(gameState, parentId.index);
			auto parentLoop = getComponent<components::LoopSociety>(parentShape);
			for (int i = 0; i < parentLoop->loopMemberIds.size(); ++i) {
				Id parentChildIndex = parentLoop->loopMemberIds[i];
				if (parentChildIndex.index == index) {
					parentLoop->loopMemberIds.erase(parentLoop->loopMemberIds.begin() + i);
					break;
				}
			}
		}

		auto componentRefParent = getComponent<components::ComponentRefParent>(gameState->shapes[index]);
		if (componentRefParent) {
			for (Id childId : componentRefParent->memberIds) {
				deleteShape(gameState, childId.index, deleteComponentsOnly);
			}
		}

		for (auto& pair : gameState->shapes[index].componentMap) {
			Component c = pair.second;
			int typeId = pair.first;
			// store changed component typeids to trigger cache updates
			gameState->componentTypeIdSetWithStructuralChanges.insert(typeId);

			componentListMap[typeId]->shrink(c.componentOffset);
		}

		auto& delShape = gameState->shapes[index];
		if (deleteComponentsOnly) {
			delShape.componentMap.clear();
		}

		if (!deleteComponentsOnly) {
			int prevGeneration = gameState->shapes[index].id.generation;
			gameState->shapes[index] = Shape();
			delShape.id.generation = prevGeneration + 1;
		}
	}

	void deleteShapeRecursive(GameState* gameState, int index, bool deleteComponentsOnly) {
		if (!isShapeAlive(gameState, index)) {
			return;
		}
		Shape& shape = gameState->shapes[index];
		std::vector<middle::Id>children;
		middle::getChildren(gameState, shape.id, children);
		int size = children.size();
		for (int i = size - 1; i >= 0; --i) {
			middle::Id& childId = children[i];
			deleteShapeRecursive(gameState, childId.index, deleteComponentsOnly);
		}
		deleteShape(gameState, index, deleteComponentsOnly);
	}

	Shape& registerShape(GameState* gameState, middle::Shape shape)
	{
		int freeIndex = findFreeIndex(gameState);
		shape.id.generation = gameState->shapes[freeIndex].id.generation + 1;
		shape.id.index = freeIndex;
		gameState->ids[freeIndex] = shape.id;
		gameState->shapes[freeIndex] = shape;
		middle::Shape& newShape = gameState->shapes[freeIndex];
		for (auto& pair : newShape.componentMap) {
			int typeId = pair.first;
			gameState->componentTypeIdSetWithStructuralChanges.insert(typeId);
		}
		return newShape;
	}

	Shape& registerShapeAtIndex(GameState* gameState, middle::Shape shape, int index)
	{
		shape.id.generation = gameState->shapes[index].id.generation + 1;
		shape.id.index = index;
		gameState->ids[index] = shape.id;
		gameState->shapes[index] = shape;
		middle::Shape& newShape = gameState->shapes[index];
		for (auto& pair : newShape.componentMap) {
			int typeId = pair.first;
			gameState->componentTypeIdSetWithStructuralChanges.insert(typeId);
		}
		return newShape;
	}

	Shape& registerAsGhostShape(GameState* gameState, middle::Shape shape) {
		int freeIndex = findNextFreeGhostIndex(gameState);
		shape.id.generation = gameState->shapes[freeIndex].id.generation + 1;
		shape.id.index = freeIndex;
		gameState->ids[freeIndex] = shape.id;
		gameState->shapes[freeIndex] = shape;
		middle::Shape& newShape = gameState->shapes[freeIndex];
		for (auto& pair : newShape.componentMap) {
			int typeId = pair.first;
			gameState->componentTypeIdSetWithStructuralChanges.insert(typeId);
		}
		return newShape;
	}

	Shape& insertShape(GameState* gameState, middle::Id& id)
	{
		Shape shape;
		shape.id = id;
		gameState->ids[id.index] = id;
		gameState->shapes[id.index] = shape;
		for (auto& pair : shape.componentMap) {
			int typeId = pair.first;
			gameState->componentTypeIdSetWithStructuralChanges.insert(typeId);
		}
		return gameState->shapes[id.index];
	}

	Shape& addGhostShape(GameState* gameState) {
		Shape shape;
		int index = findNextFreeGhostIndex(gameState);
		shape.id.generation = gameState->shapes[index].id.generation + 1;
		shape.id.index = index;
		gameState->ids[index] = shape.id;
		gameState->shapes[index] = shape;
		return gameState->shapes[index];
	}

	void moveCameraXZ(Camera3D& initCamera, const Vector3& pos)
	{
		Vector3 displacement = pos - initCamera.position;
		initCamera.position += displacement;
		initCamera.target += displacement;
	}
	std::vector<int> getSelectedShapes(GameState* gameState)
	{
		std::vector<int>result;
		loopInstances(gameState, [&result](int i, Shape& shape) {
			auto selectable = getComponent<components::MouseSelectable>(shape);
			if (selectable && selectable->selected) {
				result.push_back(i);
			}
			return true;
			});
		return result;
	}
	int getMouseIntersectedShape(GameState* gameState)
	{
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isShapeAlive(gameState, i))
				continue;
			if (isMouseIntersectingShape(gameState, i)) {
				return i;
			}
		}
		return UNASSIGNED;
	}

	Id copyShape(GameState* gameState, int shapeToCopyIndex, int parentIndex) {
		std::vector<FieldInfo> ogFields;
		std::vector<FieldInfo> copyFields;
		// resize with 100 probabbly no components with that many fields.. hopefully
		int maxFieldCount = 100;
		ogFields.resize(maxFieldCount);
		copyFields.resize(maxFieldCount);

		Shape& ogShape = getShape(gameState, shapeToCopyIndex);

		int freeIndex;
		if (isGhostShape(shapeToCopyIndex)) {
			freeIndex = findNextFreeGhostIndex(gameState);
		}
		else {
			freeIndex = findFreeIndex(gameState);
		}
		Shape newShape;

		// copy components to the new shape
		for (auto& pair : ogShape.componentMap) {

			int typeId = pair.first;
			Component component = pair.second;

			// grow component vector.. add new component for the copy
			int copyOffset = componentListMap[typeId]->grow();

			// get og serializable to get fields
			Serializable* ogSerializable =
				componentListMap[typeId]->getSerializable(component.componentOffset);

			// get fields
			int ogSize = 0;
			ogSerializable->getFields(ogFields, &ogSize);


			// get copy serializable to get fields
			auto copySerializable = componentListMap[typeId]->getSerializable(copyOffset);

			// create component ref for the shape
			Component copyComponent;
			copyComponent.componentOffset = copyOffset;
			newShape.componentMap[typeId] = copyComponent;


			int copySize = 0;
			copySerializable->getFields(copyFields, &copySize);

			// copy fields to components
			for (int i = 0; i < ogSize; ++i) {
				FieldInfo& ogField = ogFields[i];
				FieldInfo& copyField = copyFields[i];
				switch (ogField.type) {
				case FieldType::Bool: {
					bool* valueptr = static_cast<bool*>(copyField.value);
					*valueptr = *static_cast<bool*>(ogField.value);
					break;
				}
				case FieldType::Double: {
					double* valueptr = static_cast<double*>(copyField.value);
					*valueptr = *static_cast<double*>(ogField.value);
					break;
				}
				case FieldType::Float: {
					float* valueptr = static_cast<float*>(copyField.value);
					*valueptr = *static_cast<float*>(ogField.value);
					break;
				}
				case FieldType::Id: {
					Id* valueptr = static_cast<Id*>(copyField.value);
					//*valueptr = *static_cast<Id*>(ogField.value);
					*valueptr = middle::Id();
					break;
				}
				case FieldType::IdVector: {
					std::vector<Id>* valueptr = static_cast<std::vector<Id>*>(copyField.value);
					*valueptr = *static_cast<std::vector<Id>*>(ogField.value);
					for (middle::Id& id : *valueptr) {
						id = middle::Id();
					}
					break;
				}
				case FieldType::Int: {
					int* valueptr = static_cast<int*>(copyField.value);
					*valueptr = *static_cast<int*>(ogField.value);
					break;
				}
				case FieldType::String: {
					std::string* valueptr = static_cast<std::string*>(copyField.value);
					*valueptr = *static_cast<std::string*>(ogField.value);
					break;
				}
				case FieldType::Quaternion: {
					Quaternion* valueptr = static_cast<Quaternion*>(copyField.value);
					*valueptr = *static_cast<Quaternion*>(ogField.value);
					break;
				}
				case FieldType::Vector3: {
					Vector3* valueptr = static_cast<Vector3*>(copyField.value);
					*valueptr = *static_cast<Vector3*>(ogField.value);
					break;
				}
				case FieldType::Vector2: {
					Vector2* valueptr = static_cast<Vector2*>(copyField.value);
					*valueptr = *static_cast<Vector2*>(ogField.value);
					break;
				}
				case FieldType::Color: {
					Color* valueptr = static_cast<Color*>(copyField.value);
					*valueptr = *static_cast<Color*>(ogField.value);
					break;
				}
				default:
					assert(true, "not supported");
				}
			}


		}

		newShape = middle::registerShape(gameState, newShape);

		return newShape.id;
	}


	Id deepCopyShape(GameState* gameState, int shapeToCopyIndex, int parentIndex) {

		Shape& ogShape = getShape(gameState, shapeToCopyIndex);

		middle::Id newShapeId = copyShape(gameState, shapeToCopyIndex, parentIndex);
		auto& newShape = middle::getShape(gameState, newShapeId.index);

		auto copyLoop = getComponent<components::LoopSociety>(newShape);
		if (copyLoop) {

			auto ogLoop = getComponent<components::LoopSociety>(ogShape);

			if (parentIndex >= 0) {
				Shape& parentShape = getShape(gameState, parentIndex);
				copyLoop->parentLoopId = parentShape.id;
			}
			else {
				copyLoop->parentLoopId = middle::Id();
			}

			// clear exact copies from earlier absolute copy
			copyLoop->loopMemberIds.clear();

			// copy children and assign their ids as children to the new copied shape
			for (Id& id : ogLoop->loopMemberIds) {
				Id childCopy = deepCopyShape(gameState, id.index, newShape.id.index);
				// update pointer since deepcopy might rearrange component vector
				copyLoop = getComponent<components::LoopSociety>(newShape);
				copyLoop->loopMemberIds.push_back(childCopy);
			}
		}

		return newShape.id;

	}
	Id deepCopyShapeToStorage(GameState* gameState, int shapeToCopyIndex, int parentIndex)
	{
		return Id();
	}

	std::vector<Vector3> getRectVertices(GameState* gameState, const Id& shapeId)
	{
		auto& shape = getShape(gameState, shapeId.index);
		auto rect = getComponent<components::Rectangle>(shape);
		Vector3 position = getShapePosition(gameState, shapeId.index);
		Vector3 s = getTotalScale(gameState, shapeId);
		std::vector<Vector3> vertices;
		vertices.resize(4);
		vertices[0] = { -rect->width * 0.5f * s.x, 0, rect->height * 0.5f * s.z };
		vertices[1] = { -rect->width * 0.5f * s.x, 0, -rect->height * 0.5f * s.z };
		vertices[2] = { rect->width * 0.5f * s.x, 0, -rect->height * 0.5f * s.z };
		vertices[3] = { rect->width * 0.5f * s.x, 0, rect->height * 0.5f * s.z };
		vertices[0] += position;
		vertices[1] += position;
		vertices[2] += position;
		vertices[3] += position;
		return vertices;
	}
	Vector3 getTotalScale(GameState* gameState, const Id& shapeId)
	{
		auto& shape = getShape(gameState, shapeId.index);
		auto scale = middle::getComponent<components::Scale>(shape);
		if (!scale) {
			return { 1,1,1 };
		}
		middle::Id parentId = middle::getParent(gameState, shape.id);
		if (parentId.index != middle::UNASSIGNED) {
			return scale->scale * getTotalScale(gameState, parentId);
		}
		return scale->scale;
	}
	Id getParent(GameState* gameState, Id& id)
	{
		Shape& shape = getShape(gameState, id.index);
		auto loopSociety = middle::getComponent<components::LoopSociety>(shape);
		if (!loopSociety) {
			return middle::Id();
		}
		if (!isValidId(gameState, loopSociety->parentLoopId)) {
			return middle::Id();
		}
		return loopSociety->parentLoopId;
	}
	void getChildren(GameState* gameState, Id id, std::vector<Id>& result)
	{
		Shape& shape = getShape(gameState, id.index);
		auto loop = getComponent<components::LoopSociety>(shape);
		if (loop) {
			for (Id& childId : loop->loopMemberIds) {
				if (isValidId(gameState, childId)) {
					result.push_back(childId);
				}
			}
		}
	}

	void getAllChildren(GameState* gameState, Id id, std::vector<Id>& result)
	{
		Shape& shape = getShape(gameState, id.index);
		auto loop = getComponent<components::LoopSociety>(shape);
		if (loop) {
			for (Id& childId : loop->loopMemberIds) {
				if (isValidId(gameState, childId)) {
					result.push_back(childId);
					getAllChildren(gameState, childId, result);
				}
			}
		}
	}

	void getChildrenWithComp(GameState* gameState, Id id, std::vector<Id>& result, int typeId)
	{
		Shape& shape = getShape(gameState, id.index);
		auto loop = getComponent<components::LoopSociety>(shape);
		if (loop) {
			for (Id& childId : loop->loopMemberIds) {
				if (isValidId(gameState, childId)) {
					auto& child = middle::getShape(gameState, childId.index);
					if (child.componentMap.find(typeId) != child.componentMap.end()) {
						result.push_back(childId);
					}
				}
			}
		}
	}

	void getAllChildrenWithComp(GameState* gameState, Id id, std::vector<Id>& result, int typeId)
	{
		Shape& shape = getShape(gameState, id.index);
		auto loop = getComponent<components::LoopSociety>(shape);
		if (loop) {
			for (Id& childId : loop->loopMemberIds) {
				if (isValidId(gameState, childId)) {
					auto& child = middle::getShape(gameState, childId.index);
					if (child.componentMap.find(typeId) != child.componentMap.end()) {
						result.push_back(childId);
					}
					getAllChildrenWithComp(gameState, childId, result, typeId);
				}
			}
		}

	}



	middle::Id getFirstChildWithComponent(GameState* gameState, Id& id, int typeId)
	{
		std::vector<middle::Id>children;
		middle::getChildren(gameState, id, children);
		for (middle::Id& childId : children) {
			auto& childShape = middle::getShape(gameState, childId.index);
			if (childShape.componentMap.find(typeId) != childShape.componentMap.end()) {
				return childId;
			}
		}
		return middle::Id();
	}
	int getLoopIndex(GameState* gameState, Id& parentId, Id& childId)
	{
		auto& parentShape = getShape(gameState, parentId.index);
		std::vector<middle::Id>children;
		middle::getChildren(gameState, parentId, children);
		for (int i = 0; i < children.size(); ++i) {
			if (children[i] == childId) {
				return i;
			}
		}
		return middle::UNASSIGNED;
	}
	middle::Id findFirstShapeWithComp(GameState* gameState, int typeId)
	{
		middle::Id id;
		middle::loopInstances(gameState, [gameState, &id, &typeId](int i, middle::Shape& shape) {
			if (shape.componentMap.find(typeId) != shape.componentMap.end()) {
				id = shape.id;
				return false;
			}
			return true;
			});
		return id;
	}
	void findShapesWithComp(GameState* gameState, std::vector<Id>& result, int typeId)
	{
		middle::loopInstances(gameState, [gameState, &result, &typeId](int i, middle::Shape& shape) {
			if (shape.componentMap.find(typeId) != shape.componentMap.end()) {
				result.push_back(shape.id);
			}
			return true;
			});
	}
	bool isIdCurrent(GameState* gameState, middle::Id& id)
	{
		if (id.index == middle::UNASSIGNED) {
			return false;
		}
		assert(gameState->ids[id.index] == gameState->shapes[id.index].id);
		return gameState->ids[id.index] == id;
	}

	components::CompCache* newCompCache(GameState* gameState, const std::string& systemName)
	{
		auto newCache = std::make_unique<components::CompCache>();
		newCache->systemName = systemName;
		gameState->compCaches.push_back(
			std::move(newCache)
		);
		return gameState->compCaches.back().get();
	}

	void queueAction(GameState* gameState, std::shared_ptr<EditorActionContainer> container)
	{
		gameState->actionQueue.push(container);
	}

	void queueEditorAction(GameState* gameState, std::shared_ptr<EditorActionContainer> container)
	{
		gameState->actionQueue.push(container);
		while (gameState->editorState.historySinkDepth > 0) {
			gameState->editorState.actionHistory.pop_back();
			--gameState->editorState.historySinkDepth;
		}
		gameState->editorState.actionHistory.push_back(container);
	}


	//components::CompCache* newCompCache(GameState* gameState)
	//{
	//	gameState->compCaches.push_back(
	//		std::make_unique<components::CompCache>()
	//	);
	//	return gameState->compCaches.back().get();
	//}
}
