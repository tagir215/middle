#pragma once
#include "middle_constants.h"
#include <string>
#include "game_colors.h"
#include <set>
#include <memory>
#include <unordered_map>

namespace middle {
	static float DEF_RADIUS = 2;
	static float DEF_RADIUS_LOOP_INDICATOR = 4;
	static float DEF_RADIUS_REFERENCE_INDICATOR = 9;
	static float DEF_RADIUS_CAMERA = 5;
	static float DEF_RADIUS_SYSTEM = 4;
	static float DEF_RADIUS_COMPONENT = 3;
	static midPrimitive::Color DEF_COLOR = UGLY_PINK;
	static float DEF_LIFETIME = INFINITY;
	static float DEF_GRAVITY = 0;
	static float DEF_DAMPING = 0.8f;
	static int DEF_HISTORY_MEMEORY_LENGTH = 10;
	static float DEF_STIFFNESS = 0.8f;
	static float DEF_MASS = 1;
	static float DEF_INERTIA = 1;
	static float DEF_LINE_PADDING_H = -3.0f;
	static float DEF_LINE_PADDING_V = 2.2f;

	struct Id {
		int index = UNASSIGNED;
		int generation = 0;
		bool operator==(const Id& other) const {
			return other.generation == generation && other.index == index;
		}

		bool operator!=(const Id& other) const {
			return !(*this == other);
		}
	};

	struct IdHash {
		std::size_t operator()(const Id& id) const {
			return std::hash<int>{}(id.index) ^
				(std::hash<int>{}(id.generation) << 1);
		}
	};

	typedef int componentOffset;
	typedef int compoenntTypeId;

	struct Shape {
		Id id;
		std::vector<componentOffset>componentOffsets;
		std::vector<compoenntTypeId>componentTypes;
		std::set<int>affectingSystems;
	};


}
