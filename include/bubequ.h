#pragma once
#include <vector>
#include <string>
#include <memory>

namespace bubequ {


	enum class LinkType {
		NONE, 
		MULTIPLICATION,
		POWER,
		EQUALS,
		GREATER,
		GREATER_OR_EQUAL,
		FUNCTION,
		SUMMATION,
	};

	enum class UnitType {
		NONE,
		CONSTANT,
		VARIABLE,
		ZERO,
	};

	struct Scope {
		std::string hash;
		// actual loaded children
		std::vector<std::shared_ptr<Scope>>children;
		virtual ~Scope() = default;
	};

	// path from root to node
	typedef std::vector<int> BubTraversePath;


	struct Unit : public Scope{
		UnitType type;
		std::string label;
		int value = -1;
	};
	struct Link : public Scope {
		LinkType type;
		std::string label;
	};


	struct SentenceUnit{
		std::string text = "";
		std::string varLabel = "";
	};

	struct WordProblem {
		std::string rawText;
		std::vector<SentenceUnit> sentenceUnits;
	};

	struct WordProblemMobjs {
		std::shared_ptr<WordProblem> problem;
		std::shared_ptr<Scope> solutionMobj;
	};
}
