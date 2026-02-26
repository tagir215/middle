#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLECODEBLOCK(X) \
	X(type)

namespace components {
	struct CodeBlock : public middle::Serializable{
		int type = 0;
		bool exitLoop = false;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLECODEBLOCK(X)
#undef X
		}
	};
}

namespace codeBlockTypes {
	inline int BLOCK = 0; 
	inline int LOOP_BLOCK = 1;
}
