#pragma once
#include "middle_math.h"
#include <string>
// copied from raylib
//
namespace midPrimitive{

	// Color, 4 components, R8G8B8A8 (32bit)
	typedef struct Color {
		unsigned char r;        // Color red value
		unsigned char g;        // Color green value
		unsigned char b;        // Color blue value
		unsigned char a;        // Color alpha value
	} Color;

	typedef struct Sound {
		std::string name;
	} Sound;

	// Camera, defines position/orientation in 3d space
	typedef struct Camera3D {
	    midMath::Vector3 position;       // Camera position
	    midMath::Vector3 target;         // Camera target it looks-at
	    midMath::Vector3 up;             // Camera up vector (rotation over its axis)
	    float fovy;             // Camera field-of-view aperture in Y (degrees) in perspective, used as near plane height in world units in orthographic
	    int projection;         // Camera projection: CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC
	} Camera3D;

	typedef Camera3D Camera;

	// Texture, tex data stored in GPU memory (VRAM)
	typedef struct Texture {
		unsigned int id;        // OpenGL texture id
		int width;              // Texture base width
		int height;             // Texture base height
		int mipmaps;            // Mipmap levels, 1 by default
		int format;             // Data format (PixelFormat type)
	} Texture;

	// Texture2D, same as Texture
	typedef Texture Texture2D;

	// Rectangle, 4 components
	typedef struct Rectangle {
		float x;                // Rectangle top-left corner position x
		float y;                // Rectangle top-left corner position y
		float width;            // Rectangle width
		float height;           // Rectangle height
	} Rectangle;

	// Image, pixel data stored in CPU memory (RAM)
	typedef struct Image {
		void* data;             // Image raw data
		int width;              // Image base width
		int height;             // Image base height
		int mipmaps;            // Mipmap levels, 1 by default
		int format;             // Data format (PixelFormat type)
	} Image;


	// GlyphInfo, font characters glyphs info
	typedef struct GlyphInfo {
		int value;              // Character value (Unicode)
		int offsetX;            // Character offset X when drawing
		int offsetY;            // Character offset Y when drawing
		int advanceX;           // Character advance position X
		Image image;            // Character image data
	} GlyphInfo;


	// Font, font texture and GlyphInfo array data
	typedef struct Font {
		int baseSize;           // Base size (default chars height)
		int glyphCount;         // Number of glyph characters
		int glyphPadding;       // Padding around the glyph characters
		Texture2D texture;      // Texture atlas containing the glyphs
		Rectangle* recs;        // Rectangles in texture for the glyphs
		GlyphInfo* glyphs;      // Glyphs info data
	} Font;

	// Transform, vertex transformation data
	typedef struct Transform {
		midMath::Vector3 translation;    // Translation
		midMath::Quaternion rotation;    // Rotation
		midMath::Vector3 scale;          // Scale
	} Transform;

	// Anim pose, an array of Transform[]
	typedef Transform* ModelAnimPose;

	// Mesh, vertex data and vao/vbo
	typedef struct Mesh {
		int vertexCount;        // Number of vertices stored in arrays
		int triangleCount;      // Number of triangles stored (indexed or not)

		// Vertex attributes data
		float* vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
		float* texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
		float* texcoords2;      // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
		float* normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
		float* tangents;        // Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
		unsigned char* colors;  // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
		unsigned short* indices; // Vertex indices (in case vertex data comes indexed)

		// Skin data for animation
		int boneCount;          // Number of bones (MAX: 256 bones)
		unsigned char* boneIndices; // Vertex bone indices, up to 4 bones influence by vertex (skinning) (shader-location = 6)
		float* boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning) (shader-location = 7)

		// Runtime animation vertex data (CPU skinning)
		// NOTE: In case of GPU skinning, not used, pointers are NULL
		float* animVertices;    // Animated vertex positions (after bones transformations)
		float* animNormals;     // Animated normals (after bones transformations)

		// OpenGL identifiers
		unsigned int vaoId;     // OpenGL Vertex Array Object id
		unsigned int* vboId;    // OpenGL Vertex Buffer Objects id (default vertex data)
	} Mesh;

	// Shader
	typedef struct Shader {
		unsigned int id;        // Shader program id
		int* locs;              // Shader locations array (RL_MAX_SHADER_LOCATIONS)
	} Shader;

	// MaterialMap
	typedef struct MaterialMap {
		Texture2D texture;      // Material map texture
		Color color;            // Material map color
		float value;            // Material map value
	} MaterialMap;


	// Material, includes shader and maps
	typedef struct Material {
		Shader shader;          // Material shader
		MaterialMap* maps;      // Material maps array (MAX_MATERIAL_MAPS)
		float params[4];        // Material generic parameters (if required)
	} Material;

	// Bone, skeletal animation bone
	typedef struct BoneInfo {
		char name[32];          // Bone name
		int parent;             // Bone parent
	} BoneInfo;


	// Skeleton, animation bones hierarchy
	typedef struct ModelSkeleton {
		unsigned int boneCount; // Number of bones
		BoneInfo* bones;        // Bones information (skeleton)
		ModelAnimPose bindPose; // Bones base transformation (Transform[])
	} ModelSkeleton;


	// Model, meshes, materials and animation data
	typedef struct Model {
		midMath::Matrix transform;       // Local transform matrix

		int meshCount;          // Number of meshes
		int materialCount;      // Number of materials
		Mesh* meshes;           // Meshes array
		Material* materials;    // Materials array
		int* meshMaterial;      // Mesh material number

		// Animation data
		ModelSkeleton skeleton; // Skeleton for animation

		// Runtime animation data (CPU/GPU skinning)
		ModelAnimPose currentPose; // Current animation pose (Transform[])
		midMath::Matrix* boneMatrices;   // Bones animated transformation matrices
	} Model;


}
