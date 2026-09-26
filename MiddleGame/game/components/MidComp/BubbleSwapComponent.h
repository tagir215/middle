#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEBUBBLESWAPCOMPONENT(X) \
	X(activeIndex)

namespace components {

	enum BubbleSwapRole {
		WORD_PROBLEM,
		SOLUTION_BUBBLE
	};

	enum SwapComponentStatus {
		SWAP_DISABLED,
		SWAP_ENABLED
	};

	struct BubbleSwapComponent : public middle::Serializable{
		int activeIndex = 0;
		SwapComponentStatus status;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;

		template<typename V>
		void reflect(V& v) {
#define X(f) v(#f, f);
			MIDDLEBUBBLESWAPCOMPONENT(X)
#undef X
		}
	};
}
