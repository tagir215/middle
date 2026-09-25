#pragma once
#include <raymath.h>
#include "middle_math.h"

inline static Vector3 toRVec(const midMath::Vector3& vec) {
	return { vec.x,vec.y,vec.z };
}
inline static Color toRColor(const midPrimitive::Color& col) {
	return { col.r, col.g, col.b, col.a };
}
inline static Quaternion toRQuat(const midMath::Quaternion& quat) {
	return { quat.x,quat.y,quat.z,quat.w };
}
inline static Transform toRTransform(const midPrimitive::Transform& tra) {
	return {
		toRVec(tra.translation),
		toRQuat(tra.rotation),
		toRVec(tra.scale)
	};
}
inline static Camera3D toRCam(midPrimitive::Camera& cam) {
	return Camera3D{
		toRVec(cam.position),
		toRVec(cam.target),
		toRVec(cam.up),
		cam.fovy,
		cam.projection
	};
}
inline static Texture2D toRTexture(midPrimitive::Texture2D& texture) {
	return Texture2D{
		texture.id,
		texture.width,
		texture.height,
		texture.mipmaps,
		texture.format
	};
}
inline static Shader toRShader(midPrimitive::Shader& shader) {
	return Shader{
		shader.id,
		shader.locs
	};
}
