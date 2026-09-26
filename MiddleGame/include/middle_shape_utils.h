#pragma once
#include <vector>
#include "game_state.h"

namespace middle {
	// next free index in shape array
	int findFreeIndex(GameState* gameState);
	// constraints connected to shape
	std::vector<int>findConnectedConstraints(GameState* gameState, Id id);
	// this constraint already exists... don't make duplicates
	int constraintExistsAt(GameState* gameState, Id idA, Id idB);
	// unselect selected things
	void unselect(GameState* gameState);
	// move shape and its chidlren
	void moveShape(GameState* gameState, int index, const midMath::Vector3& displacement);
	// set local pos in a way so it has this global pos
	void setGlobalPosition(GameState* gameState, middle::Id id, const midMath::Vector3& targetPos);
	// set local pos
	void setLocalPosition(GameState* gameState, middle::Id id, const midMath::Vector3& targetPos);
	// find container of containers containers
	int findHighestLevelContainer(GameState* gameState, int index);
	// local scale
	midMath::Vector3 getLocalScale(GameState* gameState, middle::Id id);
	// set local scale
	void setLocalScale(GameState* gameState, middle::Id id, const midMath::Vector3& targetScale);
	// loop the shape instances
	int findHighestUsedIndex(GameState* gameState);
	// find next usable ghost index
	int findNextFreeGhostIndex(GameState* gameState);
	// is the shape real editable thing or a ghost (reference from other scene)
	bool isGhostShape(int index);
	// check that if the child belong to the parent or parents children
	bool isRecursiveChildOf(GameState* gameState, int childIndex, int parentIndex);
	// is the shape selected via mouse
	bool isShapeSelected(GameState* gameState, int index);
	// is the mouse intersecting this shape
	bool isMouseIntersectingShape(GameState* gameState, int index);
	// is id generation correct?  and is alive
	bool isValidId(GameState* gameState, middle::Id id);
	// get pos quickly
	midMath::Vector3 getGlobalPosition(GameState* gameState, middle::Id id);
	// get pos quickly very
	midMath::Vector3 getLocalPosition(GameState* gameState, middle::Id id);
	// get shape instance
	Shape& getShape(GameState* gameState, int index);
	// delete shape , updates generational indexes
	void deleteShape(GameState* gameState, int index, bool deleteComponentsOnly = false);
	// deletes shapes and its children
	void deleteShapeRecursive(GameState* gameState, int index, bool deleteComponentsOnly = false);
	// add shape and updates generations
	Shape& registerShape(GameState* gameState, middle::Shape shape);
	// add shape and updates generations
	Shape& registerShapeAtIndex(GameState* gameState, middle::Shape shape, int index);
	// add shape and updates generations
	Shape& registerAsGhostShape(GameState* gameState, middle::Shape shape);
	// add shape, doesn't update generations
	Shape& insertShape(GameState* gameState, middle::Id& id);
	// adds not serialized ghost shape and updates generations
	Shape& addGhostShape(GameState* gameState);
	// move camera in xz plane moving also the target 
	void moveCameraXZ(midPrimitive::Camera3D& initCamera, const midMath::Vector3& pos);
	// get shapes selected..
	std::vector<int>getSelectedShapes(GameState* gameState);
	// return first shape intersect by mouse
	int getMouseIntersectedShape(GameState* gameState);
	// copy shape 
	Id copyShape(GameState* gameState, int shapeToCopyIndex, int parentIndex = UNASSIGNED);
	// copy shape and its children
	Id deepCopyShape(GameState* gameState, int shapeToCopyIndex, int parentIndex = UNASSIGNED);
	// copy shape shallowly and preserve its global coordinate
	Id shallowCopyShapeGlobalCoordinates(GameState* gameState, middle::Id id);
	// copy shape and preserve its global coordinate
	Id deepCopyShapeGlobalCoordinates(GameState* gameState, middle::Id id);
	// get vertices of rectangles
	std::vector<midMath::Vector3>getRectVertices(GameState* gameState, const Id& shapeId);
	// get scale and multiply it with all the parents scales
	midMath::Vector3 getTotalScale(GameState* gameState, const Id& shapeId);
	// get parent of shape with loopSocietyComponent
	Id getParent(GameState* gameState, Id& id);
	// get children in a flat array
	void getChildren(GameState* gameState, Id id, std::vector<Id>& result);
	// get children in a flat array
	void getAllChildren(GameState* gameState, Id id, std::vector<Id>& result);
	// get children in a flat array
	void getChildrenWithComp(GameState* gameState, Id id, std::vector<Id>& result, int typeId);
	// get children in a flat array
	void getAllChildrenWithComp(GameState* gameState, Id id, std::vector<Id>& result, int typeId);
	// iterate children and return first child with component type id
	middle::Id getFirstChildWithComponent(GameState* gameState, Id& id, int typeId);
	// get index of child
	int getLoopIndex(GameState* gameState, Id& parentId, Id& childId);
	// find shape with comp
	middle::Id findFirstShapeWithComp(GameState* gameState, int typeId);
	// get children in a flat array
	void findShapesWithComp(GameState* gameState, std::vector<Id>& result, int typeId);
	// check if is id is assigned and up to date
	bool isIdCurrent(GameState* gameState, middle::Id& id);
	// new comp cache for caching components for better cache locality of components
	components::CompCache* newCompCache(GameState* gameState, const std::string& systemName);
	// queue action
	void queueAction(GameState* gameState, std::shared_ptr<EditorActionContainer> container);
	// queue action for editor, with undos
	void queueEditorAction(GameState* gameState, std::shared_ptr<EditorActionContainer> container);
	// get transform matrix for some id with components globaltransform, localPos, localScale, and probably rotation in future 
	midMath::Matrix getTransformMatrix(GameState* gameState, middle::Id id);
	// project world coordinate as local coordinate 
	midMath::Vector3 projectGlobalCoordinateToLocalCoordinate(GameState* gameState, const midMath::Vector3& globalCoord, middle::Id shapeId);
	// project local coordinate to match old global coordinate
	void updateLocalCoordinateToProjectedGlobalCoordinate(GameState* gameState, middle::Id id, middle::Id oldParentId);
	// project local scale to mathc old global scale
	midMath::Vector3 projectGlobalScaleToLocalScale(GameState* gameState, middle::Id id, const midMath::Vector3& globalScale);
	// get global scale parents scale multiplied
	midMath::Vector3 getGlobalScale(GameState* gameState, middle::Id id);
	// get index on the loop
	int getLoopIndex(GameState* gameState, middle::Id id);
	// update global transforms
	void updateGlobalTransforms(middle::GameState* gameState, middle::Id id, const midMath::Matrix& parentM, const midMath::Vector3& parentScale);
	// notify structural changes for cache updates
	void notifyStructuralChanges(middle::GameState* gameState, middle::Id id, middle::componentType componentType);
	// check whether has comp
	bool hasComp(middle::Shape& shape, int typeId);
	// get offset
	middle::componentOffset getCompOffset(middle::Shape& shape, int typeId);
	// set offset... these are new
	void setCompOffset(middle::Shape& shape, int typeId, int offset);
	// remove comp
	void removeComp(middle::Shape& shape, int typeId);
	// create shape... replace all the old initializations!
	Shape createShape(middle::GameState* gameState);
	// add to rendering list
	void queueForRender(middle::GameState* gameState, middle::RenderItem item);
	// add to ui list
	void queueUi(middle::GameState* gameState, std::function<void()>ui);
	// add block to block blockable blocking needing inputs
	void insertInputBlock(middle::GameState* gameState, middle::InputBlockers block);
	// get Active camera pos
	midPrimitive::Camera3D getActiveCam(middle::GameState* gameState);

	void assertPos(const midMath::Vector3& pos);

	template<typename F>
	void loopInstances(GameState* gameState, F func) {
		for (int i = 0; i < gameState->shapes.size(); ++i) {
			if (!isValidId(gameState, gameState->ids[i]))
				continue;
			if (!func(i, gameState->shapes[i])) {
				break;
			}
		}
	}
}