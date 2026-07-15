#pragma once
#include <vector>
#include <string>

namespace bubequ {


	enum class LinkType {
		NONE, 
		ADDITION,
		MULTIPLICATION,
		POWER,
		EQUALS
	};

	enum class UnitType {
		NONE,
		CONSTANT,
		VARIABLE
	};

	struct Scope {
		std::vector<Scope>children;
	};

	struct Unit : public Scope{
		UnitType type;
		std::string label;
		int value = -1;
	};
	struct Link : public Scope {
		LinkType type;
	};

}
