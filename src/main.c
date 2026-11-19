#define VK_USE_PLATFORM_ANDROID_KHR
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <jni.h>
#include <android/native_window.h>
#include <vulkan/vulkan.h>
#include "world.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define LOG(...) __android_log_print(ANDROID_LOG_INFO,"TORNIA_GULD",__VA_ARGS__)
#define ERR(...) __android_log_print(ANDROID_LOG_ERROR,"TORNIA_GULD",__VA_ARGS__)
#define MAXV 40000
typedef struct {float x,y,u,v,r,g,b,a,tex;} V;
typedef struct {
 struct android_app *app; VkInstance in; VkSurfaceKHR surface; VkPhysicalDevice gpu; VkDevice dev; VkQueue q;
 uint32_t qf,w,h,n; VkSwapchainKHR sw; VkFormat fmt; VkImage *imgs; VkImageView *views; VkFramebuffer *fb;
 VkRenderPass rp; VkPipelineLayout layout; VkPipeline pipe; VkCommandPool pool; VkCommandBuffer *cmd;
 VkBuffer vb; VkDeviceMemory vm; VkSemaphore semA,semB; VkFence fence;
 VkImage atlas; VkDeviceMemory atlas_mem; VkImageView atlas_view; VkSampler atlas_sampler;
 VkDescriptorSetLayout descriptor_layout; VkDescriptorPool descriptor_pool; VkDescriptorSet descriptor_set;
 V v[MAXV]; uint32_t nv; int ready,notice,scene,dragging,world_logged;
 World world; WorldCamera camera; float last_x,last_y;
} E;
static void push(E*e,float x,float y,float w,float h,float r,float g,float b){
 float l=x*2/e->w-1,rr=(x+w)*2/e->w-1,t=y*2/e->h-1,bb=(y+h)*2/e->h-1;
 V a={l,t,0,0,r,g,b,1,0},c={rr,t,0,0,r,g,b,1,0},d={rr,bb,0,0,r,g,b,1,0},f={l,bb,0,0,r,g,b,1,0};
 if(e->nv+6>MAXV)return;e->v[e->nv++]=a;e->v[e->nv++]=c;e->v[e->nv++]=d;e->v[e->nv++]=a;e->v[e->nv++]=d;e->v[e->nv++]=f;
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
static void draw_world(E*e){
 e->nv=0;
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
 push(e,0,0,e->w,42,.025f,.065f,.035f);
 label(e,"WORLD",20,14,2,.91f,.88f,.68f);
 label(e,"DRAG TO MOVE",20,e->h-30,1.5f,.78f,.87f,.70f);
 if(!e->world_logged){LOG("Textured world ready: vertices=%u viewport=%ux%u",e->nv,e->w,e->h);e->world_logged=1;}
}

static void menu(E*e){
 e->nv=0;float cx=e->w*.5f,bw=e->w*.36f,bh=e->h*.14f,bx=cx-bw/2,by=e->h*.52f;
 push(e,0,0,e->w,e->h,.035f,.052f,.07f);
 label(e,"TORNIA_GULD",cx-84,e->h*.20f,4,.91f,.82f,.63f);
 
 push(e,bx,by,bw,bh,.62f,.45f,.23f);push(e,bx+3,by+3,bw-6,bh-6,.10f,.12f,.14f);
 label(e,"START GAME",cx-59,by+bh/2-7,2,.96f,.87f,.70f);
 float ey=by+bh+e->h*.045f;push(e,bx,ey,bw,bh,.38f,.4f,.42f);push(e,bx+3,ey+3,bw-6,bh-6,.09f,.11f,.13f);
 label(e,"EXIT",cx-20,ey+bh/2-7,2,.85f,.86f,.87f);
 if(e->notice){float my=e->h*.84f,mw=e->w*.55f;push(e,cx-mw/2,my,mw,e->h*.11f,.60f,.43f,.22f);push(e,cx-mw/2+3,my+3,mw-6,e->h*.11f-6,.08f,.10f,.12f);label(e,"COMING SOON",cx-49,my+e->h*.035f,1.5f,.98f,.88f,.72f);}
}
static uint32_t memtype(E*e,uint32_t mask,VkMemoryPropertyFlags flags){VkPhysicalDeviceMemoryProperties p;vkGetPhysicalDeviceMemoryProperties(e->gpu,&p);for(uint32_t i=0;i<p.memoryTypeCount;i++)if((mask&(1u<<i))&&(p.memoryTypes[i].propertyFlags&flags)==flags)return i;return UINT32_MAX;}
static VkShaderModule shader(E*e,const char*path){AAsset*a=AAssetManager_open(e->app->activity->assetManager,path,AASSET_MODE_BUFFER);if(!a)return 0;size_t n=AAsset_getLength(a);uint32_t*p=malloc(n);int got=AAsset_read(a,p,n);AAsset_close(a);if(got!=(int)n){free(p);return 0;}VkShaderModuleCreateInfo ci={.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,.codeSize=n,.pCode=p};VkShaderModule m=0;vkCreateShaderModule(e->dev,&ci,0,&m);free(p);return m;}
#define ATLAS_SIDE 1024
#define ATLAS_CELL 256
static int decode_asset_rgba(E*e,const char*path,uint8_t*out){
 AAsset*a=AAssetManager_open(e->app->activity->assetManager,path,AASSET_MODE_BUFFER);
 if(!a)return -1;
 off_t length=AAsset_getLength(a);
 if(length<=0||length>8*1024*1024){AAsset_close(a);return -1;}
 jbyte*bytes=malloc((size_t)length);
 if(!bytes){AAsset_close(a);return -1;}
 int got=AAsset_read(a,bytes,(size_t)length);AAsset_close(a);
 if(got!=(int)length){free(bytes);return -1;}
 JNIEnv*env=NULL;
 JavaVM*vm=e->app->activity->vm;
 if(!vm||(*vm)->AttachCurrentThread(vm,&env,NULL)!=JNI_OK||!env){ERR("Could not attach render thread to Android JVM");return -1;}
 jclass factory=(*env)->FindClass(env,"android/graphics/BitmapFactory");
 if(!factory||(*env)->ExceptionCheck(env)){(*env)->ExceptionClear(env);(*vm)->DetachCurrentThread(vm);ERR("BitmapFactory class lookup failed");return -1;}
 jmethodID decode=(*env)->GetStaticMethodID(env,factory,"decodeByteArray","([BII)Landroid/graphics/Bitmap;");
 jbyteArray encoded=(*env)->NewByteArray(env,(jsize)length);
 (*env)->SetByteArrayRegion(env,encoded,0,(jsize)length,bytes);free(bytes);
 jobject bitmap=(*env)->CallStaticObjectMethod(env,factory,decode,encoded,0,(jint)length);
 (*env)->DeleteLocalRef(env,encoded);
 if((*env)->ExceptionCheck(env)){(*env)->ExceptionClear(env);(*vm)->DetachCurrentThread(vm);return -1;}
 if(!bitmap){(*vm)->DetachCurrentThread(vm);return -1;}
 jclass bitmap_cls=(*env)->FindClass(env,"android/graphics/Bitmap");
 jmethodID scale=(*env)->GetStaticMethodID(env,bitmap_cls,"createScaledBitmap","(Landroid/graphics/Bitmap;IIZ)Landroid/graphics/Bitmap;");
 jobject scaled=(*env)->CallStaticObjectMethod(env,bitmap_cls,scale,bitmap,(jint)ATLAS_CELL,(jint)ATLAS_CELL,JNI_TRUE);
 jmethodID get_pixels=(*env)->GetMethodID(env,bitmap_cls,"getPixels","([IIIIIII)V");
 jintArray pixels=(*env)->NewIntArray(env,ATLAS_CELL*ATLAS_CELL);
 (*env)->CallVoidMethod(env,scaled,get_pixels,pixels,0,ATLAS_CELL,0,0,ATLAS_CELL,ATLAS_CELL);
 if((*env)->ExceptionCheck(env)){(*env)->ExceptionClear(env);(*vm)->DetachCurrentThread(vm);return -1;}
 jint*p=(*env)->GetIntArrayElements(env,pixels,0);
 if(!p){(*vm)->DetachCurrentThread(vm);return -1;}
 for(int i=0;i<ATLAS_CELL*ATLAS_CELL;i++){
  uint32_t v=(uint32_t)p[i];
  out[i*4+0]=(uint8_t)(v>>16);out[i*4+1]=(uint8_t)(v>>8);
  out[i*4+2]=(uint8_t)v;out[i*4+3]=(uint8_t)(v>>24);
 }
 (*env)->ReleaseIntArrayElements(env,pixels,p,JNI_ABORT);
 (*env)->DeleteLocalRef(env,pixels);(*env)->DeleteLocalRef(env,scaled);
 (*env)->DeleteLocalRef(env,bitmap);(*env)->DeleteLocalRef(env,bitmap_cls);
 (*env)->DeleteLocalRef(env,factory);
 (*vm)->DetachCurrentThread(vm);
 return 0;
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
 uint8_t*rgba=calloc(1,atlas_bytes);uint8_t*cell=malloc((size_t)ATLAS_CELL*ATLAS_CELL*4u);
 if(!rgba||!cell){free(rgba);free(cell);return -1;}
 for(uint32_t i=0;i<14;i++){
  if(decode_asset_rgba(e,paths[i],cell)){ERR("Could not decode terrain asset %s",paths[i]);free(rgba);free(cell);return -1;}
  uint32_t ox=(i%4u)*ATLAS_CELL,oy=(i/4u)*ATLAS_CELL;
  for(uint32_t y=0;y<ATLAS_CELL;y++)
   memcpy(rgba+(((size_t)(oy+y)*ATLAS_SIDE+ox)*4u),cell+(size_t)y*ATLAS_CELL*4u,(size_t)ATLAS_CELL*4u);
 }
 free(cell);
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
 VkDescriptorPoolSize ps={VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1};
 VkDescriptorPoolCreateInfo dp={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,.maxSets=1,.poolSizeCount=1,.pPoolSizes=&ps};
 if(vkCreateDescriptorPool(e->dev,&dp,0,&e->descriptor_pool))return -1;
 VkDescriptorSetAllocateInfo da={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,.descriptorPool=e->descriptor_pool,.descriptorSetCount=1,.pSetLayouts=&e->descriptor_layout};
 if(vkAllocateDescriptorSets(e->dev,&da,&e->descriptor_set))return -1;
 VkDescriptorImageInfo image_info={.sampler=e->atlas_sampler,.imageView=e->atlas_view,.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
 VkWriteDescriptorSet wr={.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,.dstSet=e->descriptor_set,.dstBinding=0,.descriptorCount=1,.descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,.pImageInfo=&image_info};
 vkUpdateDescriptorSets(e->dev,1,&wr,0,0);
 LOG("Loaded grass base and 13 transparent overlays into Vulkan atlas");
 return 0;
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
 VkAttachmentDescription ad={.format=e->fmt,.samples=VK_SAMPLE_COUNT_1_BIT,.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR,.storeOp=VK_ATTACHMENT_STORE_OP_STORE,.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};VkAttachmentReference ar={0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};VkSubpassDescription sub={.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS,.colorAttachmentCount=1,.pColorAttachments=&ar};VkRenderPassCreateInfo ri={.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,.attachmentCount=1,.pAttachments=&ad,.subpassCount=1,.pSubpasses=&sub};if(vkCreateRenderPass(e->dev,&ri,0,&e->rp))return -1;
 VkDescriptorSetLayoutBinding db={.binding=0,.descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,.descriptorCount=1,.stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT};VkDescriptorSetLayoutCreateInfo dl={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,.bindingCount=1,.pBindings=&db};if(vkCreateDescriptorSetLayout(e->dev,&dl,0,&e->descriptor_layout))return -1;
 VkShaderModule vs=shader(e,"shaders/menu.vert.spv"),fsmod=shader(e,"shaders/menu.frag.spv");if(!vs||!fsmod)return -1;
 VkPipelineShaderStageCreateInfo st[2]={{.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_VERTEX_BIT,.module=vs,.pName="main"},{.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage=VK_SHADER_STAGE_FRAGMENT_BIT,.module=fsmod,.pName="main"}};
 VkVertexInputBindingDescription bd={0,sizeof(V),VK_VERTEX_INPUT_RATE_VERTEX};VkVertexInputAttributeDescription at[4]={{0,0,VK_FORMAT_R32G32_SFLOAT,0},{1,0,VK_FORMAT_R32G32B32A32_SFLOAT,16},{2,0,VK_FORMAT_R32G32_SFLOAT,8},{3,0,VK_FORMAT_R32_SFLOAT,32}};VkPipelineVertexInputStateCreateInfo vx={.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,.vertexBindingDescriptionCount=1,.pVertexBindingDescriptions=&bd,.vertexAttributeDescriptionCount=4,.pVertexAttributeDescriptions=at};VkPipelineInputAssemblyStateCreateInfo ia={.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};VkPipelineViewportStateCreateInfo vp={.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,.viewportCount=1,.scissorCount=1};VkPipelineRasterizationStateCreateInfo rs={.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,.polygonMode=VK_POLYGON_MODE_FILL,.cullMode=VK_CULL_MODE_NONE,.lineWidth=1};VkPipelineMultisampleStateCreateInfo ms={.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT};VkPipelineColorBlendAttachmentState ba={.blendEnable=VK_TRUE,.srcColorBlendFactor=VK_BLEND_FACTOR_SRC_ALPHA,.dstColorBlendFactor=VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,.colorBlendOp=VK_BLEND_OP_ADD,.srcAlphaBlendFactor=VK_BLEND_FACTOR_ONE,.dstAlphaBlendFactor=VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,.alphaBlendOp=VK_BLEND_OP_ADD,.colorWriteMask=15};VkPipelineColorBlendStateCreateInfo cb={.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,.attachmentCount=1,.pAttachments=&ba};VkDynamicState dyns[]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};VkPipelineDynamicStateCreateInfo dy={.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,.dynamicStateCount=2,.pDynamicStates=dyns};VkPipelineLayoutCreateInfo li={.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,.setLayoutCount=1,.pSetLayouts=&e->descriptor_layout};if(vkCreatePipelineLayout(e->dev,&li,0,&e->layout))return -1;VkGraphicsPipelineCreateInfo pi={.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,.stageCount=2,.pStages=st,.pVertexInputState=&vx,.pInputAssemblyState=&ia,.pViewportState=&vp,.pRasterizationState=&rs,.pMultisampleState=&ms,.pColorBlendState=&cb,.pDynamicState=&dy,.layout=e->layout,.renderPass=e->rp};VkResult pkr=vkCreateGraphicsPipelines(e->dev,0,1,&pi,0,&e->pipe);vkDestroyShaderModule(e->dev,vs,0);vkDestroyShaderModule(e->dev,fsmod,0);if(pkr)return -1;
 for(uint32_t i=0;i<e->n;i++){VkFramebufferCreateInfo fi={.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,.renderPass=e->rp,.attachmentCount=1,.pAttachments=&e->views[i],.width=e->w,.height=e->h,.layers=1};if(vkCreateFramebuffer(e->dev,&fi,0,&e->fb[i]))return -1;}
 VkCommandPoolCreateInfo pci={.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,.queueFamilyIndex=e->qf};if(vkCreateCommandPool(e->dev,&pci,0,&e->pool))return -1;VkCommandBufferAllocateInfo cai={.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,.commandPool=e->pool,.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY,.commandBufferCount=e->n};if(vkAllocateCommandBuffers(e->dev,&cai,e->cmd))return -1;
 VkBufferCreateInfo bi={.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,.size=sizeof(V)*MAXV,.usage=VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,.sharingMode=VK_SHARING_MODE_EXCLUSIVE};if(vkCreateBuffer(e->dev,&bi,0,&e->vb))return -1;VkMemoryRequirements mr;vkGetBufferMemoryRequirements(e->dev,e->vb,&mr);uint32_t mt=memtype(e,mr.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);if(mt==UINT32_MAX)return -1;VkMemoryAllocateInfo mai={.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,.allocationSize=mr.size,.memoryTypeIndex=mt};if(vkAllocateMemory(e->dev,&mai,0,&e->vm))return -1;vkBindBufferMemory(e->dev,e->vb,e->vm,0);
 if(create_texture_atlas(e)){ERR("Terrain texture atlas creation failed");return -1;}
 VkSemaphoreCreateInfo sem={.sType=VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};vkCreateSemaphore(e->dev,&sem,0,&e->semA);vkCreateSemaphore(e->dev,&sem,0,&e->semB);VkFenceCreateInfo fe={.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,.flags=VK_FENCE_CREATE_SIGNALED_BIT};vkCreateFence(e->dev,&fe,0,&e->fence);world_generate(&e->world,0x4f4c5245u);world_camera_reset(&e->camera);e->camera.x=((float)WORLD_WIDTH*WORLD_TILE_SIZE-(float)e->w)*.5f;e->camera.y=((float)WORLD_HEIGHT*WORLD_TILE_SIZE-(float)e->h)*.5f;world_camera_clamp(&e->camera,(float)e->w,(float)e->h);e->ready=1;LOG("Vulkan main menu ready %ux%u",e->w,e->h);return 0;
}
static void frame(E*e){if(!e->ready)return;vkWaitForFences(e->dev,1,&e->fence,VK_TRUE,UINT64_MAX);uint32_t ix;if(vkAcquireNextImageKHR(e->dev,e->sw,UINT64_MAX,e->semA,0,&ix)!=VK_SUCCESS)return;if(e->scene)draw_world(e);else menu(e);void*m;if(vkMapMemory(e->dev,e->vm,0,e->nv*sizeof(V),0,&m))return;memcpy(m,e->v,e->nv*sizeof(V));vkUnmapMemory(e->dev,e->vm);VkCommandBuffer c=e->cmd[ix];vkResetCommandBuffer(c,0);VkCommandBufferBeginInfo bi={.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};vkBeginCommandBuffer(c,&bi);VkClearValue cl={.color={{.035f,.052f,.07f,1}}};VkRenderPassBeginInfo ri={.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,.renderPass=e->rp,.framebuffer=e->fb[ix],.renderArea={{0,0},{e->w,e->h}},.clearValueCount=1,.pClearValues=&cl};vkCmdBeginRenderPass(c,&ri,VK_SUBPASS_CONTENTS_INLINE);VkViewport vp={0,0,(float)e->w,(float)e->h,0,1};VkRect2D sc={{0,0},{e->w,e->h}};vkCmdSetViewport(c,0,1,&vp);vkCmdSetScissor(c,0,1,&sc);vkCmdBindPipeline(c,VK_PIPELINE_BIND_POINT_GRAPHICS,e->pipe);vkCmdBindDescriptorSets(c,VK_PIPELINE_BIND_POINT_GRAPHICS,e->layout,0,1,&e->descriptor_set,0,0);VkDeviceSize off=0;vkCmdBindVertexBuffers(c,0,1,&e->vb,&off);vkCmdDraw(c,e->nv,1,0,0);vkCmdEndRenderPass(c);vkEndCommandBuffer(c);vkResetFences(e->dev,1,&e->fence);VkPipelineStageFlags wait=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;VkSubmitInfo si={.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,.waitSemaphoreCount=1,.pWaitSemaphores=&e->semA,.pWaitDstStageMask=&wait,.commandBufferCount=1,.pCommandBuffers=&c,.signalSemaphoreCount=1,.pSignalSemaphores=&e->semB};if(vkQueueSubmit(e->q,1,&si,e->fence))return;VkPresentInfoKHR pr={.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,.waitSemaphoreCount=1,.pWaitSemaphores=&e->semB,.swapchainCount=1,.pSwapchains=&e->sw,.pImageIndices=&ix};vkQueuePresentKHR(e->q,&pr);}
static void closevk(E*e){if(e->dev)vkDeviceWaitIdle(e->dev);if(e->dev){if(e->fence)vkDestroyFence(e->dev,e->fence,0);if(e->descriptor_pool)vkDestroyDescriptorPool(e->dev,e->descriptor_pool,0);if(e->atlas_sampler)vkDestroySampler(e->dev,e->atlas_sampler,0);if(e->atlas_view)vkDestroyImageView(e->dev,e->atlas_view,0);if(e->atlas)vkDestroyImage(e->dev,e->atlas,0);if(e->atlas_mem)vkFreeMemory(e->dev,e->atlas_mem,0);if(e->descriptor_layout)vkDestroyDescriptorSetLayout(e->dev,e->descriptor_layout,0);if(e->semA)vkDestroySemaphore(e->dev,e->semA,0);if(e->semB)vkDestroySemaphore(e->dev,e->semB,0);if(e->vb)vkDestroyBuffer(e->dev,e->vb,0);if(e->vm)vkFreeMemory(e->dev,e->vm,0);if(e->pool)vkDestroyCommandPool(e->dev,e->pool,0);for(uint32_t i=0;i<e->n;i++){if(e->fb&&e->fb[i])vkDestroyFramebuffer(e->dev,e->fb[i],0);if(e->views&&e->views[i])vkDestroyImageView(e->dev,e->views[i],0);}if(e->pipe)vkDestroyPipeline(e->dev,e->pipe,0);if(e->layout)vkDestroyPipelineLayout(e->dev,e->layout,0);if(e->rp)vkDestroyRenderPass(e->dev,e->rp,0);if(e->sw)vkDestroySwapchainKHR(e->dev,e->sw,0);vkDestroyDevice(e->dev,0);}if(e->surface&&e->in)vkDestroySurfaceKHR(e->in,e->surface,0);if(e->in)vkDestroyInstance(e->in,0);free(e->imgs);free(e->views);free(e->fb);free(e->cmd);e->ready=0;memset(&e->in,0,sizeof(e->in));e->dev=0;e->surface=0;}
static int32_t input(struct android_app*a,AInputEvent*ev){
 E*e=a->userData;
 if(!e->ready||AInputEvent_getType(ev)!=AINPUT_EVENT_TYPE_MOTION)return 0;
 int action=AMotionEvent_getAction(ev)&AMOTION_EVENT_ACTION_MASK;
 float x=AMotionEvent_getX(ev,0),y=AMotionEvent_getY(ev,0);
 if(e->scene){
  if(action==AMOTION_EVENT_ACTION_DOWN){e->dragging=1;e->last_x=x;e->last_y=y;return 1;}
  if(action==AMOTION_EVENT_ACTION_MOVE&&e->dragging){
   world_camera_pan(&e->camera,x-e->last_x,y-e->last_y,(float)e->w,(float)e->h);
   e->last_x=x;e->last_y=y;return 1;
  }
  if(action==AMOTION_EVENT_ACTION_UP||action==AMOTION_EVENT_ACTION_CANCEL){e->dragging=0;return 1;}
  return 1;
 }
 if(action==AMOTION_EVENT_ACTION_UP){
  float cx=e->w*.5f,bw=e->w*.36f,bx=cx-bw/2,by=e->h*.52f,bh=e->h*.14f;
  if(x>=bx&&x<=bx+bw&&y>=by&&y<=by+bh){e->scene=1;e->notice=0;return 1;}
  float ey=by+bh+e->h*.045f;
  if(x>=bx&&x<=bx+bw&&y>=ey&&y<=ey+bh){ANativeActivity_finish(a->activity);return 1;}
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
