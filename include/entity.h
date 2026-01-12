#pragma once
#include "middle_constants.h"
#include <string>
#include "descart_constraints.h"
#include "game_colors.h"
#include "physics_body.h"
#include <set>
#include <memory>
#include <unordered_map>

namespace middle {
	static float DEF_RADIUS = 2;
	static float DEF_RADIUS_LOOP_INDICATOR = 4;
	static float DEF_RADIUS_REFERENCE_INDICATOR = 5;
	static float DEF_RADIUS_CAMERA = 5;
	static float DEF_RADIUS_SYSTEM = 4;
	static float DEF_RADIUS_COMPONENT = 3;
	static Color DEF_COLOR = UGLY_PINK;
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
		int generation = UNASSIGNED;
		bool operator==(const Id& other) {
			return other.generation == generation;
		}

		bool operator!=(const Id& other) {
			return !(*this == other);
		}
	};

	struct Component {
		// offset where the shapes component is in component vector in middle_component_table (currently)
		int componentOffset;
	};

	struct Shape {
		Id id;
		std::unordered_map<int, Component> componentMap;
	};
}
