#version 450
layout(push_constant) uniform SkyState{vec4 cameraForward;vec4 cameraRight;vec4 cameraUp;vec4 sunDirection;vec4 weather;vec4 cloud;vec4 cameraPosition;}sky;
layout(location=0) in vec2 inUv;layout(location=0) out vec4 outColor;
float hash31(vec3 p){p=fract(p*.1031);p+=dot(p,p.yzx+33.33);return fract((p.x+p.y)*p.z);}
float noise3(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);float a=mix(hash31(i),hash31(i+vec3(1,0,0)),f.x);float b=mix(hash31(i+vec3(0,1,0)),hash31(i+vec3(1,1,0)),f.x);float c=mix(hash31(i+vec3(0,0,1)),hash31(i+vec3(1,0,1)),f.x);float d=mix(hash31(i+vec3(0,1,1)),hash31(i+vec3(1,1,1)),f.x);return mix(mix(a,b,f.y),mix(c,d,f.y),f.z);}
float cloudNoise(vec3 p){float n=noise3(p)*.56;p*=2.03;n+=noise3(p)*.28;p*=2.01;n+=noise3(p)*.14;return n/.98;}
float cloudDensity(vec3 p){vec2 w=vec2(cos(sky.weather.z),sin(sky.weather.z));p.xz-=w*(sky.weather.x*sky.weather.y*.018);float n=cloudNoise(p*vec3(.010,.014,.010));float th=mix(.72,.43,clamp(sky.weather.w,0.,1.));return smoothstep(th,th+.15,n)*smoothstep(.02,.18,sky.weather.w);}
void main(){
 /* Match the Vulkan top-left viewport convention: screen top looks upward. */
 vec2 ndc=vec2(inUv.x*2.-1.,inUv.y*2.-1.);
 vec3 ray=normalize(sky.cameraForward.xyz+sky.cameraRight.xyz*ndc.x*sky.cameraForward.w*sky.cameraRight.w+sky.cameraUp.xyz*ndc.y*sky.cameraForward.w);
 vec3 sun=normalize(sky.sunDirection.xyz);float h=pow(1.-max(ray.y,0.),3.);
 vec3 color=mix(vec3(.105,.34,.68),vec3(.66,.78,.88),h*.82)+vec3(1.,.70,.38)*pow(max(dot(ray,sun),0.),18.)*.22;
 float trans=1.;vec3 cloudLight=vec3(0.);
 if(ray.y>.008&&sky.weather.w>.01){
  float t0=max((sky.cloud.x-sky.cameraPosition.y)/ray.y,0.),t1=(sky.cloud.y-sky.cameraPosition.y)/ray.y;
  if(t1>t0){const int STEPS=12;float ds=(t1-t0)/float(STEPS);
   for(int i=0;i<STEPS;i++){float t=t0+(float(i)+.5)*ds;vec3 p=sky.cameraPosition.xyz+ray*t;float d=cloudDensity(p),a=1.-exp(-d*ds*.0048);vec3 lit=mix(vec3(.38,.46,.55),vec3(1.,.94,.82),.35+.65*max(sun.y,0.));cloudLight+=trans*a*lit;trans*=1.-a;if(trans<.025)break;}
  }
 }
 color=color*trans+cloudLight;
 /* Placeholder precipitation streaks; the initial weather preset is dry. */
 if(sky.cloud.z>0.){float amount=clamp(sky.cloud.z/35.,0.,1.);float streak=fract(inUv.x*830.+inUv.y*113.+sky.weather.x*(12.+sky.weather.y));color=mix(color,vec3(.72,.82,.91),smoothstep(.965,1.,streak)*amount*(1.-trans)*.55);}
 outColor=vec4(color,1.);
}
