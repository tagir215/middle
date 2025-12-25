#include "descart_math.h"
#include <cmath>

namespace descart {

	Vec RelV(const Vec& b, const Vec& a) {
		return { b.x - a.x, b.y - a.y, b.z - a.z };
	}

	float DotV(const Vec& a, const Vec& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	Vec CrossV(const Vec& a, const Vec& b)
	{
		return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
	}

	float MagV(const Vec& v) {
		return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	float MagSqV(const Vec& v) {
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}

	float DistV(const Vec& v1, const Vec& v2)
	{
		return MagV(SubV(v1, v2));
	}

	Vec NormV(const Vec& v) {
		float invMag = 1.0f / MagV(v);
		return { v.x * invMag, v.y * invMag, v.z * invMag };
	}

	Vec AddV(const Vec& a, const Vec& b) {
		return { a.x + b.x, a.y + b.y, a.z + b.z };
	}

	Vec SubV(const Vec& a, const Vec& b) {
		return { a.x - b.x, a.y - b.y, a.z - b.z };
	}

	Vec ScaleV(const Vec& v, float scale) {
		return { v.x * scale, v.y * scale, v.z * scale };
	}

	Vec DivideV(const Vec& v, float d) {
		float m = 1.0f / d;
		return { v.x * m, v.y * m, v.z * m };
	}

	Vec NegateV(const Vec& v) {
		return { -v.x, -v.y, -v.z };
	}

	void AssertNanVec(const Vec& v) {
		assert(!std::isnan(v.x));
		assert(!std::isnan(v.y));
		assert(!std::isnan(v.z));
	}

	Vec VecTransform(const Vec& v, const Matr& mat)
	{
		Vec result = { 0, 0, 0 };

		float x = v.x;
		float y = v.y;
		float z = v.z;

		result.x = mat.m0 * x + mat.m4 * y + mat.m8 * z + mat.m12;
		result.y = mat.m1 * x + mat.m5 * y + mat.m9 * z + mat.m13;
		result.z = mat.m2 * x + mat.m6 * y + mat.m10 * z + mat.m14;

		return result;
	}

	Matr MatrMul(const Matr& left, const Matr& right)
	{
		Matr result = { 0, 0, 0 };

		result.m0 = left.m0 * right.m0 + left.m1 * right.m4 + left.m2 * right.m8 + left.m3 * right.m12;
		result.m1 = left.m0 * right.m1 + left.m1 * right.m5 + left.m2 * right.m9 + left.m3 * right.m13;
		result.m2 = left.m0 * right.m2 + left.m1 * right.m6 + left.m2 * right.m10 + left.m3 * right.m14;
		result.m3 = left.m0 * right.m3 + left.m1 * right.m7 + left.m2 * right.m11 + left.m3 * right.m15;
		result.m4 = left.m4 * right.m0 + left.m5 * right.m4 + left.m6 * right.m8 + left.m7 * right.m12;
		result.m5 = left.m4 * right.m1 + left.m5 * right.m5 + left.m6 * right.m9 + left.m7 * right.m13;
		result.m6 = left.m4 * right.m2 + left.m5 * right.m6 + left.m6 * right.m10 + left.m7 * right.m14;
		result.m7 = left.m4 * right.m3 + left.m5 * right.m7 + left.m6 * right.m11 + left.m7 * right.m15;
		result.m8 = left.m8 * right.m0 + left.m9 * right.m4 + left.m10 * right.m8 + left.m11 * right.m12;
		result.m9 = left.m8 * right.m1 + left.m9 * right.m5 + left.m10 * right.m9 + left.m11 * right.m13;
		result.m10 = left.m8 * right.m2 + left.m9 * right.m6 + left.m10 * right.m10 + left.m11 * right.m14;
		result.m11 = left.m8 * right.m3 + left.m9 * right.m7 + left.m10 * right.m11 + left.m11 * right.m15;
		result.m12 = left.m12 * right.m0 + left.m13 * right.m4 + left.m14 * right.m8 + left.m15 * right.m12;
		result.m13 = left.m12 * right.m1 + left.m13 * right.m5 + left.m14 * right.m9 + left.m15 * right.m13;
		result.m14 = left.m12 * right.m2 + left.m13 * right.m6 + left.m14 * right.m10 + left.m15 * right.m14;
		result.m15 = left.m12 * right.m3 + left.m13 * right.m7 + left.m14 * right.m11 + left.m15 * right.m15;

		return result;
	}

	Matr MatTranslate(Vec v)
	{
		Matr result = { 1.0f, 0.0f, 0.0f, v.x,
						  0.0f, 1.0f, 0.0f, v.y,
						  0.0f, 0.0f, 1.0f, v.z,
						  0.0f, 0.0f, 0.0f, 1.0f };

		return result;
	}

}
