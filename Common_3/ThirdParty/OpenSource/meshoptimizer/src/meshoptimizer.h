/**
 * meshoptimizer - version 0.11
 *
 * Copyright (C) 2016-2019, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
 * Report bugs and download new versions at https://github.com/zeux/meshoptimizer
 *
 * This library is distributed under the MIT License. See notice at the end of this file.
 */
#pragma once

#include "../../../../OS/Interfaces/ILog.h"
#include "../../../../OS/Interfaces/IMemory.h"

/* Version macro; major * 1000 + minor * 10 + patch */
#define MESHOPTIMIZER_VERSION 110

/* If no API is defined, assume default */
#ifndef MESHOPTIMIZER_API
#define MESHOPTIMIZER_API
#endif

/* Experimental APIs have unstable interface and might have implementation that's not fully tested or optimized */
#define MESHOPTIMIZER_EXPERIMENTAL MESHOPTIMIZER_API

/* C interface */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Vertex attribute stream, similar to glVertexPointer
 * Each element takes size bytes, with stride controlling the spacing between successive elements.
 */
struct meshopt_Stream
{
	const void* data;
	size_t size;
	size_t stride;
};

/**
 * Generates a vertex remap table from the vertex buffer and an optional index buffer and returns number of unique vertices
 * As a result, all vertices that are binary equivalent map to the same (new) location, with no gaps in the resulting sequence.
 * Resulting remap table maps old vertices to new vertices and can be used in meshopt_remapVertexBuffer/meshopt_remapIndexBuffer.
 *
 * destination must contain enough space for the resulting remap table (vertex_count elements)
 * indices can be NULL if the input is unindexed
 */
MESHOPTIMIZER_API size_t meshopt_generateVertexRemap(unsigned int* destination, const unsigned int* indices, size_t index_count, const void* vertices, size_t vertex_count, size_t vertex_size);

/**
 * Experimental: Generates a vertex remap table from multiple vertex streams and an optional index buffer and returns number of unique vertices
 * As a result, all vertices that are binary equivalent map to the same (new) location, with no gaps in the resulting sequence.
 * Resulting remap table maps old vertices to new vertices and can be used in meshopt_remapVertexBuffer/meshopt_remapIndexBuffer.
 * To remap vertex buffers, you will need to call meshopt_remapVertexBuffer for each vertex stream.
 *
 * destination must contain enough space for the resulting remap table (vertex_count elements)
 * indices can be NULL if the input is unindexed
 */
MESHOPTIMIZER_EXPERIMENTAL size_t meshopt_generateVertexRemapMulti(unsigned int* destination, const unsigned int* indices, size_t index_count, size_t vertex_count, const struct meshopt_Stream* streams, size_t stream_count);

/**
 * Generates vertex buffer from the source vertex buffer and remap table generated by meshopt_generateVertexRemap
 *
 * destination must contain enough space for the resulting vertex buffer (unique_vertex_count elements, returned by meshopt_generateVertexRemap)
 * vertex_count should be the initial vertex count and not the value returned by meshopt_generateVertexRemap
 */
MESHOPTIMIZER_API void meshopt_remapVertexBuffer(void* destination, const void* vertices, size_t vertex_count, size_t vertex_size, const unsigned int* remap);

/**
 * Generate index buffer from the source index buffer and remap table generated by meshopt_generateVertexRemap
 *
 * destination must contain enough space for the resulting index buffer (index_count elements)
 * indices can be NULL if the input is unindexed
 */
MESHOPTIMIZER_API void meshopt_remapIndexBuffer(unsigned int* destination, const unsigned int* indices, size_t index_count, const unsigned int* remap);

/**
 * Experimental: Generate index buffer that can be used for more efficient rendering when only a subset of the vertex attributes is necessary
 * All vertices that are binary equivalent (wrt first vertex_size bytes) map to the first vertex in the original vertex buffer.
 * This makes it possible to use the index buffer for Z pre-pass or shadowmap rendering, while using the original index buffer for regular rendering.
 *
 * destination must contain enough space for the resulting index buffer (index_count elements)
 */
MESHOPTIMIZER_EXPERIMENTAL void meshopt_generateShadowIndexBuffer(unsigned int* destination, const unsigned int* indices, size_t index_count, const void* vertices, size_t vertex_count, size_t vertex_size, size_t vertex_stride);

/**
 * Experimental: Generate index buffer that can be used for more efficient rendering when only a subset of the vertex attributes is necessary
 * All vertices that are binary equivalent (wrt specified streams) map to the first vertex in the original vertex buffer.
 * This makes it possible to use the index buffer for Z pre-pass or shadowmap rendering, while using the original index buffer for regular rendering.
 *
 * destination must contain enough space for the resulting index buffer (index_count elements)
 */
MESHOPTIMIZER_EXPERIMENTAL void meshopt_generateShadowIndexBufferMulti(unsigned int* destination, const unsigned int* indices, size_t index_count, size_t vertex_count, const struct meshopt_Stream* streams, size_t stream_count);

/**
 * Vertex transform cache optimizer
 * Reorders indices to reduce the number of GPU vertex shader invocations
 * If index buffer contains multiple ranges for multiple draw calls, this functions needs to be called on each range individually.
 *
 * destination must contain enough space for the resulting index buffer (index_count elements)
 */
MESHOPTIMIZER_API void meshopt_optimizeVertexCache(unsigned int* destination, const unsigned int* indices, size_t index_count, size_t vertex_count);

/**
 * Vertex transform cache optimizer for FIFO caches
 * Reorders indices to reduce the number of GPU vertex shader invocations
 * Generally takes ~3x less time to optimize meshes but produces inferior results compared to meshopt_optimizeVertexCache
 * If index buffer contains multiple ranges for multiple draw calls, this functions needs to be called on each range individually.
 *
 * destination must contain enough space for the resulting index buffer (index_count elements)
 * cache_size should be less than the actual GPU cache size to avoid cache thrashing
 */
MESHOPTIMIZER_API void meshopt_optimizeVertexCacheFifo(unsigned int* destination, const unsigned int* indices, size_t index_count, size_t vertex_count, unsigned int cache_size);

/**
 * Overdraw optimizer
 * Reorders indices to reduce the number of GPU vertex shader invocations and the pixel overdraw
 * If index buffer contains multiple ranges for multiple draw calls, this functions needs to be called on each range individually.
 *
 * destination must contain enough space for the resulting index buffer (index_count elements)
 * indices must contain index data that is the result of meshopt_optimizeVertexCache (*not* the original mesh indices!)
 * vertex_positions should have float3 position in the first 12 bytes of each vertex - similar to glVertexPointer
 * threshold indicates how much the overdraw optimizer can degrade vertex cache efficiency (1.05 = up to 5%) to reduce overdraw more efficiently
 */
MESHOPTIMIZER_API void meshopt_optimizeOverdraw(unsigned int* destination, const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, float threshold);

/**
 * Vertex fetch cache optimizer
 * Reorders vertices and changes indices to reduce the amount of GPU memory fetches during vertex processing
 * Returns the number of unique vertices, which is the same as input vertex count unless some vertices are unused
 * This functions works for a single vertex stream; for multiple vertex streams, use meshopt_optimizeVertexFetchRemap + meshopt_remapVertexBuffer for each stream.
 *
 * destination must contain enough space for the resulting vertex buffer (vertex_count elements)
 * indices is used both as an input and as an output index buffer
 */
MESHOPTIMIZER_API size_t meshopt_optimizeVertexFetch(void* destination, unsigned int* indices, size_t index_count, const void* vertices, size_t vertex_count, size_t vertex_size);

/**
 * Vertex fetch cache optimizer
 * Generates vertex remap to reduce the amount of GPU memory fetches during vertex processing
 * Returns the number of unique vertices, which is the same as input vertex count unless some vertices are unused
 * The resulting remap table should be used to reorder vertex/index buffers using meshopt_remapVertexBuffer/meshopt_remapIndexBuffer
 *
 * destination must contain enough space for the resulting remap table (vertex_count elements)
 */
MESHOPTIMIZER_API size_t meshopt_optimizeVertexFetchRemap(unsigned int* destination, const unsigned int* indices, size_t index_count, size_t vertex_count);

/**
 * Index buffer encoder
 * Encodes index data into an array of bytes that is generally much smaller (<1.5 bytes/triangle) and compresses better (<1 bytes/triangle) compared to original.
 * Returns encoded data size on success, 0 on error; the only error condition is if buffer doesn't have enough space
 * For maximum efficiency the index buffer being encoded has to be optimized for vertex cache and vertex fetch first.
 *
 * buffer must contain enough space for the encoded index buffer (use meshopt_encodeIndexBufferBound to compute worst case size)
 */
MESHOPTIMIZER_API size_t meshopt_encodeIndexBuffer(unsigned char* buffer, size_t buffer_size, const unsigned int* indices, size_t index_count);
MESHOPTIMIZER_API size_t meshopt_encodeIndexBufferBound(size_t index_count, size_t vertex_count);

/**
 * Index buffer decoder
 * Decodes index data from an array of bytes generated by meshopt_encodeIndexBuffer
 * Returns 0 if decoding was successful, and an error code otherwise
 * The decoder is safe to use for untrusted input, but it may produce garbage data (e.g. out of range indices).
 *
 * destination must contain enough space for the resulting index buffer (index_count elements)
 */
MESHOPTIMIZER_API int meshopt_decodeIndexBuffer(void* destination, size_t index_count, size_t index_size, const unsigned char* buffer, size_t buffer_size);

/**
 * Vertex buffer encoder
 * Encodes vertex data into an array of bytes that is generally smaller and compresses better compared to original.
 * Returns encoded data size on success, 0 on error; the only error condition is if buffer doesn't have enough space
 * This function works for a single vertex stream; for multiple vertex streams, call meshopt_encodeVertexBuffer for each stream.
 *
 * buffer must contain enough space for the encoded vertex buffer (use meshopt_encodeVertexBufferBound to compute worst case size)
 */
MESHOPTIMIZER_API size_t meshopt_encodeVertexBuffer(unsigned char* buffer, size_t buffer_size, const void* vertices, size_t vertex_count, size_t vertex_size);
MESHOPTIMIZER_API size_t meshopt_encodeVertexBufferBound(size_t vertex_count, size_t vertex_size);

/**
 * Vertex buffer decoder
 * Decodes vertex data from an array of bytes generated by meshopt_encodeVertexBuffer
 * Returns 0 if decoding was successful, and an error code otherwise
 * The decoder is safe to use for untrusted input, but it may produce garbage data.
 *
 * destination must contain enough space for the resulting vertex buffer (vertex_count * vertex_size bytes)
 */
MESHOPTIMIZER_API int meshopt_decodeVertexBuffer(void* destination, size_t vertex_count, size_t vertex_size, const unsigned char* buffer, size_t buffer_size);

/**
 * Experimental: Mesh simplifier
 * Reduces the number of triangles in the mesh, attempting to preserve mesh appearance as much as possible
 * The algorithm tries to preserve mesh topology and can stop short of the target goal based on topology constraints or target error.
 * If not all attributes from the input mesh are required, it's recommended to reindex the mesh using meshopt_generateShadowIndexBuffer prior to simplification.
 * Returns the number of indices after simplification, with destination containing new index data
 * The resulting index buffer references vertices from the original vertex buffer.
 * If the original vertex data isn't required, creating a compact vertex buffer using meshopt_optimizeVertexFetch is recommended.
 *
 * destination must contain enough space for the *source* index buffer (since optimization is iterative, this means index_count elements - *not* target_index_count!)
 * vertex_positions should have float3 position in the first 12 bytes of each vertex - similar to glVertexPointer
 */
MESHOPTIMIZER_EXPERIMENTAL size_t meshopt_simplify(unsigned int* destination, const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, size_t target_index_count, float target_error);

/**
 * Experimental: Mesh simplifier (sloppy)
 * Reduces the number of triangles in the mesh, sacrificing mesh apperance for simplification performance
 * The algorithm doesn't preserve mesh topology but is always able to reach target triangle count.
 * Returns the number of indices after simplification, with destination containing new index data
 * The resulting index buffer references vertices from the original vertex buffer.
 * If the original vertex data isn't required, creating a compact vertex buffer using meshopt_optimizeVertexFetch is recommended.
 *
 * destination must contain enough space for the target index buffer
 * vertex_positions should have float3 position in the first 12 bytes of each vertex - similar to glVertexPointer
 */
MESHOPTIMIZER_EXPERIMENTAL size_t meshopt_simplifySloppy(unsigned int* destination, const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, size_t target_index_count);

/**
 * Mesh stripifier
 * Converts a previously vertex cache optimized triangle list to triangle strip, stitching strips using restart index or degenerate triangles
 * Returns the number of indices in the resulting strip, with destination containing new index data
 * For maximum efficiency the index buffer being converted has to be optimized for vertex cache first.
 * Using restart indices can result in ~10% smaller index buffers, but on some GPUs restart indices may result in decreased performance.
 *
 * destination must contain enough space for the target index buffer, worst case can be computed with meshopt_stripifyBound
 * restart_index should be 0xffff or 0xffffffff depending on index size, or 0 to use degenerate triangles
 */
MESHOPTIMIZER_API size_t meshopt_stripify(unsigned int* destination, const unsigned int* indices, size_t index_count, size_t vertex_count, unsigned int restart_index);
MESHOPTIMIZER_API size_t meshopt_stripifyBound(size_t index_count);

/**
 * Mesh unstripifier
 * Converts a triangle strip to a triangle list
 * Returns the number of indices in the resulting list, with destination containing new index data
 *
 * destination must contain enough space for the target index buffer, worst case can be computed with meshopt_unstripifyBound
 */
MESHOPTIMIZER_API size_t meshopt_unstripify(unsigned int* destination, const unsigned int* indices, size_t index_count, unsigned int restart_index);
MESHOPTIMIZER_API size_t meshopt_unstripifyBound(size_t index_count);

struct meshopt_VertexCacheStatistics
{
	unsigned int vertices_transformed;
	unsigned int warps_executed;
	float acmr; /* transformed vertices / triangle count; best case 0.5, worst case 3.0, optimum depends on topology */
	float atvr; /* transformed vertices / vertex count; best case 1.0, worst case 6.0, optimum is 1.0 (each vertex is transformed once) */
};

/**
 * Vertex transform cache analyzer
 * Returns cache hit statistics using a simplified FIFO model
 * Results may not match actual GPU performance
 */
MESHOPTIMIZER_API struct meshopt_VertexCacheStatistics meshopt_analyzeVertexCache(const unsigned int* indices, size_t index_count, size_t vertex_count, unsigned int cache_size, unsigned int warp_size, unsigned int primgroup_size);

struct meshopt_OverdrawStatistics
{
	unsigned int pixels_covered;
	unsigned int pixels_shaded;
	float overdraw; /* shaded pixels / covered pixels; best case 1.0 */
};

/**
 * Overdraw analyzer
 * Returns overdraw statistics using a software rasterizer
 * Results may not match actual GPU performance
 *
 * vertex_positions should have float3 position in the first 12 bytes of each vertex - similar to glVertexPointer
 */
MESHOPTIMIZER_API struct meshopt_OverdrawStatistics meshopt_analyzeOverdraw(const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride);

struct meshopt_VertexFetchStatistics
{
	unsigned int bytes_fetched;
	float overfetch; /* fetched bytes / vertex buffer size; best case 1.0 (each byte is fetched once) */
};

/**
 * Vertex fetch cache analyzer
 * Returns cache hit statistics using a simplified direct mapped model
 * Results may not match actual GPU performance
 */
MESHOPTIMIZER_API struct meshopt_VertexFetchStatistics meshopt_analyzeVertexFetch(const unsigned int* indices, size_t index_count, size_t vertex_count, size_t vertex_size);

struct meshopt_Meshlet
{
	unsigned int vertices[64];
	unsigned char indices[126][3];
	unsigned char triangle_count;
	unsigned char vertex_count;
};

/**
 * Experimental: Meshlet builder
 * Splits the mesh into a set of meshlets where each meshlet has a micro index buffer indexing into meshlet vertices that refer to the original vertex buffer
 * The resulting data can be used to render meshes using NVidia programmable mesh shading pipeline, or in other cluster-based renderers.
 * For maximum efficiency the index buffer being converted has to be optimized for vertex cache first.
 *
 * destination must contain enough space for all meshlets, worst case size can be computed with meshopt_buildMeshletsBound
 * max_vertices and max_triangles can't exceed limits statically declared in meshopt_Meshlet (max_vertices <= 64, max_triangles <= 126)
 */
MESHOPTIMIZER_EXPERIMENTAL size_t meshopt_buildMeshlets(struct meshopt_Meshlet* destination, const unsigned int* indices, size_t index_count, size_t vertex_count, size_t max_vertices, size_t max_triangles);
MESHOPTIMIZER_EXPERIMENTAL size_t meshopt_buildMeshletsBound(size_t index_count, size_t max_vertices, size_t max_triangles);

struct meshopt_Bounds
{
	/* bounding sphere, useful for frustum and occlusion culling */
	float center[3];
	float radius;

	/* normal cone, useful for backface culling */
	float cone_apex[3];
	float cone_axis[3];
	float cone_cutoff; /* = cos(angle/2) */

	/* normal cone axis and cutoff, stored in 8-bit SNORM format; decode using x/127.0 */
	signed char cone_axis_s8[3];
	signed char cone_cutoff_s8;
};

/**
 * Experimental: Cluster bounds generator
 * Creates bounding volumes that can be used for frustum, backface and occlusion culling.
 *
 * For backface culling with orthographic projection, use the following formula to reject backfacing clusters:
 *   dot(view, cone_axis) >= cone_cutoff
 *
 * For perspective projection, you can the formula that needs cone apex in addition to axis & cutoff:
 *   dot(normalize(cone_apex - camera_position), cone_axis) >= cone_cutoff
 *
 * Alternatively, you can use the formula that doesn't need cone apex and uses bounding sphere instead:
 *   dot(normalize(center - camera_position), cone_axis) >= cone_cutoff + radius / length(center - camera_position)
 * or an equivalent formula that doesn't have a singularity at center = camera_position:
 *   dot(center - camera_position, cone_axis) >= cone_cutoff * length(center - camera_position) + radius
 *
 * The formula that uses the apex is slightly more accurate but needs the apex; if you are already using bounding sphere
 * to do frustum/occlusion culling, the formula that doesn't use the apex may be preferable.
 *
 * vertex_positions should have float3 position in the first 12 bytes of each vertex - similar to glVertexPointer
 * index_count should be less than or equal to 256*3 (the function assumes clusters of limited size)
 */
MESHOPTIMIZER_EXPERIMENTAL struct meshopt_Bounds meshopt_computeClusterBounds(const unsigned int* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride);
MESHOPTIMIZER_EXPERIMENTAL struct meshopt_Bounds meshopt_computeMeshletBounds(const struct meshopt_Meshlet* meshlet, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride);

/**
 * Experimental: Set allocation callbacks
 * These callbacks will be used instead of the default operator new/operator delete for all temporary allocations in the library.
 * Note that all algorithms only allocate memory for temporary use.
 * allocate/deallocate are always called in a stack-like order - last pointer to be allocated is deallocated first.
 */
MESHOPTIMIZER_EXPERIMENTAL void meshopt_setAllocator(void* (*allocate)(size_t), void (*deallocate)(void*));

#ifdef __cplusplus
} /* extern "C" */
#endif

/* Quantization into commonly supported data formats */
#ifdef __cplusplus
/**
 * Quantize a float in [0..1] range into an N-bit fixed point unorm value
 * Assumes reconstruction function (q / (2^N-1)), which is the case for fixed-function normalized fixed point conversion
 * Maximum reconstruction error: 1/2^(N+1)
 */
inline int meshopt_quantizeUnorm(float v, int N);

/**
 * Quantize a float in [-1..1] range into an N-bit fixed point snorm value
 * Assumes reconstruction function (q / (2^(N-1)-1)), which is the case for fixed-function normalized fixed point conversion (except early OpenGL versions)
 * Maximum reconstruction error: 1/2^N
 */
inline int meshopt_quantizeSnorm(float v, int N);

/**
 * Quantize a float into half-precision floating point value
 * Generates +-inf for overflow, preserves NaN, flushes denormals to zero, rounds to nearest
 * Representable magnitude range: [6e-5; 65504]
 * Maximum relative reconstruction error: 5e-4
 */
inline unsigned short meshopt_quantizeHalf(float v);

/**
 * Quantize a float into a floating point value with a limited number of significant mantissa bits
 * Generates +-inf for overflow, preserves NaN, flushes denormals to zero, rounds to nearest
 * Assumes N is in a valid mantissa precision range, which is 1..23
 */
inline float meshopt_quantizeFloat(float v, int N);
#endif

/**
 * C++ template interface
 *
 * These functions mirror the C interface the library provides, providing template-based overloads so that
 * the caller can use an arbitrary type for the index data, both for input and output.
 * When the supplied type is the same size as that of unsigned int, the wrappers are zero-cost; when it's not,
 * the wrappers end up allocating memory and copying index data to convert from one type to another.
 */
#ifdef __cplusplus
template <typename T, bool ZeroCopy = sizeof(T) == sizeof(unsigned int)>
struct meshopt_IndexAdapter;

template <typename T>
struct meshopt_IndexAdapter<T, false>
{
	T* result;
	unsigned int* data;
	size_t count;

	meshopt_IndexAdapter(T* result_, const T* input, size_t count_)
	    : result(result_)
	    , data(0)
	    , count(count_)
	{
		data = (unsigned int*)conf_malloc(sizeof(unsigned int) * count);

		if (input)
		{
			for (size_t i = 0; i < count; ++i)
				data[i] = input[i];
		}
	}

	~meshopt_IndexAdapter()
	{
		if (result)
		{
			for (size_t i = 0; i < count; ++i)
				result[i] = T(data[i]);
		}
		conf_free(data);
	}
};

template <typename T>
struct meshopt_IndexAdapter<T, true>
{
	unsigned int* data;

	meshopt_IndexAdapter(T* result, const T* input, size_t)
	    : data(reinterpret_cast<unsigned int*>(result ? result : const_cast<T*>(input)))
	{
	}
};

template <typename T>
inline size_t meshopt_generateVertexRemap(unsigned int* destination, const T* indices, size_t index_count, const void* vertices, size_t vertex_count, size_t vertex_size)
{
	meshopt_IndexAdapter<T> in(0, indices, indices ? index_count : 0);

	return meshopt_generateVertexRemap(destination, indices ? in.data : 0, index_count, vertices, vertex_count, vertex_size);
}

template <typename T>
inline size_t meshopt_generateVertexRemapMulti(unsigned int* destination, const T* indices, size_t index_count, size_t vertex_count, const meshopt_Stream* streams, size_t stream_count)
{
	meshopt_IndexAdapter<T> in(0, indices, indices ? index_count : 0);

	return meshopt_generateVertexRemapMulti(destination, indices ? in.data : 0, index_count, vertex_count, streams, stream_count);
}

template <typename T>
inline void meshopt_remapIndexBuffer(T* destination, const T* indices, size_t index_count, const unsigned int* remap)
{
	meshopt_IndexAdapter<T> in(0, indices, indices ? index_count : 0);
	meshopt_IndexAdapter<T> out(destination, 0, index_count);

	meshopt_remapIndexBuffer(out.data, indices ? in.data : 0, index_count, remap);
}

template <typename T>
void meshopt_generateShadowIndexBuffer(T* destination, const T* indices, size_t index_count, const void* vertices, size_t vertex_count, size_t vertex_size, size_t vertex_stride)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);
	meshopt_IndexAdapter<T> out(destination, 0, index_count);

	meshopt_generateShadowIndexBuffer(out.data, in.data, index_count, vertices, vertex_count, vertex_size, vertex_stride);
}

template <typename T>
void meshopt_generateShadowIndexBufferMulti(T* destination, const T* indices, size_t index_count, size_t vertex_count, const meshopt_Stream* streams, size_t stream_count)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);
	meshopt_IndexAdapter<T> out(destination, 0, index_count);

	meshopt_generateShadowIndexBufferMulti(out.data, in.data, index_count, vertex_count, streams, stream_count);
}

template <typename T>
inline void meshopt_optimizeVertexCache(T* destination, const T* indices, size_t index_count, size_t vertex_count)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);
	meshopt_IndexAdapter<T> out(destination, 0, index_count);

	meshopt_optimizeVertexCache(out.data, in.data, index_count, vertex_count);
}

template <typename T>
inline void meshopt_optimizeVertexCacheFifo(T* destination, const T* indices, size_t index_count, size_t vertex_count, unsigned int cache_size)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);
	meshopt_IndexAdapter<T> out(destination, 0, index_count);

	meshopt_optimizeVertexCacheFifo(out.data, in.data, index_count, vertex_count, cache_size);
}

template <typename T>
inline void meshopt_optimizeOverdraw(T* destination, const T* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, float threshold)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);
	meshopt_IndexAdapter<T> out(destination, 0, index_count);

	meshopt_optimizeOverdraw(out.data, in.data, index_count, vertex_positions, vertex_count, vertex_positions_stride, threshold);
}

template <typename T>
inline size_t meshopt_optimizeVertexFetchRemap(unsigned int* destination, const T* indices, size_t index_count, size_t vertex_count)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);

	return meshopt_optimizeVertexFetchRemap(destination, in.data, index_count, vertex_count);
}

template <typename T>
inline size_t meshopt_optimizeVertexFetch(void* destination, T* indices, size_t index_count, const void* vertices, size_t vertex_count, size_t vertex_size)
{
	meshopt_IndexAdapter<T> inout(indices, indices, index_count);

	return meshopt_optimizeVertexFetch(destination, inout.data, index_count, vertices, vertex_count, vertex_size);
}

template <typename T>
inline size_t meshopt_encodeIndexBuffer(unsigned char* buffer, size_t buffer_size, const T* indices, size_t index_count)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);

	return meshopt_encodeIndexBuffer(buffer, buffer_size, in.data, index_count);
}

template <typename T>
inline int meshopt_decodeIndexBuffer(T* destination, size_t index_count, const unsigned char* buffer, size_t buffer_size)
{
	char index_size_valid[sizeof(T) == 2 || sizeof(T) == 4 ? 1 : -1];
	(void)index_size_valid;

	return meshopt_decodeIndexBuffer(destination, index_count, sizeof(T), buffer, buffer_size);
}

template <typename T>
inline size_t meshopt_simplify(T* destination, const T* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, size_t target_index_count, float target_error)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);
	meshopt_IndexAdapter<T> out(destination, 0, index_count);

	return meshopt_simplify(out.data, in.data, index_count, vertex_positions, vertex_count, vertex_positions_stride, target_index_count, target_error);
}

template <typename T>
inline size_t meshopt_simplifySloppy(T* destination, const T* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride, size_t target_index_count)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);
	meshopt_IndexAdapter<T> out(destination, 0, target_index_count);

	return meshopt_simplifySloppy(out.data, in.data, index_count, vertex_positions, vertex_count, vertex_positions_stride, target_index_count);
}

template <typename T>
inline size_t meshopt_stripify(T* destination, const T* indices, size_t index_count, size_t vertex_count, T restart_index)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);
	meshopt_IndexAdapter<T> out(destination, 0, (index_count / 3) * 5);

	return meshopt_stripify(out.data, in.data, index_count, vertex_count, unsigned(restart_index));
}

template <typename T>
inline size_t meshopt_unstripify(T* destination, const T* indices, size_t index_count, T restart_index)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);
	meshopt_IndexAdapter<T> out(destination, 0, (index_count - 2) * 3);

	return meshopt_unstripify(out.data, in.data, index_count, unsigned(restart_index));
}

template <typename T>
inline meshopt_VertexCacheStatistics meshopt_analyzeVertexCache(const T* indices, size_t index_count, size_t vertex_count, unsigned int cache_size, unsigned int warp_size, unsigned int buffer_size)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);

	return meshopt_analyzeVertexCache(in.data, index_count, vertex_count, cache_size, warp_size, buffer_size);
}

template <typename T>
inline meshopt_OverdrawStatistics meshopt_analyzeOverdraw(const T* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);

	return meshopt_analyzeOverdraw(in.data, index_count, vertex_positions, vertex_count, vertex_positions_stride);
}

template <typename T>
inline meshopt_VertexFetchStatistics meshopt_analyzeVertexFetch(const T* indices, size_t index_count, size_t vertex_count, size_t vertex_size)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);

	return meshopt_analyzeVertexFetch(in.data, index_count, vertex_count, vertex_size);
}

template <typename T>
inline size_t meshopt_buildMeshlets(meshopt_Meshlet* destination, const T* indices, size_t index_count, size_t vertex_count, size_t max_vertices, size_t max_triangles)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);

	return meshopt_buildMeshlets(destination, in.data, index_count, vertex_count, max_vertices, max_triangles);
}

template <typename T>
inline meshopt_Bounds meshopt_computeClusterBounds(const T* indices, size_t index_count, const float* vertex_positions, size_t vertex_count, size_t vertex_positions_stride)
{
	meshopt_IndexAdapter<T> in(0, indices, index_count);

	return meshopt_computeClusterBounds(in.data, index_count, vertex_positions, vertex_count, vertex_positions_stride);
}
#endif

/* Inline implementation */
#ifdef __cplusplus
inline int meshopt_quantizeUnorm(float v, int N)
{
	const float scale = float((1 << N) - 1);

	v = (v >= 0) ? v : 0;
	v = (v <= 1) ? v : 1;

	return int(v * scale + 0.5f);
}

inline int meshopt_quantizeSnorm(float v, int N)
{
	const float scale = float((1 << (N - 1)) - 1);

	float round = (v >= 0 ? 0.5f : -0.5f);

	v = (v >= -1) ? v : -1;
	v = (v <= +1) ? v : +1;

	return int(v * scale + round);
}

inline unsigned short meshopt_quantizeHalf(float v)
{
	union { float f; unsigned int ui; } u = {v};
	unsigned int ui = u.ui;

	int s = (ui >> 16) & 0x8000;
	int em = ui & 0x7fffffff;

	/* bias exponent and round to nearest; 112 is relative exponent bias (127-15) */
	int h = (em - (112 << 23) + (1 << 12)) >> 13;

	/* underflow: flush to zero; 113 encodes exponent -14 */
	h = (em < (113 << 23)) ? 0 : h;

	/* overflow: infinity; 143 encodes exponent 16 */
	h = (em >= (143 << 23)) ? 0x7c00 : h;

	/* NaN; note that we convert all types of NaN to qNaN */
	h = (em > (255 << 23)) ? 0x7e00 : h;

	return (unsigned short)(s | h);
}

inline float meshopt_quantizeFloat(float v, int N)
{
	union { float f; unsigned int ui; } u = {v};
	unsigned int ui = u.ui;

	const int mask = (1 << (23 - N)) - 1;
	const int round = (1 << (23 - N)) >> 1;

	int e = ui & 0x7f800000;
	unsigned int rui = (ui + round) & ~mask;

	/* round all numbers except inf/nan; this is important to make sure nan doesn't overflow into -0 */
	ui = e == 0x7f800000 ? ui : rui;

	/* flush denormals to zero */
	ui = e == 0 ? 0 : ui;

	u.ui = ui;
	return u.f;
}
#endif

/* Internal implementation helpers */
#ifdef __cplusplus
class meshopt_Allocator
{
public:
	meshopt_Allocator()
	    : blocks()
	    , count(0)
	{
	}

	~meshopt_Allocator()
	{
		for (size_t i = count; i > 0; --i)
			conf_free(blocks[i - 1]);
	}

	void* allocate(size_t size)
	{
		ASSERT(count < sizeof(blocks) / sizeof(blocks[0]));
		void* result = conf_malloc(size);
		blocks[count++] = result;
		return result;
	}

private:
	void* blocks[16];
	size_t count;
};


#endif

/**
 * Copyright (c) 2016-2019 Arseny Kapoulkine
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
