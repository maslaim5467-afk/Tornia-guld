#ifndef TORNIA_TERRAIN_GENERATOR_H
#define TORNIA_TERRAIN_GENERATOR_H
#include "mesh3d.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { float width, depth; uint32_t segments_x, segments_z, seed; float height_scale; } TgTerrainConfig;
/* Owns CPU arrays; mesh borrows vertices and indices from this object. */
typedef struct { TgMeshVertex3D *vertices; uint32_t *indices; TgMesh3D mesh; uint32_t vertex_count, index_count; } TgTerrain;
typedef struct { float minimum, maximum, mean; } TgTerrainHeightStats;
/* Deterministic five-octave rolling heightfield. This is phenomenological noise,
 * not a physical erosion simulation. Mesh layout stays position-only to match
 * the existing Vulkan vertex input. */
int tg_terrain_generate(const TgTerrainConfig *config, TgTerrain *out_terrain);
void tg_terrain_destroy(TgTerrain *terrain);
float tg_terrain_sample_height(float x, float z, uint32_t seed, float height_scale);
int tg_terrain_sample_normal(float x, float z, uint32_t seed, float height_scale, float sample_spacing, TgVec3 *out_normal);
int tg_terrain_get_height_stats(const TgTerrain *terrain, TgTerrainHeightStats *out_stats);
TgTerrainConfig tg_terrain_default_config(uint32_t seed);
#ifdef __cplusplus
}
#endif
#endif
