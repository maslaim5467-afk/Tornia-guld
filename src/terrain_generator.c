#include "terrain_generator.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define OCTAVES 5u
#define BASE_FREQUENCY 0.0032f
#define LACUNARITY 2.0f
#define GAIN 0.5f
#define RIDGE_WEIGHT 0.22f
#define EPSILON 1.0e-6f
static int positive(float v){return isfinite(v)&&v>0.0f;}
static uint32_t hash_grid(int32_t x,int32_t z,uint32_t seed){
 uint32_t h=seed^0x9e3779b9u; h^=(uint32_t)x*0x85ebca6bu;
 h=(h^(h>>16))*0x7feb352du; h^=(uint32_t)z*0xc2b2ae35u;
 h=(h^(h>>15))*0x846ca68bu; return h^(h>>16);
}
static float lattice(int32_t x,int32_t z,uint32_t seed){
 return (float)(hash_grid(x,z,seed)>>8)*(1.0f/16777215.0f)*2.0f-1.0f;
}
static float fade5(float t){return t*t*t*(t*(t*6.0f-15.0f)+10.0f);}
static float mixf(float a,float b,float t){return a+(b-a)*t;}
static float noise(float x,float z,uint32_t seed){
 float fx=floorf(x),fz=floorf(z),tx=fade5(x-fx),tz=fade5(z-fz);
 int32_t ix=(int32_t)fx,iz=(int32_t)fz;
 float a=mixf(lattice(ix,iz,seed),lattice(ix+1,iz,seed),tx);
 float b=mixf(lattice(ix,iz+1,seed),lattice(ix+1,iz+1,seed),tx);
 return mixf(a,b,tz);
}
static float height_raw(float x,float z,uint32_t seed,float scale){
 float frequency=BASE_FREQUENCY,amplitude=1.0f,total=0.0f,weights=0.0f,ridges=0.0f;
 uint32_t i;
 for(i=0;i<OCTAVES;++i){
  float n=noise(x*frequency,z*frequency,seed+i*0x9e3779b9u);
  float ridge=1.0f-fabsf(n);
  total+=n*amplitude; weights+=amplitude;
  ridges+=(ridge*ridge*2.0f-0.65f)*amplitude;
  frequency*=LACUNARITY; amplitude*=GAIN;
 }
 if(weights<=EPSILON)return 0.0f;
 return (total/weights*0.82f+ridges/weights*RIDGE_WEIGHT)*scale;
}
float tg_terrain_sample_height(float x,float z,uint32_t seed,float scale){
 if(!isfinite(x)||!isfinite(z)||!isfinite(scale)||scale<0.0f)return 0.0f;
 return height_raw(x,z,seed,scale);
}
int tg_terrain_sample_normal(float x,float z,uint32_t seed,float scale,float spacing,TgVec3 *out){
 float dx,dz; TgVec3 n;
 if(!out||!isfinite(x)||!isfinite(z)||!isfinite(scale)||scale<0.0f||!positive(spacing))return -1;
 dx=(tg_terrain_sample_height(x+spacing,z,seed,scale)-tg_terrain_sample_height(x-spacing,z,seed,scale))/(2.0f*spacing);
 dz=(tg_terrain_sample_height(x,z+spacing,seed,scale)-tg_terrain_sample_height(x,z-spacing,seed,scale))/(2.0f*spacing);
 n=tg_vec3_normalize(tg_vec3(-dx,1.0f,-dz));
 if(!isfinite(n.x)||!isfinite(n.y)||!isfinite(n.z))return -1;
 *out=n; return 0;
}
TgTerrainConfig tg_terrain_default_config(uint32_t seed){
 TgTerrainConfig c={512.0f,512.0f,512u,512u,seed,24.0f}; return c;
}
static int counts(const TgTerrainConfig *c,size_t *vertices,size_t *indices){
 size_t cols,rows,cells;
 if(!c||!positive(c->width)||!positive(c->depth)||!c->segments_x||!c->segments_z||!isfinite(c->height_scale)||c->height_scale<0.0f)return -1;
 cols=(size_t)c->segments_x+1u; rows=(size_t)c->segments_z+1u;
 if(cols>SIZE_MAX/rows)return -1;
 *vertices=cols*rows;
 if((size_t)c->segments_x>SIZE_MAX/(size_t)c->segments_z)return -1;
 cells=(size_t)c->segments_x*(size_t)c->segments_z;
 if(cells>SIZE_MAX/6u)return -1;
 *indices=cells*6u;
 if(*vertices>UINT32_MAX||*indices>UINT32_MAX||*vertices>SIZE_MAX/sizeof(TgMeshVertex3D)||*indices>SIZE_MAX/sizeof(uint32_t))return -1;
 return 0;
}
int tg_terrain_generate(const TgTerrainConfig *c,TgTerrain *out){
 TgTerrain t; size_t vc,ic,row,col,cursor=0,cols; float hw,hd;
 if(!out||counts(c,&vc,&ic)!=0)return -1;
 memset(&t,0,sizeof(t)); cols=(size_t)c->segments_x+1u; hw=c->width*0.5f; hd=c->depth*0.5f;
 t.vertices=(TgMeshVertex3D*)malloc(vc*sizeof(*t.vertices));
 t.indices=(uint32_t*)malloc(ic*sizeof(*t.indices));
 if(!t.vertices||!t.indices){free(t.vertices);free(t.indices);return -1;}
 for(row=0;row<=(size_t)c->segments_z;++row){
  float z=-hd+(float)row/(float)c->segments_z*c->depth;
  for(col=0;col<=(size_t)c->segments_x;++col){
   float x=-hw+(float)col/(float)c->segments_x*c->width;
   t.vertices[row*cols+col].position=tg_vec3(x,height_raw(x,z,c->seed,c->height_scale),z);
  }
 }
 for(row=0;row<(size_t)c->segments_z;++row)for(col=0;col<(size_t)c->segments_x;++col){
  uint32_t v00=(uint32_t)(row*cols+col),v10=v00+1u;
  uint32_t v01=(uint32_t)((row+1u)*cols+col),v11=v01+1u;
  t.indices[cursor++]=v00;t.indices[cursor++]=v01;t.indices[cursor++]=v10;
  t.indices[cursor++]=v10;t.indices[cursor++]=v01;t.indices[cursor++]=v11;
 }
 t.vertex_count=(uint32_t)vc;t.index_count=(uint32_t)ic;
 if(tg_mesh3d_init(&t.mesh,t.vertices,vc,t.indices,ic)!=0){free(t.vertices);free(t.indices);return -1;}
 *out=t; return 0;
}
void tg_terrain_destroy(TgTerrain *t){if(!t)return;free(t->vertices);free(t->indices);memset(t,0,sizeof(*t));}
int tg_terrain_get_height_stats(const TgTerrain *t,TgTerrainHeightStats *out){
 size_t i; double sum=0.0f; float lo,hi;
 if(!t||!out||!t->vertices||!t->vertex_count)return -1;
 lo=hi=t->vertices[0].position.y;
 for(i=0;i<t->vertex_count;++i){float h=t->vertices[i].position.y;if(!isfinite(h))return -1;if(h<lo)lo=h;if(h>hi)hi=h;sum+=(double)h;}
 out->minimum=lo;out->maximum=hi;out->mean=(float)(sum/(double)t->vertex_count);return 0;
}
