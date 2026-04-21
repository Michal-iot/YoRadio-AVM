#pragma once
#include <Adafruit_GFX.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ClockFontSpec { 
    const GFXfont* font; 
    const GFXfont* fontSmall; 
    int8_t baseline; 
    int8_t baselineSmall; 
    uint8_t adv; 
};

void clockfont_set_large(bool large);          // <-- DODANE
uint8_t     clockfont_clamp_id(uint8_t id);
const GFXfont* clockfont_get(uint8_t id);
int8_t      clockfont_baseline(uint8_t id);
uint8_t     clockfont_advance(uint8_t id);
uint8_t     clockfont_count();
const GFXfont* clockfont_get_small(uint8_t id);
int8_t      clockfont_baseline_small(uint8_t id);

#ifdef __cplusplus
}
#endif