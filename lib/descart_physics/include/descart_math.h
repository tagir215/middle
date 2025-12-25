#pragma once
#include <cassert>
#include <cmath>

namespace descart {

	struct Vec {
		float x;
		float y;
		float z;
	};

	struct Matr {
		float m0, m4, m8, m12;
		float m1, m5, m9, m13;
		float m2, m6, m10, m14;
		float m3, m7, m11, m15;
	};

	void AssertNanVec(const Vec& v);

	Vec VecTransform(const Vec& v, const Matr& mat);

	Matr MatrMul(const Matr& left, const Matr& right);

	Matr MatTranslate(Vec v);

	Vec RelV(const Vec& b, const Vec& a);

	float DotV(const Vec& a, const Vec& b);

	float MagV(const Vec& v);

	float MagSqV(const Vec& v);

	float DistV(const Vec& v1, const Vec& v2);

	Vec NormV(const Vec& v);

	Vec AddV(const Vec& a, const Vec& b);

	Vec SubV(const Vec& a, const Vec& b);

	Vec ScaleV(const Vec& v, float scale);

	Vec NegateV(const Vec& v);

	Vec DivideV(const Vec& v, float d);

	Matr MulM(const Matr& ma, const Matr& mb);


}
