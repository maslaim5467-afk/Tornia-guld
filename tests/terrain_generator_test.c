#include "../src/terrain_generator.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
int main(void){
 TgTerrainConfig c=tg_terrain_default_config(0x1234u);
 TgTerrain a={0},b={0}; TgTerrainHeightStats stats; TgVec3 normal; size_t i; int relief=0;
 c.width=32.0f;c.depth=24.0f;c.segments_x=32u;c.segments_z=24u;c.height_scale=12.0f;
 assert(tg_terrain_generate(&c,&a)==0);assert(tg_terrain_generate(&c,&b)==0);
 assert(a.vertex_count==33u*25u);assert(a.index_count==32u*24u*6u);
 assert(a.mesh.vertex_count==a.vertex_count&&a.mesh.index_count==a.index_count);
 assert(fabsf(a.mesh.bounds_min.x+16.0f)<1e-5f&&fabsf(a.mesh.bounds_max.x-16.0f)<1e-5f);
 assert(fabsf(a.mesh.bounds_min.z+12.0f)<1e-5f&&fabsf(a.mesh.bounds_max.z-12.0f)<1e-5f);
 for(i=0;i<a.vertex_count;++i){assert(memcmp(&a.vertices[i].position,&b.vertices[i].position,sizeof(TgVec3))==0);if(fabsf(a.vertices[i].position.y)>1e-4f)relief=1;}
 assert(relief);assert(tg_terrain_get_height_stats(&a,&stats)==0);assert(stats.maximum>stats.minimum&&isfinite(stats.mean));
 assert(tg_terrain_sample_normal(0,0,c.seed,12.0f,0.5f,&normal)==0);
 assert(normal.y>0.0f&&fabsf(sqrtf(normal.x*normal.x+normal.y*normal.y+normal.z*normal.z)-1.0f)<1e-4f);
 for(i=0;i<a.index_count;i+=3){TgVec3 p0=a.vertices[a.indices[i]].position,p1=a.vertices[a.indices[i+1]].position,p2=a.vertices[a.indices[i+2]].position;assert(tg_vec3_cross(tg_vec3_sub(p1,p0),tg_vec3_sub(p2,p0)).y>0.0f);}
 assert(tg_terrain_sample_normal(0,0,c.seed,12.0f,0.0f,&normal)!=0);
 c.segments_x=0;assert(tg_terrain_generate(&c,&b)!=0);
 tg_terrain_destroy(&a);tg_terrain_destroy(&b);assert(!a.vertices&&!a.indices);
 puts("Terrain generator tests passed");return 0;
}
