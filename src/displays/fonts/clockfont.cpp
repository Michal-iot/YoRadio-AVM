#include "clockfont_api.h"

// Pliki danych – przekształcone wersje oryginalnych dsfont35.h i dsfont52.h
// (patrz punkt 4 i 5)
#include "dsfont35_data.h"   // definiuje CLOCK_FONTS_SMALL
#include "dsfont52_data.h"   // definiuje CLOCK_FONTS_LARGE

static const ClockFontSpec* current_fonts = CLOCK_FONTS_SMALL;
static uint8_t current_count = (sizeof(CLOCK_FONTS_SMALL)/sizeof(CLOCK_FONTS_SMALL[0]));

void clockfont_set_large(bool large) {
    if (large) {
        current_fonts = CLOCK_FONTS_LARGE;
        current_count = (sizeof(CLOCK_FONTS_LARGE)/sizeof(CLOCK_FONTS_LARGE[0]));
    } else {
        current_fonts = CLOCK_FONTS_SMALL;
        current_count = (sizeof(CLOCK_FONTS_SMALL)/sizeof(CLOCK_FONTS_SMALL[0]));
    }
}

uint8_t clockfont_clamp_id(uint8_t id) {
    return (id < current_count) ? id : 0;
}

const GFXfont* clockfont_get(uint8_t id) {
    return current_fonts[clockfont_clamp_id(id)].font;
}

int8_t clockfont_baseline(uint8_t id) {
    return current_fonts[clockfont_clamp_id(id)].baseline;
}

uint8_t clockfont_advance(uint8_t id) {
    return current_fonts[clockfont_clamp_id(id)].adv;
}

const GFXfont* clockfont_get_small(uint8_t id) {
    return current_fonts[clockfont_clamp_id(id)].fontSmall;
}

int8_t clockfont_baseline_small(uint8_t id) {
    return current_fonts[clockfont_clamp_id(id)].baselineSmall;
}

uint8_t clockfont_count() {
    return current_count;
}