#ifndef dsfont52_data_h
#define dsfont52_data_h

#pragma once
#define CLOCKFONT_MONO true
#include <Adafruit_GFX.h>
#include "clockfont_api.h"

// -- DS_DIGI42pt7b --
#define Clock_GFXfont DS_DIGI42pt7b_mono
#include "DS_DIGI42pt7b_mono.h"
#undef Clock_GFXfont

// -- DS_DIGI15pt7b --
#define Clock_GFXfont DS_DIGI15pt7b_mono
#include "DS_DIGI15pt7b_mono.h"
#undef Clock_GFXfont

// -- PointedlyMad38pt7b --
#define Clock_GFXfont PointedlyMad38pt7b_mono
#include "PointedlyMad38pt7b_mono.h"
#undef Clock_GFXfont

// -- PointedlyMad14pt7b --
#define Clock_GFXfont PointedlyMad14pt7b_mono
#include "PointedlyMad14pt7b_mono.h"
#undef Clock_GFXfont

// -- Office39pt7b --
#define Clock_GFXfont Office39pt7b_mono
#include "Office39pt7b_mono.h"
#undef Clock_GFXfont

// -- Office14pt7b --
#define Clock_GFXfont Office14pt7b_mono
#include "Office14pt7b_mono.h"
#undef Clock_GFXfont

// -- Oldtimer30pt7b --
#define Clock_GFXfont Oldtimer30pt7b_mono
#include "Oldtimer30pt7b_mono.h"
#undef Clock_GFXfont

// -- Oldtimer10pt7b --
#define Clock_GFXfont Oldtimer10pt7b_mono
#include "Oldtimer10pt7b_mono.h"
#undef Clock_GFXfont

// -- LaradotSerif38pt7b --
#define Clock_GFXfont LaradotSerif38pt7b_mono
#include "LaradotSerif38pt7b_mono.h"
#undef Clock_GFXfont

// -- LaradotSerif14pt7b --
#define Clock_GFXfont LaradotSerif14pt7b_mono
#include "LaradotSerif14pt7b_mono.h"
#undef Clock_GFXfont

// -- SquareFont37pt7b --
#define Clock_GFXfont SquareFont37pt7b_mono
#include "SquareFont37pt7b_mono.h"
#undef Clock_GFXfont

// -- SquareFont14pt7b --
#define Clock_GFXfont SquareFont14pt7b_mono
#include "SquareFont14pt7b_mono.h"
#undef Clock_GFXfont

// -- Decoderr39pt7b --
#define Clock_GFXfont Decoderr39pt7b_mono
#include "Decoderr39pt7b_mono.h"
#undef Clock_GFXfont

// -- Decoderr14pt7b --
#define Clock_GFXfont Decoderr14pt7b_mono
#include "Decoderr14pt7b_mono.h"
#undef Clock_GFXfont

static const ClockFontSpec CLOCK_FONTS_LARGE[] PROGMEM = {
  { &DS_DIGI42pt7b_mono, &DS_DIGI15pt7b_mono, 0, -3, 40 },
  { &PointedlyMad38pt7b_mono, &PointedlyMad14pt7b_mono, 0, -3, 40 },
  { &Office39pt7b_mono, &Office14pt7b_mono, 0, -3, 40 },
  { &Oldtimer30pt7b_mono, &Oldtimer10pt7b_mono, 0, -3, 40 },
  { &LaradotSerif38pt7b_mono, &LaradotSerif14pt7b_mono, 0, -3, 40 },
  { &SquareFont37pt7b_mono, &SquareFont14pt7b_mono, 0, -3, 40 },
  { &Decoderr39pt7b_mono, &Decoderr14pt7b_mono, 0, -3, 40 },
};

#endif