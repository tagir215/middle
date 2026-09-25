#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEBUBBLEEQUALSVARIABLE(X) 

namespace components {
	struct BubbleEqualsVariable : public middle::Serializable{
		std::string variableLabel;
		bool wantsToReplaceVariable = false;
		bool wantsToReplaceBubble = false;
		middle::Id matchingIdRef;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEBUBBLEEQUALSVARIABLE(X)
#undef X
		}
	};
}
