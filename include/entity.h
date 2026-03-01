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
		int index = UNASSIGNED;
		int generation = 0;
		bool operator==(const Id& other) {
			return other.generation == generation && other.index == index;
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

	enum class FieldType : char {
		Bool = 'b',
		Int = 'i',
		String = 's',
		Float = 'f',
		Double = 'd',
		Vector3 = 'v',
		Vector2 = 'z',
		Quaternion = 'r',
		Color = 'c',
		Id = 'q',
		IdVector = 'p',
	};

	struct Serializer {
		std::ostream& ostream;
		template<typename T>
		void operator()(const char* name, T& value){
			ostream << middle::fieldToString(value);
		}
	};

	struct Deserializer {
		const std::vector<std::string>& buffer;
		int indexOffset;
		int index = 0;
		int bufferSize = buffer.size();
		template<typename T>
		void operator()(const char* name, T& value) {
			if(index >= bufferSize){
				return;
			}
			middle::fillField(&value, buffer[index++], indexOffset);
		}
	};
	struct FieldInfo {
		const char* name;
		void* value;
		FieldType type;
	};
	struct FieldCollector {
		std::vector<FieldInfo>& fields;
		int* size;
		template<typename T>
		void operator()(const char* name, T& value) {
			fields[*size].name = name;
			fields[*size].value = &value;
			fields[*size].type = middle::fieldToType(value);
			++(*size);
		}
	};

	struct Serializable {
		virtual ~Serializable() = default;
		virtual void serialize(std::ostream& istream) = 0;
		virtual void deserialize(const std::vector<std::string>& buffer, int indexOffset) = 0;
		virtual void getFields(std::vector<FieldInfo>& fields, int* size) = 0;
	};
}
