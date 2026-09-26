#pragma once
#include "entity.h"
#include <string>
#include <any>

namespace middle {

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



	inline std::string Vector3ToString(const midMath::Vector3& v) {
		std::string result = "\n";
		result += std::to_string(v.x) + "\n";
		result += std::to_string(v.y) + "\n";
		result += std::to_string(v.z);
		return result;
	}

	inline std::string Vector2ToString(const midMath::Vector2& v) {
		std::string result = "\n";
		result += std::to_string(v.x) + "\n";
		result += std::to_string(v.y);
		return result;
	}

	inline std::string QuaternionToString(const midMath::Quaternion& q) {
		std::string result = "\n";
		result += std::to_string(q.x) + "\n";
		result += std::to_string(q.y) + "\n";
		result += std::to_string(q.z) + "\n";
		result += std::to_string(q.w) + "\n";
		return result;
	}

	inline std::string ColorToString(const midPrimitive::Color& c) {
		std::string result = "\n";
		result += std::to_string(c.r) + "\n";
		result += std::to_string(c.g) + "\n";
		result += std::to_string(c.b) + "\n";
		result += std::to_string(c.a) + "\n";
		return result;
	}

	inline FieldType fieldToType(const std::any& field) {

		if (field.type() == typeid(std::string)) {
			return FieldType::String;
		}
		else if (field.type() == typeid(int)) {
			return FieldType::Int;
		}
		else if (field.type() == typeid(float)) {
			return FieldType::Float;
		}
		else if (field.type() == typeid(double)) {
			return FieldType::Double;
		}
		else if (field.type() == typeid(bool)) {
			return FieldType::Bool;
		}
		else if (field.type() == typeid(midMath::Vector3)) {
			return FieldType::Vector3;
		}
		else if (field.type() == typeid(midMath::Vector2)) {
			return FieldType::Vector2;
		}
		else if (field.type() == typeid(midMath::Quaternion)) {
			return FieldType::Quaternion;
		}
		else if (field.type() == typeid(midPrimitive::Color)) {
			return FieldType::Color;
		}
		else if (field.type() == typeid(Id)) {
			return FieldType::Id;
		}
		else if (field.type() == typeid(std::vector<Id>)) {
			return FieldType::IdVector;
		}

		assert(false && "no we are not supporting this");
	}

	inline std::string fieldToString(const std::any& field) {
		FieldType type = fieldToType(field);
		std::string result = std::string(1, static_cast<char>(type)) + "  ";

		switch (type) {
		case FieldType::String:
			return result + std::any_cast<const std::string&>(field) + '\n';
		case FieldType::Int:
			return result + std::to_string(std::any_cast<int>(field)) + '\n';
		case FieldType::Float:
			return result + std::to_string(std::any_cast<float>(field)) + '\n';
		case FieldType::Double:
			return result + std::to_string(std::any_cast<double>(field)) + '\n';
		case FieldType::Bool:
			return result + std::to_string(std::any_cast<bool>(field)) + '\n';
		case FieldType::Vector3:
			return result + Vector3ToString(std::any_cast<midMath::Vector3>(field)) + '\n';
		case FieldType::Vector2:
			return result + Vector2ToString(std::any_cast<midMath::Vector2>(field)) + '\n';
		case FieldType::Quaternion:
			return result + QuaternionToString(std::any_cast<midMath::Quaternion>(field)) + '\n';
		case FieldType::Color:
			return result + ColorToString(std::any_cast<midPrimitive::Color>(field)) + '\n';
		case FieldType::Id: {
			middle::Id id = std::any_cast<Id>(field);
			return result + std::to_string(id.index) + '_' + std::to_string(id.generation) + '\n';
		}
		case FieldType::IdVector: {
			auto v = std::any_cast<std::vector<Id>>(field);
			result += '\n';
			for (int i = 0; i < v.size(); ++i) {
				middle::Id id = v[i];
					result += fieldToString(id);
			}
			return result;
		}
		}

		assert(false && "no we are not supporting this");
	}



	struct Serializer {
		std::ostream& ostream;
		template<typename T>
		void operator()(const char* name, T& value){
			ostream << middle::fieldToString(value);
		}
	};

	inline std::vector<std::string> split(const std::string& s, char delim) {
		std::vector<std::string> parts;
		std::size_t start = 0;
		std::size_t pos = 0;

		if (s == "")
			return parts;

		while ((pos = s.find(delim, start)) != std::string::npos) {
			if (pos == start) {
				break;
			}
			parts.push_back(s.substr(start, pos - start));
			start = pos + 1;
		}

		std::string lastSegment = s.substr(start);
		if(start != pos && lastSegment != "")
			parts.push_back(lastSegment);  // last segment
		return parts;
	}

	inline void fillField(void* field, const std::string& fieldString, int indexOffset = 0) {
		char c = fieldString[0];
		std::string valueStr = fieldString.substr(3);

		switch (c) {
		case static_cast<char>(FieldType::String): {
			std::string* strptr = static_cast<std::string*>(field);
			*strptr = valueStr;
			return;
		}
		case static_cast<char>(FieldType::Int): {
			int* intptr = static_cast<int*>(field);
			*intptr = std::stoi(valueStr);
			return;
		}
		case static_cast<char>(FieldType::Float): {
			float* floatptr = static_cast<float*>(field);
			*floatptr = std::stof(valueStr);
			return;
		}
		case static_cast<char>(FieldType::Double): {
			double* doubleptr = static_cast<double*>(field);
			*doubleptr = std::stod(valueStr);
			return;
		}
		case static_cast<char>(FieldType::Bool): {
			bool* boolptr = static_cast<bool*>(field);
			*boolptr = (std::stoi(valueStr) == 1);
			return;
		}
		case static_cast<char>(FieldType::Vector3): {
			std::vector<std::string> values = split(valueStr, '\n');
			midMath::Vector3* vptr = static_cast<midMath::Vector3*>(field);
			vptr->x = std::stof(values[0]);
			vptr->y = std::stof(values[1]);
			vptr->z = std::stof(values[2]);
			return;
		}
		case static_cast<char>(FieldType::Vector2): {
			std::vector<std::string> values = split(valueStr, '\n');
			midMath::Vector2* vptr = static_cast<midMath::Vector2*>(field);
			vptr->x = std::stof(values[0]);
			vptr->y = std::stof(values[1]);
			return;
		}
		case static_cast<char>(FieldType::Quaternion): {
			std::vector<std::string> values = split(valueStr, '\n');
			midMath::Quaternion* vptr = static_cast<midMath::Quaternion*>(field);
			vptr->x = std::stof(values[0]);
			vptr->y = std::stof(values[1]);
			vptr->z = std::stof(values[2]);
			vptr->w = std::stof(values[3]);
			return;
		}
		case static_cast<char>(FieldType::Color): {
			std::vector<std::string> values = split(valueStr, '\n');
			midPrimitive::Color* vptr = static_cast<midPrimitive::Color*>(field);
			vptr->r = std::stof(values[0]);
			vptr->g = std::stof(values[1]);
			vptr->b = std::stof(values[2]);
			vptr->a = std::stof(values[3]);
			return;
		}
		case static_cast<char>(FieldType::Id): {
			Id* id = static_cast<Id*>(field);
			// Offset by indexOffset. This is used when importing scenes into other scenes, offsetting imported scenes indexes to ghost area
			std::vector<std::string>idAndGen = split(valueStr, '_');
			id->index = std::stoi(idAndGen[0]); 
			if (id->index != UNASSIGNED) {
				id->index += indexOffset;
			}
			if (idAndGen.size() == 2) {
				id->generation = std::stoi(idAndGen[1]);
			}
			return;
		}
		case static_cast<char>(FieldType::IdVector): {
			std::vector<std::string> values = split(valueStr, '\n');
			std::vector<Id>* vectorptr = static_cast<std::vector<Id>*>(field);
			vectorptr->resize(values.size());
			for (int i = 0; i < values.size(); ++i) {
				// Offset by indexOffsetGlobal. This is used when importing scenes into other scenes, offsetting imported scenes indexes to ghost area
				fillField(&(*vectorptr)[i], values[i], indexOffset);
			}
			return;
		}
		}

		assert("not supported");
	}


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
