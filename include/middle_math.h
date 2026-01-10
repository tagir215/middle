#pragma once
#include "game_state.h"
#include <cassert>

using namespace descart;

namespace middle {

	static Vec DescVec(const Vector3& vec) {
		return { vec.x, vec.y, vec.z };
	}

	static Vector3 FromDescVec(const Vec& vec) {
		return{ vec.x, vec.y, vec.z };
	}

	static Matrix FromDescMat(Matr mat) {
		return {
				mat.m0, mat.m4, mat.m8, mat.m12,
				mat.m1, mat.m5, mat.m9, mat.m13,
				mat.m2, mat.m6, mat.m10, mat.m14,
				mat.m3, mat.m7, mat.m11, mat.m15
		};
	}

	static Matr DescMat(Matrix mat) {
		return {
				mat.m0, mat.m4, mat.m8, mat.m12,
				mat.m1, mat.m5, mat.m9, mat.m13,
				mat.m2, mat.m6, mat.m10, mat.m14,
				mat.m3, mat.m7, mat.m11, mat.m15
		};
	}


	static bool PointInsideSphere(const Vector3& point, const Vector3& spherePos, float radius) {
		float sqDist = Vector3DistanceSqr(point, spherePos);
		return sqDist < radius * radius;
	}

	static bool RayCastLineSphere(const Vector3& spherePos, float radius, const Vector3& rayStart, const Vector3& rayEnd, Vector3& outIntersectPos) {
		Vector3 toSphere = spherePos - rayStart;
		Vector3 rayDir = Vector3Normalize(rayEnd - rayStart);
		float projectionMag = Vector3DotProduct(rayDir, toSphere);
		Vector3 closestPointOnLine = rayStart + rayDir * projectionMag;

		float distSq = Vector3DistanceSqr(closestPointOnLine, spherePos);

		outIntersectPos = closestPointOnLine;
		// TODO
		return distSq < radius * radius;
	}

	static bool PointIntersectLineZX_Plane(const Vector3& pointPos, const Vector3& lineA, const Vector3& lineB, float paddingH, float paddingV) {
		Vector3 toPoint = pointPos - lineA;
		Vector3 lineDir = Vector3Normalize(lineB - lineA);
		float dot = Vector3DotProduct(toPoint, lineDir);
		float lineLength = Vector3Distance(lineA, lineB);
		bool isBetweenLinePoints = dot >= -paddingH && dot <= lineLength + paddingH;
		if (!isBetweenLinePoints)
			return false;

		// zx plane assumed
		Vector3 y = { 0,1,0 };
		Vector3 normal = Vector3CrossProduct(lineDir, y);
		float dotNormal = Vector3DotProduct(toPoint, normal);
		return dotNormal * dotNormal < paddingV * paddingV;
	}

	static Vector3 RayCastLinePlane(const Vector3& planePos, const Vector3& planeNormal, const Vector3& rayStart, const Vector3& rayDir) {
		Vector3 toPlane = Vector3Subtract(planePos, rayStart);

		Vector3 dir = Vector3Normalize(rayDir);

		float dot = Vector3DotProduct(dir, planeNormal);
		if (dot == 0)
			return rayStart;

		// raydir length is 1
		float ratio = 1.0f / -dot;

		float distToPlane = -Vector3DotProduct(Vector3Subtract(planePos, rayStart), planeNormal);
		float rayLengthUntilHit = distToPlane * ratio;

		return Vector3Add(rayStart, Vector3Scale(dir, rayLengthUntilHit));
	}

}
