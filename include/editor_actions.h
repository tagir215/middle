#pragma once
#include "game_state.h"

namespace middle {

	// creation of new spheres are here
	class EditorActionNewSphere : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	// creation of new constraints happened here
	class EditorActionNewConstraint : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	// deletion of selected things happening here actually
	class EditorActionDelete : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	// time to save positions, please don't forget to do it
	class EditorActionSaveScene : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	// build from the editor.  hopefully no crashes...
	class EditorActionBuild : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	// load scene new exiting start
	class EditorActionLoadScene : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	// create loops, 
	class EditorActionCreateLoop : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	// new scene, new world
	class EditorActionNewScene : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	// what is import scene? it's so that you can import scenes as objects, or loops 
	class EditorActionImportScene : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	// this will create scripts if theres no script, and open script if there is script.
	// it is so much work to find scripts. Finding things from list of files is not smart. You need to use the editor
	class EditorActionOpenScript : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	class EditorActionNewCamera : public EditorActionContainer {
	public:
		void execute(GameState* gameState) override;
	};

	void processEditorActions(GameState* gameState);
}
