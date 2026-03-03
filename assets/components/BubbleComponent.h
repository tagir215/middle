#pragma once
#include "registrars.h"
#include "editor_file_utils.h"
#define MIDDLEBUBBLECOMPONENT(X) 


namespace components {
	struct BubbleComponent : public middle::Serializable{
		float centerX;
		float centerY;
		float centerZ;
		float axisX;
		float axisY;
		float axisZ;
		float length;
		float width;
		float distBetweenNodes;
		float endRadius;

		float aX;
		float aY;
		float aZ;
		float bX;
		float bY;
		float bZ;

		bool hidden = false;

		int searchIndex = 0;

		bool infiniteMass = false;

		int nodeCountTarget = 0;

		void serialize(std::ostream& ostream) override;
		void deserialize(const std::vector<std::string>& buffer, int indexOffset) override;
		void getFields(std::vector<middle::FieldInfo>& fields, int* size) override;
		template<typename V>
		void reflect(V& v) {
		#define X(f) v(#f, f);
			MIDDLEBUBBLECOMPONENT(X)
		#undef X
		}
	};
}

