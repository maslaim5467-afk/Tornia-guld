#ifndef TORNIA_WEATHER_H
#define TORNIA_WEATHER_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct TgWeatherState{float time_of_day_hours,elapsed_seconds,wind_speed_mps,wind_direction_radians;float cloud_coverage,cloud_base_height_m,cloud_top_height_m,precipitation_mm_per_hour;float sun_direction[3];}TgWeatherState;
void tg_weather_init(TgWeatherState*);
void tg_weather_update(TgWeatherState*,float);
void tg_weather_wind_vector(const TgWeatherState*,float*,float*);
void tg_weather_set_precipitation(TgWeatherState*,float);
#ifdef __cplusplus
}
#endif
#endif
