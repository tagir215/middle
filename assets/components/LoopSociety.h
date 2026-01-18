#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define LOOPSOCIETY(X) \
	X(parentLoopId) \
	X(loopMemberIds)

namespace components {
	struct LoopSociety : public middle::Serializable{
		middle::Id parentLoopId;
		std::vector<middle::Id>loopMemberIds;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;
		template<typename V>
		void reflect(V& v) {
		#define X(f) v(#f, f);
			LOOPSOCIETY(X)
		#undef X
		}
	};
}
