#pragma once
#include <vector>
#include <string>
#include <memory>

namespace bubequ {


	enum class LinkType {
		NONE, 
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
		std::vector<std::shared_ptr<Scope>>children;
		virtual ~Scope() = default;
	};

	struct Unit : public Scope{
		UnitType type;
		std::string label;
		int value = -1;
	};
	struct Link : public Scope {
		LinkType type;
	};


	struct SentenceUnit{
		std::string text = "";
		std::string varLabel = "";
	};

	struct WordProblem {
		std::string rawText;
		std::vector<SentenceUnit> sentenceUnits;
		std::vector<std::shared_ptr<bubequ::Scope>>bubequs;
	};
}
