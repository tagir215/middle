#include "BubbleComponent.h"

namespace components {
	void BubbleComponent::serialize(std::ostream& ostream) {

	}

	void BubbleComponent::deserialize(const std::vector<std::string>& buffer, int indexOffset) {

	}

	static middle::ComponentRegistrar<BubbleComponent>reg("BubbleComponent");
}
