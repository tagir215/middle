#pragma once
#include "entity.h"

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
