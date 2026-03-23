#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEPROCEDURECONTAINER(X) \
	X(startBlock) \
	X(bubbleRef)

namespace procedureConstants {
	enum Direction {
		FORWARD = 1,
		BACKWARD = -1,
	};

	enum Mode {
		IDLE = 0,
		EXECUTING = 1,
		STEPPING = 2,
	};

	enum TransitionType {
		Start,
		South,
		NorthEast,
		SouthEast,
		West,
		End,
	};

	struct ProcedureTransition {
		TransitionType type;
		middle::Id previousId;
		middle::Id destinationId;
		std::shared_ptr<middle::EditorActionContainer>action;
	};


	enum StepStatus {
		CanStep,
		CannotStep,
		Stationary,
	};
}

namespace components {
	struct ProcedureContainer : public middle::Serializable {
		middle::Id activeBlock;
		middle::Id startBlock;
		middle::Id bubbleRef;
		int mode = procedureConstants::IDLE;
		int direction = procedureConstants::FORWARD;
		// action container, and all actions in history
		std::vector<procedureConstants::ProcedureTransition> procedureTransitionStack;
		bool exitingLoop = false;
		// target size or position for execution 
		int targetActionStackSize = 0;
		// whether mouse is intersecting the code blocks, which has effect on execution position
		bool intersecting = false;
		// number of top level blocks
		int size = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEPROCEDURECONTAINER(X)
#undef X
		}
	};
}
