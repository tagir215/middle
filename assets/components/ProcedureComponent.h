#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEPROCEDURECOMPONENT(X) \
	X(executing)

namespace components {
	struct ProcedureComponent : public middle::Serializable{
		bool executing = false;
		middle::Id activeCodeBlock;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEPROCEDURECOMPONENT(X)
#undef X
		}
	};
}
