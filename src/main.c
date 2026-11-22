#define VK_USE_PLATFORM_ANDROID_KHR
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <jni.h>
#include <android/native_window.h>
#include <vulkan/vulkan.h>
#include "world.h"
#include "animation.h"
#include "player.h"
#include "camera3d.h"
#include "sea_camera.h"
#include "sea_test_scene.h"
#include "gpu_mesh_vk.h"
#include "weather.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#define LOG(...) __android_log_print(ANDROID_LOG_INFO,"TORNIA_GULD",__VA_ARGS__)
#define ERR(...) __android_log_print(ANDROID_LOG_ERROR,"TORNIA_GULD",__VA_ARGS__)
#define MAXV 40000
typedef struct {float x,y,u,v,r,g,b,a,tex;} V;
typedef struct {
 TgMat4 view_projection;
 float time_seconds;
 float padding[3];
 float camera_position[4];
 float sun_direction[4];
} TgSeaPushConstants;
typedef struct { float camera_forward[4],camera_right[4],camera_up[4],sun_direction[4],weather[4],cloud[4],camera_position[4]; } TgSkyPushConstants;
typedef struct {
 struct android_app *app; VkInstance in; VkSurfaceKHR surface; VkPhysicalDevice gpu; VkDevice dev; VkQueue q;
 uint32_t qf,w,h,n; VkSwapchainKHR sw; VkFormat fmt; VkImage *imgs; VkImageView *views; VkFramebuffer *fb;
 VkRenderPass rp; VkPipelineLayout layout; VkPipeline pipe; VkCommandPool pool; VkCommandBuffer *cmd;
 VkImage depth_image; VkDeviceMemory depth_memory; VkImageView depth_view; VkFormat depth_format;
 TgSeaTestScene *sea_scene; TgCamera3D sea_camera;
 VkPipeline sea_pipe; VkPipeline sun_pipe; VkPipeline sky_pipe; VkPipelineLayout sea_layout; VkPipelineLayout sky_layout;
 uint32_t sun_start,sun_nv;
 int sea_renderer_attempted,sea_renderer_ready,sea_draw_logged;
  float sea_time_seconds,sea_yaw,sea_pitch;
 TgWeatherState weather;
 int sea_touch_mode,sea_pointer_a,sea_pointer_b;
 float sea_touch_x,sea_touch_y,sea_touch_distance;
 VkBuffer vb; VkDeviceMemory vm; VkSemaphore semA,semB; VkFence fence;
 VkImage atlas; VkDeviceMemory atlas_mem; VkImageView atlas_view; VkSampler atlas_sampler;
 VkImage sprite_image; VkDeviceMemory sprite_memory; VkImageView sprite_view; VkSampler sprite_sampler;
 uint32_t sprite_width,sprite_height; AnimationSet animations; AnimationController player_animation; int sprite_ready,world_resources_ready; uint64_t last_frame_ns;
 VkDescriptorSetLayout descriptor_layout; VkDescriptorPool descriptor_pool; VkDescriptorSet descriptor_set;
 V v[MAXV]; uint32_t nv,sky_nv; int ready,notice,scene,dragging,world_logged;
 World world; WorldCamera camera; float last_x,last_y;
 Player player; int camera_follow_enabled;
 int joystick_active,joystick_pointer_id,camera_pointer_id;

} E;

static void push(E*e,float x,float y,float w,float h,float r,float g,float b){
 float l=x*2/e->w-1,rr=(x+w)*2/e->w-1,t=y*2/e->h-1,bb=(y+h)*2/e->h-1;
 V a={l,t,0,0,r,g,b,1,0},c={rr,t,0,0,r,g,b,1,0},d={rr,bb,0,0,r,g,b,1,0},f={l,bb,0,0,r,g,b,1,0};
 if(e->nv+6>MAXV)return;e->v[e->nv++]=a;e->v[e->nv++]=c;e->v[e->nv++]=d;e->v[e->nv++]=a;e->v[e->nv++]=d;e->v[e->nv++]=f;
}

static void push_sky_band(E*e,float y0,float y1,
                         float tr,float tg,float tb,
                         float br,float bg,float bb){
 float top=y0*2.0f/(float)e->h-1.0f;
 float bottom=y1*2.0f/(float)e->h-1.0f;
 float left=-1.0f,right=1.0f;
 V a={left,top,0,0,tr,tg,tb,1,0};
 V b={right,top,0,0,tr,tg,tb,1,0};
 V c={right,bottom,0,0,br,bg,bb,1,0};
 V d={left,bottom,0,0,br,bg,bb,1,0};
 if(e->nv+6u>MAXV)return;
 e->v[e->nv++]=a;e->v[e->nv++]=b;e->v[e->nv++]=c;
 e->v[e->nv++]=a;e->v[e->nv++]=c;e->v[e->nv++]=d;
}
static void push_disk(E*e,float cx,float cy,float radius,float r,float g,float b,float a){
 const unsigned segments=28u;
 for(unsigned i=0;i<segments;i++){
  float a0=(float)i*6.28318530718f/(float)segments;
  float a1=(float)(i+1u)*6.28318530718f/(float)segments;
  float x0=cx+cosf(a0)*radius,y0=cy+sinf(a0)*radius;
  float x1=cx+cosf(a1)*radius,y1=cy+sinf(a1)*radius;
  if(e->nv+3u>MAXV)return;
  V c={cx*2/e->w-1,cy*2/e->h-1,0,0,r,g,b,a,0};
  V p0={x0*2/e->w-1,y0*2/e->h-1,0,0,r,g,b,a,0};
  V p1={x1*2/e->w-1,y1*2/e->h-1,0,0,r,g,b,a,0};
  e->v[e->nv++]=c;e->v[e->nv++]=p0;e->v[e->nv++]=p1;
 }
}
static void push_depth_disk(E*e,float cx,float cy,float radius,float depth,float r,float g,float b,float a){
 const unsigned segments=36u;
 for(unsigned i=0;i<segments;i++){
  float a0=(float)i*6.28318530718f/(float)segments;
  float a1=(float)(i+1u)*6.28318530718f/(float)segments;
  float x0=cx+cosf(a0)*radius,y0=cy+sinf(a0)*radius;
  float x1=cx+cosf(a1)*radius,y1=cy+sinf(a1)*radius;
  if(e->nv+3u>MAXV)return;
  V center={cx*2.0f/(float)e->w-1.0f,cy*2.0f/(float)e->h-1.0f,0,0,r,g,b,a,depth};
  V p0={x0*2.0f/(float)e->w-1.0f,y0*2.0f/(float)e->h-1.0f,0,0,r,g,b,a,depth};
  V p1={x1*2.0f/(float)e->w-1.0f,y1*2.0f/(float)e->h-1.0f,0,0,r,g,b,a,depth};
  e->v[e->nv++]=center;e->v[e->nv++]=p0;e->v[e->nv++]=p1;
 }
}
static void joystick_metrics(E*e,float*cx,float*cy,float*radius){
 float short_side=(float)(e->w<e->h?e->w:e->h);
 *radius=short_side*0.09f;
 if(*radius<42.0f)*radius=42.0f;
 if(*radius>76.0f)*radius=76.0f;
 *cx=*radius*1.65f;
 *cy=(float)e->h-*radius*1.55f;
}
static void draw_joystick(E*e){
 float cx,cy,r,knob_x=0.0f,knob_y=0.0f;
 joystick_metrics(e,&cx,&cy,&r);
 if(e->joystick_active){
  knob_x=e->player.input_x*e->player.input_strength*r*0.62f;
  knob_y=e->player.input_y*e->player.input_strength*r*0.62f;
 }
 push_disk(e,cx,cy,r,.035f,.055f,.075f,.48f);
 push_disk(e,cx,cy,r*.86f,.24f,.29f,.32f,.36f);
 push_disk(e,cx+knob_x,cy+knob_y,r*.36f,.77f,.82f,.84f,.72f);
}
static void update_joystick(E*e,float x,float y){
 float cx,cy,r,dx,dy,magnitude;
 joystick_metrics(e,&cx,&cy,&r);
 dx=(x-cx)/r;dy=(y-cy)/r;
 magnitude=sqrtf(dx*dx+dy*dy);
 if(magnitude>1.0f){dx/=magnitude;dy/=magnitude;}
 player_set_input(&e->player,dx,dy);
}

static const uint8_t* bits(char c){
 #define GL(ch,a,b,c1,d,e,f,g) case ch:{static const uint8_t z[7]={a,b,c1,d,e,f,g};return z;}
 static const uint8_t z[7]={0};switch(c){
 GL('A',14,17,17,31,17,17,17) GL('B',30,17,17,30,17,17,30) GL('C',14,17,16,16,16,17,14)
 GL('D',30,17,17,17,17,17,30) GL('E',31,16,16,30,16,16,31) GL('F',31,16,16,30,16,16,16)
 GL('G',14,17,16,23,17,17,15) GL('I',14,4,4,4,4,4,14) GL('L',16,16,16,16,16,16,31)
 GL('M',17,27,21,21,17,17,17) GL('N',17,25,21,19,17,17,17) GL('O',14,17,17,17,17,17,14)
 GL('P',30,17,17,30,16,16,16) GL('R',30,17,17,30,20,18,17) GL('S',15,16,16,14,1,1,30)
 GL('T',31,4,4,4,4,4,4) GL('U',17,17,17,17,17,17,14) GL('V',17,17,17,17,17,10,4)
 GL('0',14,17,19,21,25,17,14) GL('1',4,12,4,4,4,4,14) GL('2',14,17,1,2,4,8,31)
 default:return z;} 
 #undef GL
}
static void label(E*e,const char*s,float x,float y,float k,float r,float g,float b){
 for(;*s;s++,x+=6*k){const uint8_t*p=bits(*s);for(int j=0;j<7;j++)for(int i=0;i<5;i++)if(p[j]&(1<<(4-i)))push(e,x+i*k,y+j*k,k,k,r,g,b);}
}
static void push_tex(E*e,float x,float y,float w,float h,uint32_t cell){
 float l=x*2/e->w-1,rr=(x+w)*2/e->w-1,t=y*2/e->h-1,bb=(y+h)*2/e->h-1;
 float u0=(float)((cell%4u)*256u+1u)/1024.0f;
 float v0=(float)((cell/4u)*256u+1u)/1024.0f;
 float u1=(float)((cell%4u)*256u+255u)/1024.0f;
 float v1=(float)((cell/4u)*256u+255u)/1024.0f;
 V a={l,t,u0,v0,1,1,1,1,1},b={rr,t,u1,v0,1,1,1,1,1};
 V d={rr,bb,u1,v1,1,1,1,1,1},f={l,bb,u0,v1,1,1,1,1,1};
 if(e->nv+6>MAXV)return;
 e->v[e->nv++]=a;e->v[e->nv++]=b;e->v[e->nv++]=d;
 e->v[e->nv++]=a;e->v[e->nv++]=d;e->v[e->nv++]=f;
}

static uint32_t memtype(E*e,uint32_t mask,VkMemoryPropertyFlags flags);
static void push_sprite(E*e,float x,float y,float w,float h,const AnimationFrame*f,int flip_x){
 if(!f||!e->sprite_width||!e->sprite_height)return;
 float l=x*2/e->w-1,rr=(x+w)*2/e->w-1,t=y*2/e->h-1,bb=(y+h)*2/e->h-1;
 float uv[8];
 animation_frame_uvs(f,(uint16_t)e->sprite_width,(uint16_t)e->sprite_height,flip_x,uv);
 V a={l,t,uv[0],uv[1],1,1,1,1,2};
 V b={rr,t,uv[2],uv[3],1,1,1,1,2};
 V d={rr,bb,uv[4],uv[5],1,1,1,1,2};
 V c={l,bb,uv[6],uv[7],1,1,1,1,2};
 if(e->nv+6>MAXV)return;
 e->v[e->nv++]=a;e->v[e->nv++]=b;e->v[e->nv++]=d;e->v[e->nv++]=a;e->v[e->nv++]=d;e->v[e->nv++]=c;
}
static int create_sprite_texture(E*e,const char*path){
 AAsset*a=AAssetManager_open(e->app->activity->assetManager,path,AASSET_MODE_BUFFER);if(!a)return -1;
 off_t length=AAsset_getLength(a);if(length<=0||length>8*1024*1024){AAsset_close(a);return -1;}
 jbyte*bytes=malloc((size_t)length);if(!bytes){AAsset_close(a);return -1;}
 int got=AAsset_read(a,bytes,(size_t)length);AAsset_close(a);if(got!=(int)length){free(bytes);return -1;}
 JavaVM*vm=e->app->activity->vm;JNIEnv*env=NULL;int attached=0,result=-1;
 jclass factory=NULL,bc=NULL;jmethodID decode=NULL,gw=NULL,gh=NULL,gp=NULL;
 jbyteArray encoded=NULL;jobject bitmap=NULL;jintArray pixels=NULL;jint*p=NULL;uint8_t*rgba=NULL;
 if(!vm)goto done;
 jint state=(*vm)->GetEnv(vm,(void**)&env,JNI_VERSION_1_6);
 if(state==JNI_EDETACHED){if((*vm)->AttachCurrentThread(vm,&env,NULL)!=JNI_OK||!env)goto done;attached=1;}else if(state!=JNI_OK||!env)goto done;
 factory=(*env)->FindClass(env,"android/graphics/BitmapFactory");if(!factory||(*env)->ExceptionCheck(env))goto done;
 decode=(*env)->GetStaticMethodID(env,factory,"decodeByteArray","([BII)Landroid/graphics/Bitmap;");if(!decode||(*env)->ExceptionCheck(env))goto done;
 encoded=(*env)->NewByteArray(env,(jsize)length);if(!encoded||(*env)->ExceptionCheck(env))goto done;
 (*env)->SetByteArrayRegion(env,encoded,0,(jsize)length,bytes);free(bytes);bytes=NULL;if((*env)->ExceptionCheck(env))goto done;
 bitmap=(*env)->CallStaticObjectMethod(env,factory,decode,encoded,0,(jint)length);if((*env)->ExceptionCheck(env)||!bitmap)goto done;
 bc=(*env)->FindClass(env,"android/graphics/Bitmap");if(!bc||(*env)->ExceptionCheck(env))goto done;
 gw=(*env)->GetMethodID(env,bc,"getWidth","()I");gh=(*env)->GetMethodID(env,bc,"getHeight","()I");gp=(*env)->GetMethodID(env,bc,"getPixels","([IIIIIII)V");
 if(!gw||!gh||!gp||(*env)->ExceptionCheck(env))goto done;
 jint width=(*env)->CallIntMethod(env,bitmap,gw),height=(*env)->CallIntMethod(env,bitmap,gh);
 if((*env)->ExceptionCheck(env)||width<=0||height<=0||width>4096||height>4096)goto done;
 size_t count=(size_t)width*(size_t)height;if(count>16000000u)goto done;
 pixels=(*env)->NewIntArray(env,(jsize)count);if(!pixels||(*env)->ExceptionCheck(env))goto done;
 (*env)->CallVoidMethod(env,bitmap,gp,pixels,0,width,0,0,width,height);if((*env)->ExceptionCheck(env))goto done;
 p=(*env)->GetIntArrayElements(env,pixels,0);if(!p||(*env)->ExceptionCheck(env))goto done;
 rgba=malloc(count*4u);if(!rgba)goto done;
 for(size_t i=0;i<count;i++){uint32_t v=(uint32_t)p[i];rgba[i*4u]=(uint8_t)(v>>16);rgba[i*4u+1u]=(uint8_t)(v>>8);rgba[i*4u+2u]=(uint8_t)v;rgba[i*4u+3u]=(uint8_t)(v>>24);}
 (*env)->ReleaseIntArrayElements(env,pixels,p,JNI_ABORT);p=NULL;
 (*env)->DeleteLocalRef(env,pixels);pixels=NULL;(*env)->DeleteLocalRef(env,bitmap);bitmap=NULL;(*env)->DeleteLocalRef(env,bc);bc=NULL;(*env)->DeleteLocalRef(env,factory);factory=NULL;(*env)->DeleteLocalRef(env,encoded);encoded=NULL;
 if(attached){(*vm)->DetachCurrentThread(vm);attached=0;}
 e->sprite_width=(uint32_t)width;e->sprite_height=(uint32_t)height;
 VkDeviceSize data_size=(VkDeviceSize)count*4u;VkBuffer staging=0;VkDeviceMemory staging_mem=0;
 VkBufferCreateInfo bci={.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,.size=data_size,.usage=VK_BUFFER_USAGE_TRANSFER_SRC_BIT,.sharingMode=VK_SHARING_MODE_EXCLUSIVE};
 if(vkCreateBuffer(e->dev,&bci,0,&staging)!=VK_SUCCESS)goto upload_done;
 VkMemoryRequirements req;vkGetBufferMemoryRequirements(e->dev,staging,&req);
 uint32_t mt=memtype(e,req.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);if(mt==UINT32_MAX)goto upload_done;
 VkMemoryAllocateInfo mai={.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,.allocationSize=req.size,.memoryTypeIndex=mt};
 if(vkAllocateMemory(e->dev,&mai,0,&staging_mem)!=VK_SUCCESS)goto upload_done;
 if(vkBindBufferMemory(e->dev,staging,staging_mem,0)!=VK_SUCCESS)goto upload_done;
 void*m=NULL;if(vkMapMemory(e->dev,staging_mem,0,data_size,0,&m)!=VK_SUCCESS)goto upload_done;memcpy(m,rgba,(size_t)data_size);vkUnmapMemory(e->dev,staging_mem);
 VkImageCreateInfo ici={.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,.imageType=VK_IMAGE_TYPE_2D,.format=VK_FORMAT_R8G8B8A8_UNORM,.extent={(uint32_t)width,(uint32_t)height,1},.mipLevels=1,.arrayLayers=1,.samples=VK_SAMPLE_COUNT_1_BIT,.tiling=VK_IMAGE_TILING_OPTIMAL,.usage=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,.sharingMode=VK_SHARING_MODE_EXCLUSIVE,.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED};
 if(vkCreateImage(e->dev,&ici,0,&e->sprite_image)!=VK_SUCCESS)goto upload_done;
 vkGetImageMemoryRequirements(e->dev,e->sprite_image,&req);mt=memtype(e,req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);if(mt==UINT32_MAX)goto upload_done;
 mai.allocationSize=req.size;mai.memoryTypeIndex=mt;if(vkAllocateMemory(e->dev,&mai,0,&e->sprite_memory)!=VK_SUCCESS)goto upload_done;
 if(vkBindImageMemory(e->dev,e->sprite_image,e->sprite_memory,0)!=VK_SUCCESS)goto upload_done;
 VkCommandBufferAllocateInfo cai={.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,.commandPool=e->pool,.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY,.commandBufferCount=1};VkCommandBuffer cmd=0;
 if(vkAllocateCommandBuffers(e->dev,&cai,&cmd)!=VK_SUCCESS)goto upload_done;
 VkCommandBufferBeginInfo begin={.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
 if(vkBeginCommandBuffer(cmd,&begin)!=VK_SUCCESS){vkFreeCommandBuffers(e->dev,e->pool,1,&cmd);goto upload_done;}
 VkImageMemoryBarrier b1={.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT,.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED,.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.image=e->sprite_image,.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
 vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,0,0,0,1,&b1);
 VkBufferImageCopy cp={.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},.imageExtent={(uint32_t)width,(uint32_t)height,1}};vkCmdCopyBufferToImage(cmd,staging,e->sprite_image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&cp);
 VkImageMemoryBarrier b2={.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT,.dstAccessMask=VK_ACCESS_SHADER_READ_BIT,.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,.newLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.image=e->sprite_image,.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
 vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,0,0,0,1,&b2);
 if(vkEndCommandBuffer(cmd)!=VK_SUCCESS){vkFreeCommandBuffers(e->dev,e->pool,1,&cmd);goto upload_done;}
 VkSubmitInfo si={.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,.commandBufferCount=1,.pCommandBuffers=&cmd};VkResult vr=vkQueueSubmit(e->q,1,&si,0);if(vr==VK_SUCCESS)vr=vkQueueWaitIdle(e->q);vkFreeCommandBuffers(e->dev,e->pool,1,&cmd);if(vr!=VK_SUCCESS)goto upload_done;
 VkImageViewCreateInfo vi={.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,.image=e->sprite_image,.viewType=VK_IMAGE_VIEW_TYPE_2D,.format=VK_FORMAT_R8G8B8A8_UNORM,.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
 if(vkCreateImageView(e->dev,&vi,0,&e->sprite_view)!=VK_SUCCESS)goto upload_done;
 VkSamplerCreateInfo sci={.sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,.magFilter=VK_FILTER_NEAREST,.minFilter=VK_FILTER_NEAREST,.mipmapMode=VK_SAMPLER_MIPMAP_MODE_NEAREST,.addressModeU=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,.addressModeV=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,.addressModeW=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,.maxAnisotropy=1.0f,.minLod=0,.maxLod=0};
 if(vkCreateSampler(e->dev,&sci,0,&e->sprite_sampler)!=VK_SUCCESS)goto upload_done;
 VkDescriptorImageInfo image={.sampler=e->sprite_sampler,.imageView=e->sprite_view,.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
 VkWriteDescriptorSet wr={.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,.dstSet=e->descriptor_set,.dstBinding=1,.descriptorCount=1,.descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,.pImageInfo=&image};vkUpdateDescriptorSets(e->dev,1,&wr,0,0);
 result=0;LOG("Sprite sheet uploaded: %ux%u",e->sprite_width,e->sprite_height);
 upload_done:
 if(staging)vkDestroyBuffer(e->dev,staging,0);if(staging_mem)vkFreeMemory(e->dev,staging_mem,0);free(rgba);return result;
 done:
 if(p&&pixels)(*env)->ReleaseIntArrayElements(env,pixels,p,JNI_ABORT);
 if(env){if(pixels)(*env)->DeleteLocalRef(env,pixels);if(bitmap)(*env)->DeleteLocalRef(env,bitmap);if(bc)(*env)->DeleteLocalRef(env,bc);if(factory)(*env)->DeleteLocalRef(env,factory);if(encoded)(*env)->DeleteLocalRef(env,encoded);if((*env)->ExceptionCheck(env))(*env)->ExceptionClear(env);}
 if(bytes)free(bytes);if(rgba)free(rgba);if(attached)(*vm)->DetachCurrentThread(vm);return -1;
}
static int load_animation_config(E*e,const char*path){
 AAsset*a=AAssetManager_open(e->app->activity->assetManager,path,AASSET_MODE_BUFFER);if(!a)return -1;
 off_t n=AAsset_getLength(a);if(n<=0||n>65536){AAsset_close(a);return -1;}
 char*text=malloc((size_t)n+1u);if(!text){AAsset_close(a);return -1;}
 int got=AAsset_read(a,text,(size_t)n);AAsset_close(a);if(got!=(int)n){free(text);return -1;}text[n]='\0';
 int ok=animation_load_text(&e->animations,text,(uint16_t)e->sprite_width,(uint16_t)e->sprite_height);free(text);
 if(ok)return -1;
 animation_controller_init(&e->player_animation);
 if(animation_controller_update(&e->player_animation,&e->animations,"idle",0,0)!=0)return -1;
 e->sprite_ready=1;LOG("Animation system ready: %u clips / %u frames",e->animations.clip_count,e->animations.frame_count);return 0;
}

static void draw_world(E*e){
 e->nv=0;e->sky_nv=0;
 push(e,0,0,e->w,e->h,.055f,.13f,.065f);
 const float tile_px=WORLD_TILE_SIZE*e->camera.zoom;
 for(uint32_t y=0;y<WORLD_HEIGHT;y++)for(uint32_t x=0;x<WORLD_WIDTH;x++){
  const WorldTile*t=&e->world.tiles[y][x];
  float sx=((float)x*WORLD_TILE_SIZE-e->camera.x)*e->camera.zoom;
  float sy=((float)y*WORLD_TILE_SIZE-e->camera.y)*e->camera.zoom;
  if(sx+tile_px<0||sy+tile_px<0||sx>e->w||sy>e->h)continue;
  push_tex(e,sx,sy,tile_px+1,tile_px+1,0);
  for(uint32_t k=0;k<t->overlay_count;k++){
   uint32_t id=t->overlays[k];
   float scale=(id==13u)?0.88f:0.78f;
   float size=tile_px*scale;
   float jitter_x=(float)((id*17u+x*7u)%17u)/100.0f;
   float jitter_y=(float)((id*11u+y*9u)%17u)/100.0f;
   float ox=sx+(tile_px-size)*.5f+tile_px*jitter_x;
   float oy=sy+(tile_px-size)*.5f+tile_px*jitter_y;
   push_tex(e,ox,oy,size,size,id);
  }
 }
 if(e->sprite_ready){
  const AnimationFrame*f=animation_controller_current_frame(&e->player_animation,&e->animations);
  if(f){
   float screen_x=(e->player.x-e->camera.x)*e->camera.zoom;
   float screen_y=(e->player.y-e->camera.y)*e->camera.zoom;
   float height=WORLD_TILE_SIZE*.78f*e->camera.zoom;
   float width=height*(float)f->width/(float)f->height;
   push_sprite(e,screen_x-width*.5f,screen_y-height,width,height,f,e->player_animation.flip_x);
  }
 }
 push(e,0,0,e->w,42,.025f,.065f,.035f);
 label(e,"WORLD",20,14,2,.91f,.88f,.68f);
 label(e,"DRAG TO MOVE CAMERA",20,e->h-30,1.5f,.78f,.87f,.70f);
 draw_joystick(e);
 if(!e->world_logged){LOG("Textured world ready: vertices=%u viewport=%ux%u",e->nv,e->w,e->h);e->world_logged=1;}
}

static void menu(E*e){
 e->nv=0;e->sky_nv=0;float cx=e->w*.5f,bw=e->w*.36f,bh=e->h*.14f,bx=cx-bw/2,by=e->h*.52f;
 push(e,0,0,e->w,e->h,.035f,.052f,.07f);
 label(e,"TORNIA_GULD",cx-84,e->h*.20f,4,.91f,.82f,.63f);
 
 push(e,bx,by,bw,bh,.62f,.45f,.23f);push(e,bx+3,by+3,bw-6,bh-6,.10f,.12f,.14f);
 label(e,"START GAME",cx-59,by+bh/2-7,2,.96f,.87f,.70f);
 float ey=by+bh+e->h*.045f;push(e,bx,ey,bw,bh,.38f,.4f,.42f);push(e,bx+3,ey+3,bw-6,bh-6,.09f,.11f,.13f);
 label(e,"EXIT",cx-20,ey+bh/2-7,2,.85f,.86f,.87f);
 if(e->notice){float my=e->h*.84f,mw=e->w*.55f;push(e,cx-mw/2,my,mw,e->h*.11f,.60f,.43f,.22f);push(e,cx-mw/2+3,my+3,mw-6,e->h*.11f-6,.08f,.10f,.12f);label(e,"COMING SOON",cx-49,my+e->h*.035f,1.5f,.98f,.88f,.72f);}
}
static void push_sky_quad(E*e){
 const V a={-1,-1,0,0,1,1,1,1,0},b={1,-1,1,0,1,1,1,1,0},c={1,1,1,1,1,1,1,1,0},d={-1,1,0,1,1,1,1,1,0};
 if(e->nv+6u>MAXV)return;
 e->v[e->nv++]=a;e->v[e->nv++]=b;e->v[e->nv++]=c;e->v[e->nv++]=a;e->v[e->nv++]=c;e->v[e->nv++]=d;
}
static void push_preview_sun(E*e){
 if(!e->sea_renderer_ready||e->w==0u||e->h==0u)return;
 /* Project a distant point on the fixed celestial ray for screen position and depth. */
 const float sun_x=e->weather.sun_direction[0],sun_y=e->weather.sun_direction[1],sun_z=e->weather.sun_direction[2];
 const float sun_distance=1000.0f;
 const float world_x=e->sea_camera.position.x+sun_x*sun_distance;
 const float world_y=e->sea_camera.position.y+sun_y*sun_distance;
 const float world_z=e->sea_camera.position.z+sun_z*sun_distance;
 const float*m=e->sea_camera.view_projection.m;
 float clip_x=m[0]*world_x+m[4]*world_y+m[8]*world_z+m[12];
 float clip_y=m[1]*world_x+m[5]*world_y+m[9]*world_z+m[13];
 float clip_z=m[2]*world_x+m[6]*world_y+m[10]*world_z+m[14];
 float clip_w=m[3]*world_x+m[7]*world_y+m[11]*world_z+m[15];
 if(clip_w<=0.001f||clip_z<0.0f||clip_z>clip_w)return;
 float ndc_x=clip_x/clip_w,ndc_y=clip_y/clip_w;
 if(ndc_x < -1.25f||ndc_x > 1.25f||ndc_y < -1.25f||ndc_y > 1.25f)return;
 float sx=(ndc_x*0.5f+0.5f)*(float)e->w;
 float sy=(ndc_y*0.5f+0.5f)*(float)e->h;
 float sun_depth=clip_z/clip_w;
 float radius=(0.00465f/tanf(e->sea_camera.vertical_fov_radians*0.5f))*(float)e->h*0.5f;
 /* The disc carries the sun's projected depth; the ocean depth test clips it at the horizon. */
 push_depth_disk(e,sx,sy,radius*3.2f,sun_depth,1.0f,.48f,.20f,.045f);
 push_depth_disk(e,sx,sy,radius*2.0f,sun_depth,1.0f,.62f,.30f,.10f);
 push_depth_disk(e,sx,sy,radius,sun_depth,1.0f,.86f,.62f,.96f);
}
static void draw_building(E*e){
 e->nv=0;e->sky_nv=0;
 float cx=(float)e->w*.5f;
 /* 3D view-dependent sky; keep the old bands as a safe fallback. */
 if(e->sky_pipe!=VK_NULL_HANDLE)push_sky_quad(e);
 else{push_sky_band(e,0.0f,(float)e->h*0.52f,.012f,.030f,.085f,.72f,.25f,.105f);push_sky_band(e,(float)e->h*0.52f,(float)e->h,.72f,.25f,.105f,.16f,.055f,.085f);}
 e->sky_nv=e->nv;
 e->sun_start=e->nv;
 push_preview_sun(e);
 e->sun_nv=e->nv-e->sun_start;
 label(e,"TORNIA_GULD",cx-84,e->h*.27f,4,.91f,.82f,.63f);
 label(e,"BUILDING",cx-56,e->h*.47f,3,.96f,.87f,.70f);
 label(e,"IN PROGRESS",cx-66,e->h*.56f,2,.72f,.78f,.82f);
}
static uint32_t memtype(E*e,uint32_t mask,VkMemoryPropertyFlags flags){VkPhysicalDeviceMemoryProperties p;vkGetPhysicalDeviceMemoryProperties(e->gpu,&p);for(uint32_t i=0;i<p.memoryTypeCount;i++)if((mask&(1u<<i))&&(p.memoryTypes[i].propertyFlags&flags)==flags)return i;return UINT32_MAX;}
static VkShaderModule shader(E*e,const char*path){
 AAsset*a=AAssetManager_open(e->app->activity->assetManager,path,AASSET_MODE_BUFFER);
 if(!a){ERR("Shader asset not found: %s",path);return VK_NULL_HANDLE;}
 off_t length=AAsset_getLength(a);
 if(length<=0||(length%4)!=0||length>4*1024*1024){
  ERR("Invalid shader asset size: %s (%lld bytes)",path,(long long)length);AAsset_close(a);return VK_NULL_HANDLE;
 }
 uint32_t*p=malloc((size_t)length);
 if(!p){ERR("Shader allocation failed: %s",path);AAsset_close(a);return VK_NULL_HANDLE;}
 int got=AAsset_read(a,p,(size_t)length);AAsset_close(a);
 if(got!=(int)length){ERR("Short shader read: %s (%d/%lld)",path,got,(long long)length);free(p);return VK_NULL_HANDLE;}
 VkShaderModuleCreateInfo ci={.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,.codeSize=(size_t)length,.pCode=p};
 VkShaderModule module=VK_NULL_HANDLE;VkResult vr=vkCreateShaderModule(e->dev,&ci,NULL,&module);free(p);
 if(vr!=VK_SUCCESS){ERR("vkCreateShaderModule failed for %s: VkResult=%d",path,(int)vr);return VK_NULL_HANDLE;}
 LOG("Shader module loaded: %s",path);return module;
}
#define ATLAS_SIDE 1024
#define ATLAS_CELL 256
static int decode_asset_rgba(E*e,const char*path,uint8_t*out){
 AAsset*a=AAssetManager_open(e->app->activity->assetManager,path,AASSET_MODE_BUFFER);
 if(!a){ERR("Asset not found: %s",path);return -1;}
 off_t length=AAsset_getLength(a);
 if(length<=0||length>8*1024*1024){ERR("Invalid asset size %s: %lld",path,(long long)length);AAsset_close(a);return -1;}
 jbyte*bytes=malloc((size_t)length);
 if(!bytes){AAsset_close(a);return -1;}
 int got=AAsset_read(a,bytes,(size_t)length);AAsset_close(a);
 if(got!=(int)length){ERR("Short asset read %s: %d/%lld",path,got,(long long)length);free(bytes);return -1;}

 JavaVM*vm=e->app->activity->vm;
 JNIEnv*env=NULL;
 int attached=0,result=-1;
 jclass factory=NULL,bitmap_cls=NULL;
 jmethodID decode=NULL,scale=NULL,get_pixels=NULL;
 jbyteArray encoded=NULL;
 jobject bitmap=NULL,scaled=NULL;
 jintArray pixels=NULL;
 jint*p=NULL;
 if(!vm){ERR("Android JavaVM is unavailable for %s",path);goto done;}
 jint state=(*vm)->GetEnv(vm,(void**)&env,JNI_VERSION_1_6);
 if(state==JNI_EDETACHED){
  if((*vm)->AttachCurrentThread(vm,&env,NULL)!=JNI_OK||!env){ERR("Could not attach asset thread to Android JVM");goto done;}
  attached=1;
 }else if(state!=JNI_OK||!env){
  ERR("Could not access Android JNI environment for %s",path);goto done;
 }

 factory=(*env)->FindClass(env,"android/graphics/BitmapFactory");
 if(!factory||(*env)->ExceptionCheck(env)){ERR("BitmapFactory lookup failed for %s",path);goto done;}
 decode=(*env)->GetStaticMethodID(env,factory,"decodeByteArray","([BII)Landroid/graphics/Bitmap;");
 if(!decode||(*env)->ExceptionCheck(env)){ERR("BitmapFactory.decodeByteArray lookup failed for %s",path);goto done;}
 encoded=(*env)->NewByteArray(env,(jsize)length);
 if(!encoded||(*env)->ExceptionCheck(env)){ERR("Could not allocate Java byte array for %s",path);goto done;}
 (*env)->SetByteArrayRegion(env,encoded,0,(jsize)length,bytes);
 free(bytes);bytes=NULL;
 if((*env)->ExceptionCheck(env)){ERR("Could not copy encoded PNG bytes for %s",path);goto done;}
 bitmap=(*env)->CallStaticObjectMethod(env,factory,decode,encoded,0,(jint)length);
 if((*env)->ExceptionCheck(env)||!bitmap){ERR("BitmapFactory could not decode %s",path);goto done;}

 bitmap_cls=(*env)->FindClass(env,"android/graphics/Bitmap");
 if(!bitmap_cls||(*env)->ExceptionCheck(env)){ERR("Bitmap class lookup failed for %s",path);goto done;}
 scale=(*env)->GetStaticMethodID(env,bitmap_cls,"createScaledBitmap","(Landroid/graphics/Bitmap;IIZ)Landroid/graphics/Bitmap;");
 get_pixels=(*env)->GetMethodID(env,bitmap_cls,"getPixels","([IIIIIII)V");
 if(!scale||!get_pixels||(*env)->ExceptionCheck(env)){ERR("Bitmap pixel methods unavailable for %s",path);goto done;}
 scaled=(*env)->CallStaticObjectMethod(env,bitmap_cls,scale,bitmap,(jint)ATLAS_CELL,(jint)ATLAS_CELL,JNI_TRUE);
 if((*env)->ExceptionCheck(env)||!scaled){ERR("Could not scale terrain asset %s",path);goto done;}
 pixels=(*env)->NewIntArray(env,ATLAS_CELL*ATLAS_CELL);
 if(!pixels||(*env)->ExceptionCheck(env)){ERR("Could not allocate decoded pixel array for %s",path);goto done;}
 (*env)->CallVoidMethod(env,scaled,get_pixels,pixels,0,ATLAS_CELL,0,0,ATLAS_CELL,ATLAS_CELL);
 if((*env)->ExceptionCheck(env)){ERR("Bitmap.getPixels failed for %s",path);goto done;}
 p=(*env)->GetIntArrayElements(env,pixels,0);
 if(!p||(*env)->ExceptionCheck(env)){ERR("Could not access decoded pixels for %s",path);goto done;}
 for(int i=0;i<ATLAS_CELL*ATLAS_CELL;i++){
  uint32_t v=(uint32_t)p[i];
  out[i*4+0]=(uint8_t)(v>>16);out[i*4+1]=(uint8_t)(v>>8);
  out[i*4+2]=(uint8_t)v;out[i*4+3]=(uint8_t)(v>>24);
 }
 result=0;

done:
 if(p&&pixels)(*env)->ReleaseIntArrayElements(env,pixels,p,JNI_ABORT);
 if(env){
  if(pixels)(*env)->DeleteLocalRef(env,pixels);
  if(scaled&&scaled!=bitmap)(*env)->DeleteLocalRef(env,scaled);
  if(bitmap)(*env)->DeleteLocalRef(env,bitmap);
  if(bitmap_cls)(*env)->DeleteLocalRef(env,bitmap_cls);
  if(factory)(*env)->DeleteLocalRef(env,factory);
  if(encoded)(*env)->DeleteLocalRef(env,encoded);
  if((*env)->ExceptionCheck(env))(*env)->ExceptionClear(env);
 }
 if(bytes)free(bytes);
 if(attached)(*vm)->DetachCurrentThread(vm);
 return result;
}
static int create_texture_atlas(E*e){
 static const char*paths[14]={
  "tiles/grass/file_0000000016688210b7c4a280705377ef.png",
  "tiles/grass/grass_01.png","tiles/grass/grass_02.png","tiles/grass/grass_03.png",
  "tiles/grass/grass_04.png","tiles/grass/grass_05.png","tiles/grass/grass_06.png",
  "tiles/grass/grass_07.png","tiles/grass/grass_08.png","tiles/grass/grass_09.png",
  "tiles/grass/grass_10.png","tiles/grass/grass_11.png","tiles/grass/grass_12.png",
  "tiles/grass/file_0000000063808210b3fc9787a65c0cb7.png"
 };
 size_t atlas_bytes=(size_t)ATLAS_SIDE*ATLAS_SIDE*4u;
 size_t cell_bytes=(size_t)ATLAS_CELL*ATLAS_CELL*4u;
 uint8_t*rgba=calloc(1,atlas_bytes);uint8_t*cell=malloc(cell_bytes);
 uint32_t failures=0;
 if(!rgba||!cell){free(rgba);free(cell);return -1;}
 /* A deterministic grass fallback keeps the menu and world renderable even if a PNG fails. */
 for(uint32_t y=0;y<ATLAS_CELL;y++)for(uint32_t x=0;x<ATLAS_CELL;x++){
  size_t p=((size_t)y*ATLAS_SIDE+x)*4u;
  uint8_t shade=(uint8_t)(((x/8u+y/8u)&1u)?8u:0u);
  rgba[p+0]=(uint8_t)(72u+shade);rgba[p+1]=(uint8_t)(119u+shade);
  rgba[p+2]=(uint8_t)(48u+shade);rgba[p+3]=255u;
 }
 for(uint32_t i=0;i<14;i++){
  memset(cell,0,cell_bytes);
  if(decode_asset_rgba(e,paths[i],cell)){
   failures++;
   ERR("Using fallback for terrain asset %s",paths[i]);
   continue;
  }
  uint32_t ox=(i%4u)*ATLAS_CELL,oy=(i/4u)*ATLAS_CELL;
  for(uint32_t y=0;y<ATLAS_CELL;y++)
   memcpy(rgba+(((size_t)(oy+y)*ATLAS_SIDE+ox)*4u),cell+(size_t)y*ATLAS_CELL*4u,cell_bytes/ATLAS_CELL);
 }
 free(cell);
 if(failures)ERR("Terrain atlas loaded with %u fallback cell(s)",failures);
 VkBuffer staging=0;VkDeviceMemory staging_mem=0;
 VkBufferCreateInfo bci={.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,.size=atlas_bytes,.usage=VK_BUFFER_USAGE_TRANSFER_SRC_BIT,.sharingMode=VK_SHARING_MODE_EXCLUSIVE};
 if(vkCreateBuffer(e->dev,&bci,0,&staging)){free(rgba);return -1;}
 VkMemoryRequirements req;vkGetBufferMemoryRequirements(e->dev,staging,&req);
 uint32_t mt=memtype(e,req.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
 if(mt==UINT32_MAX){free(rgba);return -1;}
 VkMemoryAllocateInfo mai={.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,.allocationSize=req.size,.memoryTypeIndex=mt};
 if(vkAllocateMemory(e->dev,&mai,0,&staging_mem)){free(rgba);return -1;}
 vkBindBufferMemory(e->dev,staging,staging_mem,0);
 void*m=0;if(vkMapMemory(e->dev,staging_mem,0,atlas_bytes,0,&m)){free(rgba);return -1;}
 memcpy(m,rgba,atlas_bytes);vkUnmapMemory(e->dev,staging_mem);free(rgba);
 VkImageCreateInfo ici={.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,.imageType=VK_IMAGE_TYPE_2D,.format=VK_FORMAT_R8G8B8A8_UNORM,.extent={ATLAS_SIDE,ATLAS_SIDE,1},.mipLevels=1,.arrayLayers=1,.samples=VK_SAMPLE_COUNT_1_BIT,.tiling=VK_IMAGE_TILING_OPTIMAL,.usage=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,.sharingMode=VK_SHARING_MODE_EXCLUSIVE,.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED};
 if(vkCreateImage(e->dev,&ici,0,&e->atlas))return -1;
 vkGetImageMemoryRequirements(e->dev,e->atlas,&req);
 mt=memtype(e,req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);if(mt==UINT32_MAX)return -1;
 mai.allocationSize=req.size;mai.memoryTypeIndex=mt;
 if(vkAllocateMemory(e->dev,&mai,0,&e->atlas_mem))return -1;
 vkBindImageMemory(e->dev,e->atlas,e->atlas_mem,0);
 VkCommandBufferAllocateInfo cai={.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,.commandPool=e->pool,.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY,.commandBufferCount=1};
 VkCommandBuffer cmd;if(vkAllocateCommandBuffers(e->dev,&cai,&cmd))return -1;
 VkCommandBufferBeginInfo begin={.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
 vkBeginCommandBuffer(cmd,&begin);
 VkImageMemoryBarrier to_dst={.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,.srcAccessMask=0,.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT,.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED,.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.image=e->atlas,.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
 vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,0,0,0,1,&to_dst);
 VkBufferImageCopy copy={.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},.imageExtent={ATLAS_SIDE,ATLAS_SIDE,1}};
 vkCmdCopyBufferToImage(cmd,staging,e->atlas,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&copy);
 VkImageMemoryBarrier to_sample={.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT,.dstAccessMask=VK_ACCESS_SHADER_READ_BIT,.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,.newLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,.image=e->atlas,.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
 vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,0,0,0,1,&to_sample);
 vkEndCommandBuffer(cmd);VkSubmitInfo submit={.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,.commandBufferCount=1,.pCommandBuffers=&cmd};
 VkResult upload=vkQueueSubmit(e->q,1,&submit,0);if(upload==VK_SUCCESS)upload=vkQueueWaitIdle(e->q);
 vkFreeCommandBuffers(e->dev,e->pool,1,&cmd);vkDestroyBuffer(e->dev,staging,0);vkFreeMemory(e->dev,staging_mem,0);
 if(upload!=VK_SUCCESS)return -1;
 VkImageViewCreateInfo vi={.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,.image=e->atlas,.viewType=VK_IMAGE_VIEW_TYPE_2D,.format=VK_FORMAT_R8G8B8A8_UNORM,.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
 if(vkCreateImageView(e->dev,&vi,0,&e->atlas_view))return -1;
 VkSamplerCreateInfo sci={.sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,.magFilter=VK_FILTER_NEAREST,.minFilter=VK_FILTER_NEAREST,.mipmapMode=VK_SAMPLER_MIPMAP_MODE_NEAREST,.addressModeU=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,.addressModeV=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,.addressModeW=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,.maxAnisotropy=1.0f,.minLod=0,.maxLod=0,.borderColor=VK_BORDER_COLOR_INT_OPAQUE_BLACK};
 if(vkCreateSampler(e->dev,&sci,0,&e->atlas_sampler))return -1;
 VkDescriptorPoolSize ps={VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2};
 VkDescriptorPoolCreateInfo dp={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,.maxSets=1,.poolSizeCount=1,.pPoolSizes=&ps};
 if(vkCreateDescriptorPool(e->dev,&dp,0,&e->descriptor_pool))return -1;
 VkDescriptorSetAllocateInfo da={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,.descriptorPool=e->descriptor_pool,.descriptorSetCount=1,.pSetLayouts=&e->descriptor_layout};
 if(vkAllocateDescriptorSets(e->dev,&da,&e->descriptor_set))return -1;
 VkDescriptorImageInfo image_info={.sampler=e->atlas_sampler,.imageView=e->atlas_view,.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
 VkWriteDescriptorSet wr={.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,.dstSet=e->descriptor_set,.dstBinding=0,.descriptorCount=1,.descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,.pImageInfo=&image_info};
 vkUpdateDescriptorSets(e->dev,1,&wr,0,0);
 LOG("Vulkan terrain atlas uploaded; missing source images=%u",failures);
 return 0;
}
static int initialize_2d_world(E*e){
 if(e->world_resources_ready)return 0;
 if(create_sprite_texture(e,"sprites/characters/player/file_00000000ce0c8210854d0157a262ddc5.png")!=0||
    load_animation_config(e,"sprites/characters/player/player.anim")!=0){
  ERR("Player sprite animation resources could not be initialized");
  return -1;
 }
 world_generate(&e->world,0x4f4c5245u);
 player_init(&e->player,(float)WORLD_WIDTH*WORLD_TILE_SIZE*.5f,(float)WORLD_HEIGHT*WORLD_TILE_SIZE*.5f);
 world_camera_reset(&e->camera);
 e->camera.x=e->player.x-(float)e->w*.5f;
 e->camera.y=e->player.y-(float)e->h*.55f;
 world_camera_clamp(&e->camera,(float)e->w,(float)e->h);
 e->camera_follow_enabled=1;
 e->joystick_pointer_id=-1;
 e->camera_pointer_id=-1;
 e->world_resources_ready=1;
 LOG("2D world resources initialized on demand");
 return 0;
}
static int create_depth_target(E*e){
 static const VkFormat candidates[]={
  VK_FORMAT_D32_SFLOAT,
  VK_FORMAT_D32_SFLOAT_S8_UINT,
  VK_FORMAT_D24_UNORM_S8_UINT,
  VK_FORMAT_D16_UNORM
 };
 VkFormatProperties properties;
 VkImageCreateInfo image_info={
  .sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
  .imageType=VK_IMAGE_TYPE_2D,
  .extent={0u,0u,1u},
  .mipLevels=1u,
  .arrayLayers=1u,
  .samples=VK_SAMPLE_COUNT_1_BIT,
  .tiling=VK_IMAGE_TILING_OPTIMAL,
  .usage=VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
  .sharingMode=VK_SHARING_MODE_EXCLUSIVE,
  .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED
 };
 VkMemoryRequirements requirements;
 VkMemoryAllocateInfo allocation={.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
 VkImageViewCreateInfo view={.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
 uint32_t type=UINT32_MAX,i;
 VkResult result=VK_ERROR_INITIALIZATION_FAILED;

 if(e==NULL||e->gpu==VK_NULL_HANDLE||e->dev==VK_NULL_HANDLE||
    e->w==0u||e->h==0u){
  ERR("Depth target: invalid device or surface extent");
  return -1;
 }
 if(e->depth_image!=VK_NULL_HANDLE||e->depth_memory!=VK_NULL_HANDLE||
    e->depth_view!=VK_NULL_HANDLE){
  ERR("Depth target: handles were not empty");
  return -1;
 }

 for(i=0u;i<(uint32_t)(sizeof(candidates)/sizeof(candidates[0]));++i){
  vkGetPhysicalDeviceFormatProperties(e->gpu,candidates[i],&properties);
  if((properties.optimalTilingFeatures&
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)!=0u){
   e->depth_format=candidates[i];
   break;
  }
 }
 if(i==(uint32_t)(sizeof(candidates)/sizeof(candidates[0]))){
  ERR("Depth target probe: no supported depth attachment format");
  return -1;
 }

 image_info.format=e->depth_format;
 image_info.extent.width=e->w;
 image_info.extent.height=e->h;
 LOG("Depth target: creating image format=%d extent=%ux%u",
     (int)e->depth_format,e->w,e->h);
 result=vkCreateImage(e->dev,&image_info,NULL,&e->depth_image);
 if(result!=VK_SUCCESS)goto failed;

 vkGetImageMemoryRequirements(e->dev,e->depth_image,&requirements);
 type=memtype(e,requirements.memoryTypeBits,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
 if(type==UINT32_MAX)type=memtype(e,requirements.memoryTypeBits,0u);
 if(type==UINT32_MAX){
  result=VK_ERROR_FEATURE_NOT_PRESENT;
  goto failed;
 }

 allocation.allocationSize=requirements.size;
 allocation.memoryTypeIndex=type;
 result=vkAllocateMemory(e->dev,&allocation,NULL,&e->depth_memory);
 if(result!=VK_SUCCESS)goto failed;

 result=vkBindImageMemory(e->dev,e->depth_image,e->depth_memory,0u);
 if(result!=VK_SUCCESS)goto failed;

 view.image=e->depth_image;
 view.viewType=VK_IMAGE_VIEW_TYPE_2D;
 view.format=e->depth_format;
 view.subresourceRange.aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;
 if(e->depth_format==VK_FORMAT_D32_SFLOAT_S8_UINT||
    e->depth_format==VK_FORMAT_D24_UNORM_S8_UINT){
  view.subresourceRange.aspectMask|=VK_IMAGE_ASPECT_STENCIL_BIT;
 }
 view.subresourceRange.levelCount=1u;
 view.subresourceRange.layerCount=1u;
 result=vkCreateImageView(e->dev,&view,NULL,&e->depth_view);
 if(result!=VK_SUCCESS)goto failed;

 LOG("Depth target ready for render-pass attachment: format=%d extent=%ux%u",
     (int)e->depth_format,e->w,e->h);
 return 0;

failed:
 ERR("Depth target probe failed: VkResult=%d",(int)result);
 if(e->depth_view!=VK_NULL_HANDLE){
  vkDestroyImageView(e->dev,e->depth_view,NULL);
  e->depth_view=VK_NULL_HANDLE;
 }
 if(e->depth_image!=VK_NULL_HANDLE){
  vkDestroyImage(e->dev,e->depth_image,NULL);
  e->depth_image=VK_NULL_HANDLE;
 }
 if(e->depth_memory!=VK_NULL_HANDLE){
  vkFreeMemory(e->dev,e->depth_memory,NULL);
  e->depth_memory=VK_NULL_HANDLE;
 }
 e->depth_format=VK_FORMAT_UNDEFINED;
 return -1;
}


static int create_sea_pipeline(E*e){
 VkShaderModule vs=VK_NULL_HANDLE,fs=VK_NULL_HANDLE;
 VkPushConstantRange range={.stageFlags=VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,.offset=0,.size=(uint32_t)sizeof(TgSeaPushConstants)};
 VkPipelineLayoutCreateInfo layout_ci={.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,.pushConstantRangeCount=1,.pPushConstantRanges=&range};
 VkPipelineShaderStageCreateInfo stages[2];
 VkVertexInputBindingDescription binding={.binding=0,.stride=(uint32_t)sizeof(TgMeshVertex3D),.inputRate=VK_VERTEX_INPUT_RATE_VERTEX};
 VkVertexInputAttributeDescription attribute={.location=0,.binding=0,.format=VK_FORMAT_R32G32B32_SFLOAT,.offset=0};
 VkPipelineVertexInputStateCreateInfo vertex={.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,.vertexBindingDescriptionCount=1,.pVertexBindingDescriptions=&binding,.vertexAttributeDescriptionCount=1,.pVertexAttributeDescriptions=&attribute};
 VkPipelineInputAssemblyStateCreateInfo assembly={.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
 VkPipelineViewportStateCreateInfo viewport={.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,.viewportCount=1,.scissorCount=1};
 VkPipelineRasterizationStateCreateInfo raster={.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,.polygonMode=VK_POLYGON_MODE_FILL,.cullMode=VK_CULL_MODE_NONE,.frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE,.lineWidth=1.0f};
 VkPipelineMultisampleStateCreateInfo ms={.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT};
 VkPipelineColorBlendAttachmentState ba={.blendEnable=VK_FALSE,.colorWriteMask=VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT};
 VkPipelineColorBlendStateCreateInfo blend={.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,.attachmentCount=1,.pAttachments=&ba};
 VkPipelineDepthStencilStateCreateInfo depth={.sType=VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,.depthTestEnable=VK_TRUE,.depthWriteEnable=VK_TRUE,.depthCompareOp=VK_COMPARE_OP_LESS,.depthBoundsTestEnable=VK_FALSE,.stencilTestEnable=VK_FALSE};
 VkDynamicState dyns[]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
 VkPipelineDynamicStateCreateInfo dynamic={.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,.dynamicStateCount=2,.pDynamicStates=dyns};
 VkGraphicsPipelineCreateInfo pi={.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,.stageCount=2,.pStages=stages,.pVertexInputState=&vertex,.pInputAssemblyState=&assembly,.pViewportState=&viewport,.pRasterizationState=&raster,.pMultisampleState=&ms,.pDepthStencilState=&depth,.pColorBlendState=&blend,.pDynamicState=&dynamic,.layout=VK_NULL_HANDLE,.renderPass=e->rp,.subpass=0,.basePipelineIndex=-1};
 VkResult vr=VK_ERROR_INITIALIZATION_FAILED;
 LOG("SEA_STAGE pipeline: loading shaders");
 vs=shader(e,"shaders/sea.vert.spv");fs=shader(e,"shaders/sea.frag.spv");
 if(!vs||!fs)goto done;
 stages[0]=(VkPipelineShaderStageCreateInfo){.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_VERTEX_BIT,.module=vs,.pName="main"};
 stages[1]=(VkPipelineShaderStageCreateInfo){.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_FRAGMENT_BIT,.module=fs,.pName="main"};
 LOG("SEA_STAGE pipeline: creating layout");
 vr=vkCreatePipelineLayout(e->dev,&layout_ci,NULL,&e->sea_layout);
 if(vr!=VK_SUCCESS){ERR("SEA_STAGE pipeline-layout failed: VkResult=%d",(int)vr);goto done;}
 pi.layout=e->sea_layout;
 LOG("SEA_STAGE pipeline: creating graphics pipeline");
 vr=vkCreateGraphicsPipelines(e->dev,VK_NULL_HANDLE,1,&pi,NULL,&e->sea_pipe);
 if(vr!=VK_SUCCESS){ERR("SEA_STAGE graphics-pipeline failed: VkResult=%d",(int)vr);goto done;}
 LOG("SEA_STAGE pipeline: ready (animated waves; depth test/write enabled)");
done:
 if(vs)vkDestroyShaderModule(e->dev,vs,NULL);if(fs)vkDestroyShaderModule(e->dev,fs,NULL);
 if(vr!=VK_SUCCESS){
  if(e->sun_pipe){vkDestroyPipeline(e->dev,e->sun_pipe,NULL);e->sun_pipe=VK_NULL_HANDLE;}
   if(e->sky_pipe){vkDestroyPipeline(e->dev,e->sky_pipe,NULL);e->sky_pipe=VK_NULL_HANDLE;}
   if(e->sky_layout){vkDestroyPipelineLayout(e->dev,e->sky_layout,NULL);e->sky_layout=VK_NULL_HANDLE;}
  if(e->sea_pipe){vkDestroyPipeline(e->dev,e->sea_pipe,NULL);e->sea_pipe=VK_NULL_HANDLE;}
  if(e->sea_layout){vkDestroyPipelineLayout(e->dev,e->sea_layout,NULL);e->sea_layout=VK_NULL_HANDLE;}
  return -1;
 }
 return 0;
}

static int create_sun_pipeline(E*e){
 VkShaderModule vs=VK_NULL_HANDLE,fs=VK_NULL_HANDLE;
 VkPipelineShaderStageCreateInfo stages[2];
 VkVertexInputBindingDescription binding={.binding=0,.stride=(uint32_t)sizeof(V),.inputRate=VK_VERTEX_INPUT_RATE_VERTEX};
 VkVertexInputAttributeDescription attributes[4]={{0,0,VK_FORMAT_R32G32_SFLOAT,0},{1,0,VK_FORMAT_R32G32B32A32_SFLOAT,16},{2,0,VK_FORMAT_R32G32_SFLOAT,8},{3,0,VK_FORMAT_R32_SFLOAT,32}};
 VkPipelineVertexInputStateCreateInfo vertex={.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,.vertexBindingDescriptionCount=1,.pVertexBindingDescriptions=&binding,.vertexAttributeDescriptionCount=4,.pVertexAttributeDescriptions=attributes};
 VkPipelineInputAssemblyStateCreateInfo assembly={.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
 VkPipelineViewportStateCreateInfo viewport={.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,.viewportCount=1,.scissorCount=1};
 VkPipelineRasterizationStateCreateInfo raster={.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,.polygonMode=VK_POLYGON_MODE_FILL,.cullMode=VK_CULL_MODE_NONE,.frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE,.lineWidth=1.0f};
 VkPipelineMultisampleStateCreateInfo ms={.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT};
 VkPipelineDepthStencilStateCreateInfo depth={.sType=VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,.depthTestEnable=VK_TRUE,.depthWriteEnable=VK_FALSE,.depthCompareOp=VK_COMPARE_OP_LESS_OR_EQUAL,.depthBoundsTestEnable=VK_FALSE,.stencilTestEnable=VK_FALSE};
 VkPipelineColorBlendAttachmentState ba={.blendEnable=VK_TRUE,.srcColorBlendFactor=VK_BLEND_FACTOR_SRC_ALPHA,.dstColorBlendFactor=VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,.colorBlendOp=VK_BLEND_OP_ADD,.srcAlphaBlendFactor=VK_BLEND_FACTOR_ONE,.dstAlphaBlendFactor=VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,.alphaBlendOp=VK_BLEND_OP_ADD,.colorWriteMask=15};
 VkPipelineColorBlendStateCreateInfo blend={.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,.attachmentCount=1,.pAttachments=&ba};
 VkDynamicState dyns[]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
 VkPipelineDynamicStateCreateInfo dynamic={.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,.dynamicStateCount=2,.pDynamicStates=dyns};
 VkGraphicsPipelineCreateInfo pi={.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,.stageCount=2,.pStages=stages,.pVertexInputState=&vertex,.pInputAssemblyState=&assembly,.pViewportState=&viewport,.pRasterizationState=&raster,.pMultisampleState=&ms,.pDepthStencilState=&depth,.pColorBlendState=&blend,.pDynamicState=&dynamic,.layout=e->layout,.renderPass=e->rp,.subpass=0,.basePipelineIndex=-1};
 VkResult vr=VK_ERROR_INITIALIZATION_FAILED;
 vs=shader(e,"shaders/sun.vert.spv");fs=shader(e,"shaders/sun.frag.spv");
 if(!vs||!fs)goto done;
 stages[0]=(VkPipelineShaderStageCreateInfo){.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_VERTEX_BIT,.module=vs,.pName="main"};
 stages[1]=(VkPipelineShaderStageCreateInfo){.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_FRAGMENT_BIT,.module=fs,.pName="main"};
 vr=vkCreateGraphicsPipelines(e->dev,VK_NULL_HANDLE,1,&pi,NULL,&e->sun_pipe);
 if(vr!=VK_SUCCESS)ERR("SEA_STAGE sun depth-pipeline failed: VkResult=%d",(int)vr);
done:
 if(vs)vkDestroyShaderModule(e->dev,vs,NULL);
 if(fs)vkDestroyShaderModule(e->dev,fs,NULL);
 if(vr!=VK_SUCCESS){if(e->sun_pipe){vkDestroyPipeline(e->dev,e->sun_pipe,NULL);e->sun_pipe=VK_NULL_HANDLE;}return -1;}
 LOG("SEA_STAGE sun pipeline ready: depth-tested celestial disc");
 return 0;
}

static int create_sky_pipeline(E*e){
 VkShaderModule vs=VK_NULL_HANDLE,fs=VK_NULL_HANDLE;VkResult vr=VK_ERROR_INITIALIZATION_FAILED;VkPipelineShaderStageCreateInfo st[2];
 VkVertexInputBindingDescription bd={.binding=0,.stride=(uint32_t)sizeof(V),.inputRate=VK_VERTEX_INPUT_RATE_VERTEX};
 VkVertexInputAttributeDescription at[4]={{0,0,VK_FORMAT_R32G32_SFLOAT,0},{1,0,VK_FORMAT_R32G32B32A32_SFLOAT,16},{2,0,VK_FORMAT_R32G32_SFLOAT,8},{3,0,VK_FORMAT_R32_SFLOAT,32}};
 VkPipelineVertexInputStateCreateInfo vx={.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,.vertexBindingDescriptionCount=1,.pVertexBindingDescriptions=&bd,.vertexAttributeDescriptionCount=4,.pVertexAttributeDescriptions=at};
 VkPipelineInputAssemblyStateCreateInfo ia={.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
 VkPipelineViewportStateCreateInfo vp={.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,.viewportCount=1,.scissorCount=1};
 VkPipelineRasterizationStateCreateInfo rs={.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,.polygonMode=VK_POLYGON_MODE_FILL,.cullMode=VK_CULL_MODE_NONE,.lineWidth=1};
 VkPipelineMultisampleStateCreateInfo ms={.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT};
 VkPipelineDepthStencilStateCreateInfo ds={.sType=VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,.depthTestEnable=VK_FALSE,.depthWriteEnable=VK_FALSE,.depthCompareOp=VK_COMPARE_OP_ALWAYS,.depthBoundsTestEnable=VK_FALSE,.stencilTestEnable=VK_FALSE};
 VkPipelineColorBlendAttachmentState ba={.blendEnable=VK_FALSE,.colorWriteMask=15};VkPipelineColorBlendStateCreateInfo cb={.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,.attachmentCount=1,.pAttachments=&ba};
 VkDynamicState dyn[]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};VkPipelineDynamicStateCreateInfo dy={.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,.dynamicStateCount=2,.pDynamicStates=dyn};
 VkPushConstantRange range={.stageFlags=VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,.offset=0,.size=112};
 VkPipelineLayoutCreateInfo li={.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,.pushConstantRangeCount=1,.pPushConstantRanges=&range};
 VkGraphicsPipelineCreateInfo pi={.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,.stageCount=2,.pStages=st,.pVertexInputState=&vx,.pInputAssemblyState=&ia,.pViewportState=&vp,.pRasterizationState=&rs,.pMultisampleState=&ms,.pDepthStencilState=&ds,.pColorBlendState=&cb,.pDynamicState=&dy,.renderPass=e->rp,.subpass=0,.basePipelineIndex=-1};
 vs=shader(e,"shaders/sky.vert.spv");fs=shader(e,"shaders/sky.frag.spv");if(!vs||!fs)goto done;
 st[0]=(VkPipelineShaderStageCreateInfo){.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_VERTEX_BIT,.module=vs,.pName="main"};
 st[1]=(VkPipelineShaderStageCreateInfo){.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_FRAGMENT_BIT,.module=fs,.pName="main"};
 vr=vkCreatePipelineLayout(e->dev,&li,NULL,&e->sky_layout);if(vr!=VK_SUCCESS)goto done;pi.layout=e->sky_layout;
 vr=vkCreateGraphicsPipelines(e->dev,VK_NULL_HANDLE,1,&pi,NULL,&e->sky_pipe);
 done:if(vs)vkDestroyShaderModule(e->dev,vs,NULL);if(fs)vkDestroyShaderModule(e->dev,fs,NULL);
 if(vr!=VK_SUCCESS){ERR("SKY pipeline failed: VkResult=%d",(int)vr);if(e->sky_pipe)vkDestroyPipeline(e->dev,e->sky_pipe,NULL);if(e->sky_layout)vkDestroyPipelineLayout(e->dev,e->sky_layout,NULL);e->sky_pipe=VK_NULL_HANDLE;e->sky_layout=VK_NULL_HANDLE;return -1;}
 LOG("SKY volumetric cloud pipeline ready");return 0;
}

static int initialize_sea_renderer(E*e){
 VkResult upload_result=VK_ERROR_INITIALIZATION_FAILED;const TgGpuMeshVk*mesh;
 if(!e||e->dev==VK_NULL_HANDLE||e->q==VK_NULL_HANDLE||e->rp==VK_NULL_HANDLE||e->w==0u||e->h==0u){
  ERR("SEA_STAGE activation rejected: incomplete Vulkan context");return -1;
 }
 if(e->depth_view==VK_NULL_HANDLE||e->depth_format==VK_FORMAT_UNDEFINED){
  ERR("SEA_STAGE activation rejected: render-pass depth attachment is unavailable");return -1;
 }
 LOG("SEA_STAGE depth attachment ready; horizon occlusion enabled");
 LOG("SEA_STAGE mesh-upload begin; ocean grid=256x256 cells; extent=1200x1200 m");
 if(tg_sea_test_scene_create(e->gpu,e->dev,e->q,e->qf,&e->sea_scene,&upload_result)!=0){
  ERR("SEA_STAGE mesh-upload failed: VkResult=%d",(int)upload_result);goto failed;
 }
 mesh=tg_sea_test_scene_gpu_mesh(e->sea_scene);
 if(!mesh||tg_gpu_mesh_vk_buffer(mesh)==VK_NULL_HANDLE||tg_gpu_mesh_vk_index_count(mesh)==0u){
  ERR("SEA_STAGE mesh-upload returned invalid GPU handles");goto failed;
 }
 LOG("SEA_STAGE mesh-upload ready; indices=%u path=%s memory-flags=0x%x",
     tg_gpu_mesh_vk_index_count(mesh),tg_gpu_mesh_vk_used_staging(mesh)==VK_TRUE?"staging":"direct",
     (unsigned)tg_gpu_mesh_vk_memory_properties(mesh));
 LOG("SEA_STAGE camera initialization begin");
 e->sea_yaw=0.0f;
 e->sea_pitch=-0.025f;
 tg_weather_init(&e->weather);
 LOG("WEATHER 09:00 preset: wind %.1f m/s, clouds %.2f, rain %.1f mm/h",e->weather.wind_speed_mps,e->weather.cloud_coverage,e->weather.precipitation_mm_per_hour);
 {
  TgVec3 camera_position=tg_vec3(0.0f,7.0f,35.0f);
  TgVec3 forward=tg_sea_camera_forward(e->sea_yaw,e->sea_pitch);
  TgVec3 target=tg_vec3(camera_position.x+forward.x,
                        camera_position.y+forward.y,
                        camera_position.z+forward.z);
  if(tg_camera3d_init(&e->sea_camera,camera_position,target,
                      tg_vec3(0.0f,1.0f,0.0f),0.92f,
                      (float)e->w/(float)e->h,0.1f,2500.0f)!=0){
   ERR("SEA_STAGE camera initialization failed");goto failed;
  }
  e->sea_touch_mode=0;e->sea_pointer_a=-1;e->sea_pointer_b=-1;
 }
 if(create_sea_pipeline(e)!=0)goto failed;
 if(create_sky_pipeline(e)!=0)goto failed;
 e->sea_renderer_ready=1;
 LOG("SEA_STAGE READY: animated procedural water surface is enabled for BUILDING screen");
 return 0;
failed:
 if(e->sea_pipe){vkDestroyPipeline(e->dev,e->sea_pipe,NULL);e->sea_pipe=VK_NULL_HANDLE;}
 if(e->sea_layout){vkDestroyPipelineLayout(e->dev,e->sea_layout,NULL);e->sea_layout=VK_NULL_HANDLE;}
 if(e->sea_scene){tg_sea_test_scene_destroy(e->dev,e->sea_scene);e->sea_scene=NULL;}
 e->sea_renderer_ready=0;return -1;
}

static int setup(E*e){
 const char*ie[]={VK_KHR_SURFACE_EXTENSION_NAME,VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};VkApplicationInfo ai={.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO,.pApplicationName="TORNIA_GULD",.apiVersion=VK_API_VERSION_1_0};
 VkInstanceCreateInfo ii={.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,.pApplicationInfo=&ai,.enabledExtensionCount=2,.ppEnabledExtensionNames=ie};if(vkCreateInstance(&ii,0,&e->in))return -1;
 VkAndroidSurfaceCreateInfoKHR si={.sType=VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,.window=e->app->window};if(vkCreateAndroidSurfaceKHR(e->in,&si,0,&e->surface))return -1;
 uint32_t nd=0;vkEnumeratePhysicalDevices(e->in,&nd,0);VkPhysicalDevice*ds=malloc(nd*sizeof(*ds));vkEnumeratePhysicalDevices(e->in,&nd,ds);
 for(uint32_t d=0;d<nd&&!e->gpu;d++){uint32_t nq=0;vkGetPhysicalDeviceQueueFamilyProperties(ds[d],&nq,0);VkQueueFamilyProperties*qs=malloc(nq*sizeof(*qs));vkGetPhysicalDeviceQueueFamilyProperties(ds[d],&nq,qs);for(uint32_t q=0;q<nq;q++){VkBool32 p=0;vkGetPhysicalDeviceSurfaceSupportKHR(ds[d],q,e->surface,&p);if(p&&(qs[q].queueFlags&VK_QUEUE_GRAPHICS_BIT)){e->gpu=ds[d];e->qf=q;break;}}free(qs);}free(ds);if(!e->gpu)return -1;
 float pr=1;VkDeviceQueueCreateInfo qi={.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,.queueFamilyIndex=e->qf,.queueCount=1,.pQueuePriorities=&pr};const char*de[]={VK_KHR_SWAPCHAIN_EXTENSION_NAME};VkDeviceCreateInfo di={.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,.queueCreateInfoCount=1,.pQueueCreateInfos=&qi,.enabledExtensionCount=1,.ppEnabledExtensionNames=de};if(vkCreateDevice(e->gpu,&di,0,&e->dev))return -1;vkGetDeviceQueue(e->dev,e->qf,0,&e->q);
 VkSurfaceCapabilitiesKHR cp;vkGetPhysicalDeviceSurfaceCapabilitiesKHR(e->gpu,e->surface,&cp);uint32_t nf=0;vkGetPhysicalDeviceSurfaceFormatsKHR(e->gpu,e->surface,&nf,0);VkSurfaceFormatKHR*fs=malloc(nf*sizeof(*fs));vkGetPhysicalDeviceSurfaceFormatsKHR(e->gpu,e->surface,&nf,fs);VkSurfaceFormatKHR sf=fs[0];for(uint32_t i=0;i<nf;i++)if(fs[i].format==VK_FORMAT_R8G8B8A8_UNORM||fs[i].format==VK_FORMAT_B8G8R8A8_UNORM){sf=fs[i];break;}free(fs);e->fmt=sf.format;
 VkSurfaceTransformFlagBitsKHR chosenTransform=(cp.supportedTransforms&VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)?VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR:cp.currentTransform;
 e->w=cp.currentExtent.width;e->h=cp.currentExtent.height;
 LOG("Surface extent=%ux%u native=%dx%d currentTransform=0x%x chosenTransform=0x%x supported=0x%x",e->w,e->h,ANativeWindow_getWidth(e->app->window),ANativeWindow_getHeight(e->app->window),cp.currentTransform,chosenTransform,cp.supportedTransforms);
 uint32_t ni=cp.minImageCount+1;if(cp.maxImageCount&&ni>cp.maxImageCount)ni=cp.maxImageCount;VkSwapchainCreateInfoKHR sw={.sType=VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,.surface=e->surface,.minImageCount=ni,.imageFormat=e->fmt,.imageColorSpace=sf.colorSpace,.imageExtent=cp.currentExtent,.imageArrayLayers=1,.imageUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE,.preTransform=chosenTransform,.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,.presentMode=VK_PRESENT_MODE_FIFO_KHR,.clipped=VK_TRUE};if(vkCreateSwapchainKHR(e->dev,&sw,0,&e->sw))return -1;
 vkGetSwapchainImagesKHR(e->dev,e->sw,&e->n,0);e->imgs=malloc(e->n*sizeof(VkImage));vkGetSwapchainImagesKHR(e->dev,e->sw,&e->n,e->imgs);e->views=calloc(e->n,sizeof(VkImageView));e->fb=calloc(e->n,sizeof(VkFramebuffer));e->cmd=calloc(e->n,sizeof(VkCommandBuffer));
 for(uint32_t i=0;i<e->n;i++){VkImageViewCreateInfo vi={.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,.image=e->imgs[i],.viewType=VK_IMAGE_VIEW_TYPE_2D,.format=e->fmt,.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};if(vkCreateImageView(e->dev,&vi,0,&e->views[i]))return -1;}
 if(create_depth_target(e)!=0){ERR("Vulkan depth attachment creation failed");return -1;}
 VkAttachmentDescription ad[2]={
  {.format=e->fmt,.samples=VK_SAMPLE_COUNT_1_BIT,.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR,.storeOp=VK_ATTACHMENT_STORE_OP_STORE,.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE,.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE,.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
  {.format=e->depth_format,.samples=VK_SAMPLE_COUNT_1_BIT,.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR,.storeOp=VK_ATTACHMENT_STORE_OP_DONT_CARE,.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE,.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE,.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,.finalLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}
 };
 VkAttachmentReference ar={0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
 VkAttachmentReference dr={1,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
 VkSubpassDescription sub={.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS,.colorAttachmentCount=1,.pColorAttachments=&ar,.pDepthStencilAttachment=&dr};
 VkRenderPassCreateInfo ri={.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,.attachmentCount=2,.pAttachments=ad,.subpassCount=1,.pSubpasses=&sub};
 if(vkCreateRenderPass(e->dev,&ri,0,&e->rp))return -1;
 VkDescriptorSetLayoutBinding db[2]={{.binding=0,.descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,.descriptorCount=1,.stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT},{.binding=1,.descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,.descriptorCount=1,.stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT}};VkDescriptorSetLayoutCreateInfo dl={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,.bindingCount=2,.pBindings=db};if(vkCreateDescriptorSetLayout(e->dev,&dl,0,&e->descriptor_layout))return -1;
 VkShaderModule vs=shader(e,"shaders/menu.vert.spv"),fsmod=shader(e,"shaders/menu.frag.spv");if(!vs||!fsmod)return -1;
 VkPipelineShaderStageCreateInfo st[2]={{.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_VERTEX_BIT,.module=vs,.pName="main"},{.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_FRAGMENT_BIT,.module=fsmod,.pName="main"}};
 VkVertexInputBindingDescription bd={0,sizeof(V),VK_VERTEX_INPUT_RATE_VERTEX};VkVertexInputAttributeDescription at[4]={{0,0,VK_FORMAT_R32G32_SFLOAT,0},{1,0,VK_FORMAT_R32G32B32A32_SFLOAT,16},{2,0,VK_FORMAT_R32G32_SFLOAT,8},{3,0,VK_FORMAT_R32_SFLOAT,32}};VkPipelineVertexInputStateCreateInfo vx={.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,.vertexBindingDescriptionCount=1,.pVertexBindingDescriptions=&bd,.vertexAttributeDescriptionCount=4,.pVertexAttributeDescriptions=at};VkPipelineInputAssemblyStateCreateInfo ia={.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};VkPipelineViewportStateCreateInfo vp={.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,.viewportCount=1,.scissorCount=1};VkPipelineRasterizationStateCreateInfo rs={.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,.polygonMode=VK_POLYGON_MODE_FILL,.cullMode=VK_CULL_MODE_NONE,.lineWidth=1};VkPipelineMultisampleStateCreateInfo ms={.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT};VkPipelineDepthStencilStateCreateInfo menu_depth={.sType=VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,.depthTestEnable=VK_FALSE,.depthWriteEnable=VK_FALSE,.depthCompareOp=VK_COMPARE_OP_ALWAYS,.depthBoundsTestEnable=VK_FALSE,.stencilTestEnable=VK_FALSE};VkPipelineColorBlendAttachmentState ba={.blendEnable=VK_TRUE,.srcColorBlendFactor=VK_BLEND_FACTOR_SRC_ALPHA,.dstColorBlendFactor=VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,.colorBlendOp=VK_BLEND_OP_ADD,.srcAlphaBlendFactor=VK_BLEND_FACTOR_ONE,.dstAlphaBlendFactor=VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,.alphaBlendOp=VK_BLEND_OP_ADD,.colorWriteMask=15};VkPipelineColorBlendStateCreateInfo cb={.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,.attachmentCount=1,.pAttachments=&ba};VkDynamicState dyns[]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};VkPipelineDynamicStateCreateInfo dy={.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,.dynamicStateCount=2,.pDynamicStates=dyns};VkPipelineLayoutCreateInfo li={.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,.setLayoutCount=1,.pSetLayouts=&e->descriptor_layout};if(vkCreatePipelineLayout(e->dev,&li,0,&e->layout))return -1;VkGraphicsPipelineCreateInfo pi={.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,.stageCount=2,.pStages=st,.pVertexInputState=&vx,.pInputAssemblyState=&ia,.pViewportState=&vp,.pRasterizationState=&rs,.pMultisampleState=&ms,.pDepthStencilState=&menu_depth,.pColorBlendState=&cb,.pDynamicState=&dy,.layout=e->layout,.renderPass=e->rp};LOG("MAIN_MENU pipeline: creating with depth test disabled");VkResult pkr=vkCreateGraphicsPipelines(e->dev,0,1,&pi,0,&e->pipe);if(pkr!=VK_SUCCESS)ERR("MAIN_MENU graphics-pipeline failed: VkResult=%d",(int)pkr);else LOG("MAIN_MENU graphics-pipeline ready");vkDestroyShaderModule(e->dev,vs,0);vkDestroyShaderModule(e->dev,fsmod,0);if(pkr)return -1;
 if(create_sun_pipeline(e)!=0)return -1;
 for(uint32_t i=0;i<e->n;i++){
  VkImageView attachments[2]={e->views[i],e->depth_view};
  VkFramebufferCreateInfo fi={.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,.renderPass=e->rp,.attachmentCount=2,.pAttachments=attachments,.width=e->w,.height=e->h,.layers=1};
  if(vkCreateFramebuffer(e->dev,&fi,0,&e->fb[i]))return -1;
 }
 VkCommandPoolCreateInfo pci={.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,.queueFamilyIndex=e->qf};if(vkCreateCommandPool(e->dev,&pci,0,&e->pool))return -1;VkCommandBufferAllocateInfo cai={.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,.commandPool=e->pool,.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY,.commandBufferCount=e->n};if(vkAllocateCommandBuffers(e->dev,&cai,e->cmd))return -1;
 VkBufferCreateInfo bi={.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,.size=sizeof(V)*MAXV,.usage=VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,.sharingMode=VK_SHARING_MODE_EXCLUSIVE};if(vkCreateBuffer(e->dev,&bi,0,&e->vb))return -1;VkMemoryRequirements mr;vkGetBufferMemoryRequirements(e->dev,e->vb,&mr);uint32_t mt=memtype(e,mr.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);if(mt==UINT32_MAX)return -1;VkMemoryAllocateInfo mai={.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,.allocationSize=mr.size,.memoryTypeIndex=mt};if(vkAllocateMemory(e->dev,&mai,0,&e->vm))return -1;vkBindBufferMemory(e->dev,e->vb,e->vm,0);
 /* Experimental 2D texture assets are intentionally excluded from engine startup. */
 VkSemaphoreCreateInfo sem={.sType=VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};vkCreateSemaphore(e->dev,&sem,0,&e->semA);vkCreateSemaphore(e->dev,&sem,0,&e->semB);VkFenceCreateInfo fe={.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,.flags=VK_FENCE_CREATE_SIGNALED_BIT};vkCreateFence(e->dev,&fe,0,&e->fence);e->joystick_pointer_id=-1;e->camera_pointer_id=-1;e->ready=1;LOG("Vulkan main menu ready %ux%u; 2D world and character deferred",e->w,e->h);return 0;
}
static void frame(E*e){
 if(!e->ready)return;
 uint32_t delta_ms=0;struct timespec now;
 if(clock_gettime(CLOCK_MONOTONIC,&now)==0){
  uint64_t ns=(uint64_t)now.tv_sec*1000000000ull+(uint64_t)now.tv_nsec;
  if(e->last_frame_ns){uint64_t delta=(ns-e->last_frame_ns)/1000000ull;if(delta>100u)delta=100u;delta_ms=(uint32_t)delta;}
  e->last_frame_ns=ns;
 }
 if(e->scene==2&&e->sea_renderer_ready){float dt=(float)delta_ms/1000.0f;e->sea_time_seconds+=dt;tg_weather_update(&e->weather,dt);}
 if(e->scene==1){
  if(!e->world_resources_ready&&initialize_2d_world(e)!=0){e->scene=2;ERR("2D world initialization failed; returning to construction screen");}
  if(e->scene==1)player_update(&e->player,delta_ms,(float)WORLD_WIDTH*WORLD_TILE_SIZE,(float)WORLD_HEIGHT*WORLD_TILE_SIZE);
  if(e->sprite_ready){
   const char*clip=player_animation_name(&e->player);
   if(animation_controller_update(&e->player_animation,&e->animations,clip,player_animation_flip_x(&e->player),delta_ms)!=0)
    ERR("Unable to select player animation: %s",clip);
  }
  if(e->camera_follow_enabled&&delta_ms)
   world_camera_follow(&e->camera,e->player.x,e->player.y,.5f,.55f,(float)e->w,(float)e->h,(float)delta_ms/1000.0f,8.0f);
 }
 VkResult vr=vkWaitForFences(e->dev,1,&e->fence,VK_TRUE,UINT64_MAX);
 if(vr!=VK_SUCCESS){ERR("FRAME wait-fence failed: VkResult=%d",(int)vr);e->ready=0;return;}
 if(e->scene==2&&!e->sea_renderer_attempted){
  e->sea_renderer_attempted=1;LOG("SEA_STAGE activation requested from BUILDING screen");
  if(initialize_sea_renderer(e)!=0)ERR("SEA_STAGE activation failed; retaining safe 2D BUILDING screen");
 }
 uint32_t ix=0;vr=vkAcquireNextImageKHR(e->dev,e->sw,UINT64_MAX,e->semA,VK_NULL_HANDLE,&ix);
 if(vr!=VK_SUCCESS&&vr!=VK_SUBOPTIMAL_KHR){ERR("FRAME acquire-image failed: VkResult=%d",(int)vr);return;}
 if(e->scene==1)draw_world(e);else if(e->scene==2)draw_building(e);else menu(e);
 void*mapped=NULL;vr=vkMapMemory(e->dev,e->vm,0,e->nv*sizeof(V),0,&mapped);
 if(vr!=VK_SUCCESS){ERR("FRAME vertex-map failed: VkResult=%d",(int)vr);return;}
 memcpy(mapped,e->v,e->nv*sizeof(V));vkUnmapMemory(e->dev,e->vm);
 VkCommandBuffer cb=e->cmd[ix];vr=vkResetCommandBuffer(cb,0);
 if(vr!=VK_SUCCESS){ERR("FRAME reset-command-buffer failed: VkResult=%d",(int)vr);return;}
 VkCommandBufferBeginInfo begin={.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
 vr=vkBeginCommandBuffer(cb,&begin);if(vr!=VK_SUCCESS){ERR("FRAME begin-command-buffer failed: VkResult=%d",(int)vr);return;}
 VkClearValue clear[2]={0};
 clear[0].color=(VkClearColorValue){{.035f,.052f,.07f,1.0f}};
 clear[1].depthStencil.depth=1.0f;clear[1].depthStencil.stencil=0u;
 VkRenderPassBeginInfo rp={.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,.renderPass=e->rp,.framebuffer=e->fb[ix],
  .renderArea={{0,0},{e->w,e->h}},.clearValueCount=2,.pClearValues=clear};
 vkCmdBeginRenderPass(cb,&rp,VK_SUBPASS_CONTENTS_INLINE);
 VkViewport viewport={0.0f,0.0f,(float)e->w,(float)e->h,0.0f,1.0f};VkRect2D scissor={{0,0},{e->w,e->h}};
 vkCmdSetViewport(cb,0,1,&viewport);vkCmdSetScissor(cb,0,1,&scissor);
 if(e->scene==2&&e->sky_nv>0u){
   VkDeviceSize so=0;vkCmdBindVertexBuffers(cb,0,1,&e->vb,&so);
   if(e->sky_pipe!=VK_NULL_HANDLE){
    TgSkyPushConstants p;float cp=cosf(e->sea_pitch),sp=sinf(e->sea_pitch);
    p.camera_forward[0]=sinf(e->sea_yaw)*cp;p.camera_forward[1]=sp;p.camera_forward[2]=-cosf(e->sea_yaw)*cp;p.camera_forward[3]=tanf(e->sea_camera.vertical_fov_radians*.5f);
    p.camera_right[0]=cosf(e->sea_yaw);p.camera_right[1]=0;p.camera_right[2]=sinf(e->sea_yaw);p.camera_right[3]=(float)e->w/(float)e->h;
    p.camera_up[0]=-sinf(e->sea_yaw)*sp;p.camera_up[1]=cp;p.camera_up[2]=cosf(e->sea_yaw)*sp;p.camera_up[3]=0;
    for(int i=0;i<3;i++)p.sun_direction[i]=e->weather.sun_direction[i];p.sun_direction[3]=0;
    p.weather[0]=e->weather.elapsed_seconds;p.weather[1]=e->weather.wind_speed_mps;p.weather[2]=e->weather.wind_direction_radians;p.weather[3]=e->weather.cloud_coverage;
    p.cloud[0]=e->weather.cloud_base_height_m;p.cloud[1]=e->weather.cloud_top_height_m;p.cloud[2]=e->weather.precipitation_mm_per_hour;p.cloud[3]=e->weather.time_of_day_hours;
    p.camera_position[0]=e->sea_camera.position.x;p.camera_position[1]=e->sea_camera.position.y;p.camera_position[2]=e->sea_camera.position.z;p.camera_position[3]=1;
    vkCmdBindPipeline(cb,VK_PIPELINE_BIND_POINT_GRAPHICS,e->sky_pipe);
    vkCmdPushConstants(cb,e->sky_layout,VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,0,(uint32_t)sizeof(p),&p);
   }else vkCmdBindPipeline(cb,VK_PIPELINE_BIND_POINT_GRAPHICS,e->pipe);
   vkCmdDraw(cb,e->sky_nv,1,0,0);
  }
 if(e->scene==2&&e->sea_renderer_ready&&e->sea_scene){
  const TgGpuMeshVk*mesh=tg_sea_test_scene_gpu_mesh(e->sea_scene);
  if(mesh){
   VkBuffer buffer=tg_gpu_mesh_vk_buffer(mesh);VkDeviceSize vertex_offset=tg_gpu_mesh_vk_vertex_offset(mesh);
   if(buffer!=VK_NULL_HANDLE){
    vkCmdBindPipeline(cb,VK_PIPELINE_BIND_POINT_GRAPHICS,e->sea_pipe);
    TgSeaPushConstants sea_push;
    sea_push.view_projection=e->sea_camera.view_projection;
    sea_push.time_seconds=e->weather.elapsed_seconds;
     sea_push.padding[0]=e->weather.wind_speed_mps;sea_push.padding[1]=e->weather.wind_direction_radians;sea_push.padding[2]=0.0f;
    sea_push.camera_position[0]=e->sea_camera.position.x;
    sea_push.camera_position[1]=e->sea_camera.position.y;
    sea_push.camera_position[2]=e->sea_camera.position.z;
    sea_push.camera_position[3]=1.0f;
    /* Shared 09:00 solar direction for sky, sun disc and ocean. */
     sea_push.sun_direction[0]=e->weather.sun_direction[0];sea_push.sun_direction[1]=e->weather.sun_direction[1];sea_push.sun_direction[2]=e->weather.sun_direction[2];
    sea_push.sun_direction[3]=0.0f;
    vkCmdPushConstants(cb,e->sea_layout,
        VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,
        0,(uint32_t)sizeof(sea_push),&sea_push);
    vkCmdBindVertexBuffers(cb,0,1,&buffer,&vertex_offset);
    vkCmdBindIndexBuffer(cb,buffer,tg_gpu_mesh_vk_index_offset(mesh),VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cb,tg_gpu_mesh_vk_index_count(mesh),1,0,0,0);
    if(!e->sea_draw_logged){LOG("SEA_STAGE first indexed draw recorded; indices=%u",tg_gpu_mesh_vk_index_count(mesh));e->sea_draw_logged=1;}
   }else ERR("SEA_STAGE draw skipped: GPU buffer is null");
  }else ERR("SEA_STAGE draw skipped: scene mesh is null");
 }
 if(e->scene==2&&e->sun_nv>0u&&e->sun_pipe!=VK_NULL_HANDLE){
  vkCmdBindPipeline(cb,VK_PIPELINE_BIND_POINT_GRAPHICS,e->sun_pipe);
  VkDeviceSize sun_offset=0;
  vkCmdBindVertexBuffers(cb,0,1,&e->vb,&sun_offset);
  vkCmdDraw(cb,e->sun_nv,1,e->sun_start,0);
 }
 vkCmdBindPipeline(cb,VK_PIPELINE_BIND_POINT_GRAPHICS,e->pipe);VkDeviceSize offset=0;
 vkCmdBindVertexBuffers(cb,0,1,&e->vb,&offset);
 if(e->scene==2&&e->sky_nv>0u){
  uint32_t overlay_start=e->sun_start+e->sun_nv;
  uint32_t overlay_count=e->nv-overlay_start;
  if(overlay_count)vkCmdDraw(cb,overlay_count,1,overlay_start,0);
 }else{
  vkCmdDraw(cb,e->nv,1,0,0);
 }
 vkCmdEndRenderPass(cb);vr=vkEndCommandBuffer(cb);
 if(vr!=VK_SUCCESS){ERR("FRAME end-command-buffer failed: VkResult=%d",(int)vr);return;}
 vr=vkResetFences(e->dev,1,&e->fence);
 if(vr!=VK_SUCCESS){ERR("FRAME reset-fence failed: VkResult=%d",(int)vr);e->ready=0;return;}
 VkPipelineStageFlags wait_stage=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
 VkSubmitInfo submit={.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,.waitSemaphoreCount=1,.pWaitSemaphores=&e->semA,
  .pWaitDstStageMask=&wait_stage,.commandBufferCount=1,.pCommandBuffers=&cb,.signalSemaphoreCount=1,.pSignalSemaphores=&e->semB};
 vr=vkQueueSubmit(e->q,1,&submit,e->fence);
 if(vr!=VK_SUCCESS){ERR("FRAME queue-submit failed: VkResult=%d",(int)vr);e->ready=0;return;}
 VkPresentInfoKHR present={.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,.waitSemaphoreCount=1,.pWaitSemaphores=&e->semB,
  .swapchainCount=1,.pSwapchains=&e->sw,.pImageIndices=&ix};
 vr=vkQueuePresentKHR(e->q,&present);
 if(vr!=VK_SUCCESS&&vr!=VK_SUBOPTIMAL_KHR&&vr!=VK_ERROR_OUT_OF_DATE_KHR)
  ERR("FRAME queue-present failed: VkResult=%d",(int)vr);
}
static void closevk(E*e){
 if(e->dev)vkDeviceWaitIdle(e->dev);
 if(e->dev){
  if(e->sun_pipe){vkDestroyPipeline(e->dev,e->sun_pipe,NULL);e->sun_pipe=VK_NULL_HANDLE;}
  if(e->sky_pipe){vkDestroyPipeline(e->dev,e->sky_pipe,NULL);e->sky_pipe=VK_NULL_HANDLE;}
  if(e->sky_layout){vkDestroyPipelineLayout(e->dev,e->sky_layout,NULL);e->sky_layout=VK_NULL_HANDLE;}
  if(e->sea_pipe){vkDestroyPipeline(e->dev,e->sea_pipe,NULL);e->sea_pipe=VK_NULL_HANDLE;}
  if(e->sea_layout){vkDestroyPipelineLayout(e->dev,e->sea_layout,NULL);e->sea_layout=VK_NULL_HANDLE;}
  if(e->sea_scene){tg_sea_test_scene_destroy(e->dev,e->sea_scene);e->sea_scene=NULL;}
  if(e->fence)vkDestroyFence(e->dev,e->fence,0);
  if(e->sprite_sampler)vkDestroySampler(e->dev,e->sprite_sampler,0);
  if(e->sprite_view)vkDestroyImageView(e->dev,e->sprite_view,0);
  if(e->sprite_image)vkDestroyImage(e->dev,e->sprite_image,0);
  if(e->sprite_memory)vkFreeMemory(e->dev,e->sprite_memory,0);
  if(e->descriptor_pool)vkDestroyDescriptorPool(e->dev,e->descriptor_pool,0);
  if(e->atlas_sampler)vkDestroySampler(e->dev,e->atlas_sampler,0);
  if(e->atlas_view)vkDestroyImageView(e->dev,e->atlas_view,0);
  if(e->atlas)vkDestroyImage(e->dev,e->atlas,0);
  if(e->atlas_mem)vkFreeMemory(e->dev,e->atlas_mem,0);
  if(e->descriptor_layout)vkDestroyDescriptorSetLayout(e->dev,e->descriptor_layout,0);
  if(e->semA)vkDestroySemaphore(e->dev,e->semA,0);
  if(e->semB)vkDestroySemaphore(e->dev,e->semB,0);
  if(e->vb)vkDestroyBuffer(e->dev,e->vb,0);
  if(e->vm)vkFreeMemory(e->dev,e->vm,0);
  if(e->pool)vkDestroyCommandPool(e->dev,e->pool,0);
  for(uint32_t i=0;i<e->n;i++){if(e->fb&&e->fb[i])vkDestroyFramebuffer(e->dev,e->fb[i],0);if(e->views&&e->views[i])vkDestroyImageView(e->dev,e->views[i],0);}
  if(e->depth_view){vkDestroyImageView(e->dev,e->depth_view,0);e->depth_view=VK_NULL_HANDLE;}
  if(e->depth_image){vkDestroyImage(e->dev,e->depth_image,0);e->depth_image=VK_NULL_HANDLE;}
  if(e->depth_memory){vkFreeMemory(e->dev,e->depth_memory,0);e->depth_memory=VK_NULL_HANDLE;}
  e->depth_format=VK_FORMAT_UNDEFINED;
  if(e->pipe)vkDestroyPipeline(e->dev,e->pipe,0);
  if(e->layout)vkDestroyPipelineLayout(e->dev,e->layout,0);
  if(e->rp)vkDestroyRenderPass(e->dev,e->rp,0);
  if(e->sw)vkDestroySwapchainKHR(e->dev,e->sw,0);
  vkDestroyDevice(e->dev,0);
 }
 if(e->surface&&e->in)vkDestroySurfaceKHR(e->in,e->surface,0);
 if(e->in)vkDestroyInstance(e->in,0);
 free(e->imgs);free(e->views);free(e->fb);free(e->cmd);
 e->imgs=NULL;e->views=NULL;e->fb=NULL;e->cmd=NULL;e->n=0u;
 e->ready=0;e->sea_renderer_ready=0;e->sea_renderer_attempted=0;e->sea_draw_logged=0;
 memset(&e->in,0,sizeof(e->in));
 e->dev=VK_NULL_HANDLE;e->surface=VK_NULL_HANDLE;e->sw=VK_NULL_HANDLE;e->gpu=VK_NULL_HANDLE;e->q=VK_NULL_HANDLE;
 e->rp=VK_NULL_HANDLE;e->pipe=VK_NULL_HANDLE;e->layout=VK_NULL_HANDLE;e->pool=VK_NULL_HANDLE;
 e->vb=VK_NULL_HANDLE;e->vm=VK_NULL_HANDLE;e->fence=VK_NULL_HANDLE;e->semA=VK_NULL_HANDLE;e->semB=VK_NULL_HANDLE;
}
static int32_t input(struct android_app*a,AInputEvent*ev){
 E*e=a->userData;
 if(!e->ready||AInputEvent_getType(ev)!=AINPUT_EVENT_TYPE_MOTION)return 0;
 int raw=AMotionEvent_getAction(ev);
 int action=raw&AMOTION_EVENT_ACTION_MASK;
 int pointer_index=(raw&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)>>AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
 size_t count=(size_t)AMotionEvent_getPointerCount(ev);
 if(pointer_index<0||(size_t)pointer_index>=count)pointer_index=0;
 int pointer_id=AMotionEvent_getPointerId(ev,(size_t)pointer_index);
 float x=AMotionEvent_getX(ev,(size_t)pointer_index);
 float y=AMotionEvent_getY(ev,(size_t)pointer_index);

 if(e->scene==2&&e->sea_renderer_ready){
  if(action==AMOTION_EVENT_ACTION_DOWN){
   e->sea_touch_mode=1;e->dragging=1;e->camera_pointer_id=pointer_id;
   e->last_x=x;e->last_y=y;e->sea_pointer_a=pointer_id;e->sea_pointer_b=-1;
   return 1;
  }
  if(action==AMOTION_EVENT_ACTION_POINTER_DOWN&&count>=2u){
   e->sea_touch_mode=2;e->dragging=0;
   e->sea_pointer_a=AMotionEvent_getPointerId(ev,0);
   e->sea_pointer_b=AMotionEvent_getPointerId(ev,1);
   float ax=AMotionEvent_getX(ev,0),ay=AMotionEvent_getY(ev,0);
   float bx=AMotionEvent_getX(ev,1),by=AMotionEvent_getY(ev,1);
   e->sea_touch_x=(ax+bx)*0.5f;e->sea_touch_y=(ay+by)*0.5f;
   e->sea_touch_distance=hypotf(bx-ax,by-ay);
   return 1;
  }
  if(action==AMOTION_EVENT_ACTION_MOVE){
   if(count>=2u&&e->sea_touch_mode==2){
    float ax=AMotionEvent_getX(ev,0),ay=AMotionEvent_getY(ev,0);
    float bx=AMotionEvent_getX(ev,1),by=AMotionEvent_getY(ev,1);
    float cx=(ax+bx)*0.5f,cy=(ay+by)*0.5f;
    float distance=hypotf(bx-ax,by-ay);
    float scale=(float)(e->w<e->h?e->w:e->h);
    float sensitivity=scale>0.0f?18.0f/scale:0.018f;
    if(tg_sea_camera_translate(&e->sea_camera,e->sea_yaw,e->sea_pitch,
                               (e->sea_touch_x-cx)*sensitivity,
                               (cy-e->sea_touch_y)*sensitivity,
                               (distance-e->sea_touch_distance)*sensitivity*0.75f)!=0)
     ERR("SEA_STAGE free-camera translation failed");
    e->sea_touch_x=cx;e->sea_touch_y=cy;e->sea_touch_distance=distance;
   }else if(count==1u&&e->sea_touch_mode==1&&e->dragging){
    int id=AMotionEvent_getPointerId(ev,0);
    if(id==e->camera_pointer_id){
     float px=AMotionEvent_getX(ev,0),py=AMotionEvent_getY(ev,0);
     float dx=px-e->last_x,dy=py-e->last_y;
     e->sea_yaw-=dx*0.0055f;
     e->sea_pitch-=dy*0.0050f;
     if(e->sea_pitch < -1.48f)e->sea_pitch=-1.48f;
     if(e->sea_pitch >  1.48f)e->sea_pitch= 1.48f;
     e->last_x=px;e->last_y=py;
     if(tg_sea_camera_rebuild(&e->sea_camera,e->sea_yaw,e->sea_pitch)!=0)
       ERR("SEA_STAGE free-camera pose update failed");
    }
   }
   return 1;
  }
  if(action==AMOTION_EVENT_ACTION_POINTER_UP&&count>=2u){
   int remaining=-1;float rx=0.0f,ry=0.0f;
   for(size_t i=0;i<count;i++){
    int id=AMotionEvent_getPointerId(ev,i);
    if(id!=pointer_id){remaining=id;rx=AMotionEvent_getX(ev,i);ry=AMotionEvent_getY(ev,i);break;}
   }
   if(remaining>=0){
    e->sea_touch_mode=1;e->dragging=1;e->camera_pointer_id=remaining;
    e->last_x=rx;e->last_y=ry;e->sea_pointer_a=remaining;e->sea_pointer_b=-1;
   }
   return 1;
  }
  if(action==AMOTION_EVENT_ACTION_UP||action==AMOTION_EVENT_ACTION_CANCEL||
     action==AMOTION_EVENT_ACTION_POINTER_UP){
   e->sea_touch_mode=0;e->dragging=0;e->camera_pointer_id=-1;
   e->sea_pointer_a=-1;e->sea_pointer_b=-1;return 1;
  }
  return 1;
 }
 if(e->scene!=1){
  if(e->scene==0&&action==AMOTION_EVENT_ACTION_UP){
   float cx=e->w*.5f,bw=e->w*.36f,bx=cx-bw/2,by=e->h*.52f,bh=e->h*.14f;
   if(x>=bx&&x<=bx+bw&&y>=by&&y<=by+bh){e->scene=2;e->notice=0;LOG("Start requested; showing construction state");return 1;}
   float ey=by+bh+e->h*.045f;
   if(x>=bx&&x<=bx+bw&&y>=ey&&y<=ey+bh){ANativeActivity_finish(a->activity);return 1;}
  }
  return 1;
 }

 if(action==AMOTION_EVENT_ACTION_DOWN||action==AMOTION_EVENT_ACTION_POINTER_DOWN){
  float cx,cy,r;joystick_metrics(e,&cx,&cy,&r);
  float dx=x-cx,dy=y-cy;
  if(!e->joystick_active&&dx*dx+dy*dy<=r*r*2.25f){
   e->joystick_active=1;e->joystick_pointer_id=pointer_id;
   e->camera_follow_enabled=1;update_joystick(e,x,y);
  }else if(!e->dragging){
   e->dragging=1;e->camera_pointer_id=pointer_id;
   e->camera_follow_enabled=0;e->last_x=x;e->last_y=y;
  }
  return 1;
 }
 if(action==AMOTION_EVENT_ACTION_MOVE){
  for(size_t i=0;i<count;i++){
   int id=AMotionEvent_getPointerId(ev,i);
   float px=AMotionEvent_getX(ev,i),py=AMotionEvent_getY(ev,i);
   if(e->joystick_active&&id==e->joystick_pointer_id)update_joystick(e,px,py);
   if(e->dragging&&id==e->camera_pointer_id){
    world_camera_pan(&e->camera,px-e->last_x,py-e->last_y,(float)e->w,(float)e->h);
    e->last_x=px;e->last_y=py;
   }
  }
  return 1;
 }
 if(action==AMOTION_EVENT_ACTION_UP||action==AMOTION_EVENT_ACTION_POINTER_UP){
  if(e->joystick_active&&pointer_id==e->joystick_pointer_id){
   e->joystick_active=0;e->joystick_pointer_id=-1;player_set_input(&e->player,0.0f,0.0f);
  }
  if(e->dragging&&pointer_id==e->camera_pointer_id){
   e->dragging=0;e->camera_pointer_id=-1;
  }
  return 1;
 }
 if(action==AMOTION_EVENT_ACTION_CANCEL){
  e->joystick_active=0;e->joystick_pointer_id=-1;player_set_input(&e->player,0.0f,0.0f);
  e->dragging=0;e->camera_pointer_id=-1;return 1;
 }
 return 1;
}
static void command(struct android_app*a,int32_t c){E*e=a->userData;if(c==APP_CMD_INIT_WINDOW&&a->window){if(setup(e)){ERR("Vulkan setup failed");closevk(e);}}else if(c==APP_CMD_TERM_WINDOW)closevk(e);}
void android_main(struct android_app*s){
 app_dummy();
 E*e=(E*)calloc(1,sizeof(E));
 if(!e){ERR("Unable to allocate engine state");return;}
 e->app=s;s->userData=e;s->onAppCmd=command;s->onInputEvent=input;
 LOG("TORNIA GULD native boot; engine state=%zu bytes",sizeof(E));
 while(!s->destroyRequested){
  int events;struct android_poll_source*src;
  int id=ALooper_pollOnce(e->ready?0:-1,0,&events,(void**)&src);
  while(id>=0){if(src)src->process(s,src);if(s->destroyRequested)break;id=ALooper_pollOnce(0,0,&events,(void**)&src);}
  if(e->ready)frame(e);
 }
 closevk(e);s->userData=NULL;free(e);
}
