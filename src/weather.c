#include "weather.h"
#include <math.h>
void tg_weather_init(TgWeatherState*w){
 if(!w)return;
 w->time_of_day_hours=9.0f;w->elapsed_seconds=0.0f;w->wind_speed_mps=5.5f;w->wind_direction_radians=0.35f;
 w->cloud_coverage=0.42f;w->cloud_base_height_m=140.0f;w->cloud_top_height_m=260.0f;w->precipitation_mm_per_hour=0.0f;
 /* Fixed 09:00 preview direction; location/date-aware astronomy comes later. */
 w->sun_direction[0]=0.0f;w->sun_direction[1]=0.42f;w->sun_direction[2]=-0.907524f;
}
void tg_weather_update(TgWeatherState*w,float dt){
 if(!w||!isfinite(dt)||dt<=0.0f)return;
 if(dt>0.25f)dt=0.25f;
 w->elapsed_seconds+=dt;
}
void tg_weather_wind_vector(const TgWeatherState*w,float*x,float*z){
 if(x)*x=0.0f;
 if(z)*z=0.0f;
 if(!w)return;
 if(x)*x=cosf(w->wind_direction_radians);
 if(z)*z=sinf(w->wind_direction_radians);
}
void tg_weather_set_precipitation(TgWeatherState*w,float r){
 if(!w||!isfinite(r))return;
 if(r<0.0f)r=0.0f;
 if(r>200.0f)r=200.0f;
 w->precipitation_mm_per_hour=r;
}
