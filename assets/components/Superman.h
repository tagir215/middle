#pragma once
#include "registrars.h"
#include <fstream>

namespace components {
	static std::string componentName = "Superman";

	struct Superman {
		int power = 0;
	};

	static middle::ComponentRegistrar<Superman> reg(
		componentName, 
		[](Superman& man, std::ostream& outFile)
		{
			outFile << man.power << "\n";
		},
		[](Superman& man, const std::vector<std::string>& buffer)
		{
			man.power = std::stoi(buffer[0]);
		}
	);
}
