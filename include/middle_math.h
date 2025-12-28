#pragma once
#include "game_state.h"
#include <cassert>
using namespace descart;
using namespace middle;

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

static void TranslateShape(ShapeInstance& instance, float x, float y, float z) {
    Matr translate = {
           1,0,0,x,
           0,1,0,y,
           0,0,1,z,
           0,0,0,1
    };
    instance.pData.transform = MatrMul(instance.pData.transform, translate);
}

static void TranslateInitShape(Shape& shape, float x, float y, float z) {
    shape.initTransform = MatrixIdentity();
    Matrix translate = {
           1,0,0,x,
           0,1,0,y,
           0,0,1,z,
           0,0,0,1
    };
    shape.initTransform = MatrixMultiply(shape.initTransform, translate);
}

static Matrix ScaleShape(Shape& shape, float x, float y, float z) {
    Matrix scale = {
        x,0,0,0,
        0,y,0,0,
        0,0,z,0,
        0,0,0,1,
    };

    return MatrixMultiply(shape.initTransform, scale);
}

static bool PointInsideShape(const Vector3& point, const ShapeInstance& instance) {
    if (instance.shape.type == ShapeType::SPHERE) {
        Vec pos = VecTransform({ 0,0,0 }, instance.pData.transform);
        float sqDist = Vector3DistanceSqr(FromDescVec(pos), point);
        return sqDist < instance.shape.radius * instance.shape.radius;
    }

    return false;
}

static bool RayCastLineSphere(const Vector3& spherePos, float radius, const Vector3& rayStart, const Vector3& rayEnd) {
    Vector3 toSphere = spherePos - rayStart;
    Vector3 rayDir = Vector3Normalize(rayEnd - rayStart);
    float projectionMag = Vector3DotProduct(rayDir, toSphere);
    Vector3 closestPointOnLine = rayStart + rayDir * projectionMag;

    float distSq = Vector3DistanceSqr(closestPointOnLine, spherePos);

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

    float distToPlane = - Vector3DotProduct(Vector3Subtract(planePos, rayStart), planeNormal);
    float rayLengthUntilHit = distToPlane * ratio;

    return Vector3Add(rayStart, Vector3Scale(dir, rayLengthUntilHit));
}


static ShapeInstance MakeShapeInstance(Shape shape) {
    ShapeInstance instance;
    PhysicsBody& pdat = instance.pData;
    pdat.linearAcc = DescVec(shape.linearAcceleration);
    pdat.linearVel = DescVec(shape.linearVelocity);
    pdat.position = DescVec(shape.position);
    pdat.linearDamping = shape.linearDamping;

    // TODO rotation
    pdat.position = DescVec(shape.position);

    // shapes
    if (shape.type == ShapeType::SPHERE) {
        // TODO sphere
        pdat.colliderType = ColliderType::CIRC;
    }

	pdat.radius = shape.radius;

    // masses
    if (!shape.physicalShape) {
        pdat.infiniteMass = true;
    }
    else {
		pdat.mass = shape.mass;
		pdat.invMass = 1.0f / shape.mass;
        pdat.momentOfInertia = shape.inertia;
        pdat.invMomentOfInertia = 1.0f / shape.inertia;
	}

	instance.shape = shape;
	instance.pData.transform = DescMat(shape.initTransform);
	instance.isShapeAlive = true;
    instance.shape.loopSize;
	if (instance.pData.infiniteMass) {
		instance.pData.invMass = 0;
		instance.pData.invMomentOfInertia = 0;
	}
    return instance;
}
