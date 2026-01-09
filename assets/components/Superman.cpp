#include "Superman.h"

namespace components {
		void Superman::serialize(std::ostream& ostream) {
			ostream << middle::fieldToString(power);
		}

		void Superman::deserialize(const std::vector<std::string>& buffer) {
			middle::fillField(&power, buffer[0]);
		}
}
