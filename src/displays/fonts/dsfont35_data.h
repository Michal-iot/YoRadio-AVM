#ifndef dsfont35_data_h
#define dsfont35_data_h

#pragma once
#define CLOCKFONT_MONO true
#include <Adafruit_GFX.h>
#include "clockfont_api.h"

// -- DS_DIGI28pt7b --
#define Clock_GFXfont DS_DIGI28pt7b_mono
#include "DS_DIGI28pt7b_mono.h"
#undef Clock_GFXfont

// -- DS_DIGI12pt7b --
#define Clock_GFXfont DS_DIGI12pt7b_mono
#include "DS_DIGI12pt7b_mono.h"
#undef Clock_GFXfont

// -- PointedlyMad25pt7b --
#define Clock_GFXfont PointedlyMad25pt7b_mono
#include "PointedlyMad25pt7b_mono.h"
#undef Clock_GFXfont

// -- PointedlyMad11pt7b --
#define Clock_GFXfont PointedlyMad11pt7b_mono
#include "PointedlyMad11pt7b_mono.h"
#undef Clock_GFXfont

// -- Office26pt7b --
#define Clock_GFXfont Office26pt7b_mono
#include "Office26pt7b_mono.h"
#undef Clock_GFXfont

// -- Office11pt7b --
#define Clock_GFXfont Office11pt7b_mono
#include "Office11pt7b_mono.h"
#undef Clock_GFXfont

// -- Oldtimer20pt7b --
#define Clock_GFXfont Oldtimer20pt7b_mono
#include "Oldtimer20pt7b_mono.h"
#undef Clock_GFXfont

// -- Oldtimer8pt7b --
#define Clock_GFXfont Oldtimer8pt7b_mono
#include "Oldtimer8pt7b_mono.h"
#undef Clock_GFXfont

// -- LaradotSerif25pt7b --
#define Clock_GFXfont LaradotSerif25pt7b_mono
#include "LaradotSerif25pt7b_mono.h"
#undef Clock_GFXfont

// -- LaradotSerif11pt7b --
#define Clock_GFXfont LaradotSerif11pt7b_mono
#include "LaradotSerif11pt7b_mono.h"
#undef Clock_GFXfont

// -- SquareFont25pt7b --
#define Clock_GFXfont SquareFont25pt7b_mono
#include "SquareFont25pt7b_mono.h"
#undef Clock_GFXfont

// -- SquareFont10pt7b --
#define Clock_GFXfont SquareFont10pt7b_mono
#include "SquareFont10pt7b_mono.h"
#undef Clock_GFXfont

// -- Decoderr26pt7b --
#define Clock_GFXfont Decoderr26pt7b_mono
#include "Decoderr26pt7b_mono.h"
#undef Clock_GFXfont

// -- Decoderr11pt7b --
#define Clock_GFXfont Decoderr11pt7b_mono
#include "Decoderr11pt7b_mono.h"
#undef Clock_GFXfont

static const ClockFontSpec CLOCK_FONTS_SMALL[] PROGMEM = {
  { &DS_DIGI28pt7b_mono, &DS_DIGI12pt7b_mono, 0, 0, 27 },
  { &PointedlyMad25pt7b_mono, &PointedlyMad11pt7b_mono, 0, 0, 27 },
  { &Office26pt7b_mono, &Office11pt7b_mono, 0, 0, 27 },
  { &Oldtimer20pt7b_mono, &Oldtimer8pt7b_mono, 0, 0, 27 },
  { &LaradotSerif25pt7b_mono, &LaradotSerif11pt7b_mono, 0, 0, 27 },
  { &SquareFont25pt7b_mono, &SquareFont10pt7b_mono, 0, 0, 27 },
  { &Decoderr26pt7b_mono, &Decoderr11pt7b_mono, 0, 0, 27 },
};

#endif