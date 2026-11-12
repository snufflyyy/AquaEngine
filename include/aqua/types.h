#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <cglm/cglm.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;
#define USIZE_MAX SIZE_MAX
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

typedef struct AquaEngineContext AquaEngineContext;
typedef struct AquaRenderer AquaRenderer;
typedef struct AquaWindow AquaWindow;
typedef struct AquaCamera AquaCamera;
typedef struct AquaFlyCamera AquaFlyCamera;

// Renderer-local slot indices. Zero is valid; destroyed slots can be reused.
// Discard handles before unloading, and never use them with another renderer.
typedef usize AquaShaderHandle;
typedef usize AquaTextureHandle;
typedef usize AquaMeshHandle;
typedef usize AquaMaterialHandle;
typedef usize AquaModelHandle;
#define AQUA_SHADER_HANDLE_INVALID USIZE_MAX
#define AQUA_TEXTURE_HANDLE_INVALID USIZE_MAX
#define AQUA_MESH_HANDLE_INVALID USIZE_MAX
#define AQUA_MATERIAL_HANDLE_INVALID USIZE_MAX
#define AQUA_MODEL_HANDLE_INVALID USIZE_MAX

typedef vec4 AquaColor;
typedef struct AquaVertex {
	vec3 position;
	vec3 normal;
	vec2 texture_coords;
} AquaVertex;
