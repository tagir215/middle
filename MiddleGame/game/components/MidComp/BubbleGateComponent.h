#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEBUBBLEGATECOMPONENT(X) \
	X(status)

namespace components {

	enum BubbleGateStatus {
		CLOSED,
		OPEN,
		DUMMY
	};

	struct BubbleGateComponent : public middle::Serializable{
		int status = BubbleGateStatus::CLOSED;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEBUBBLEGATECOMPONENT(X)
#undef X
		}
	};
}
