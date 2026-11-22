#include "../src/weather.h"
#include <assert.h>
#include <math.h>
int main(void){
 TgWeatherState w;float x=0.0f,z=0.0f;tg_weather_init(&w);
 assert(fabsf(w.time_of_day_hours-9.0f)<0.0001f);
 assert(w.wind_speed_mps>0.0f&&w.cloud_top_height_m>w.cloud_base_height_m);
 assert(w.precipitation_mm_per_hour==0.0f);
 tg_weather_wind_vector(&w,&x,&z);assert(fabsf(hypotf(x,z)-1.0f)<0.0001f);
 tg_weather_update(&w,0.5f);assert(w.elapsed_seconds<=0.251f);
 tg_weather_set_precipitation(&w,250.0f);assert(w.precipitation_mm_per_hour==200.0f);
 tg_weather_set_precipitation(&w,-1.0f);assert(w.precipitation_mm_per_hour==0.0f);
 return 0;
}
