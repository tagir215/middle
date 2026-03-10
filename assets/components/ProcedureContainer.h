#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEPROCEDURECONTAINER(X) \
	X(startBlock)

namespace procedureConstants {
	const int FORWARD = 1;
	const int BACKWARD = -1;
	const int IDLE = 0;
	const int EXECUTING = 1;
	const int STEPPING = 2;

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
		int mode = procedureConstants::IDLE;
		int direction = procedureConstants::FORWARD;
		std::vector<procedureConstants::ProcedureTransition> procedureTransitionStack;
		bool reset = false;
		bool exitingLoop = false;

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
