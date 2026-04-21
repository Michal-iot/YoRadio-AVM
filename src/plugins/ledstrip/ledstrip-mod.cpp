#include "../../core/options.h"
#ifdef USE_LEDSTRIP_PLUGIN
#include "ledstrip.h"
#include <Adafruit_NeoPixel.h>

#include "../../core/config.h"
#include "../../core/player.h"
#include "../../core/network.h"
#include "../../core/display.h"
#include "../../core/common.h"



extern Player player;
extern Display display;

// -----------------------------------------------------------------------------
// VU provider hookok
// Ha később találsz stabil audio/VU forrást, csak implementáld ezeket máshol.
// 0..255 tartományt várunk.
// -----------------------------------------------------------------------------
extern "C" __attribute__((weak)) uint8_t fusion_led_vu_left()  { return 0; }
extern "C" __attribute__((weak)) uint8_t fusion_led_vu_right() { return 0; }

// -----------------------------------------------------------------------------

#ifndef LEDSTRIP_PIN
  #define LEDSTRIP_PIN 48
#endif

#ifndef LEDSTRIP_COUNT
  #define LEDSTRIP_COUNT 24
#endif

#ifndef LEDSTRIP_BRIGHTNESS
  #define LEDSTRIP_BRIGHTNESS 80
#endif

#define LEDSTRIP_VOL_TIMEOUT_MS   1800
#define LEDSTRIP_FLASH_MS          180
#define LEDSTRIP_FRAME_MS           18
#define LEDSTRIP_IDLE_PULSE_MS      22
#define LEDSTRIP_SCREEN_PULSE_MS    20   // gyorsabb tick, sin simítja
#define LEDSTRIP_CONNECT_PULSE_MS   26

// Breathing sin-lépések száma egy félciklusban (fel vagy le)
// SCREEN_PULSE_MS * BREATH_STEPS = félciklus ideje ms-ben
// pl. 20ms * 80 = 1600ms fel, 1600ms le → ~3.2s egy teljes lélegzet
#define BREATH_STEPS  80

static Adafruit_NeoPixel strip(LEDSTRIP_COUNT, LEDSTRIP_PIN, NEO_GRB + NEO_KHZ800);

enum LedMode : uint8_t {
  LM_BOOT = 0,
  LM_CONNECTING,
  LM_STOP,
  LM_PLAY,
  LM_BUFFERING,
  LM_VOLUME,
  LM_SCREENSAVER
};

static LedMode   g_mode               = LM_BOOT;
static LedMode   g_lastMode           = LM_BOOT;  // módváltás detektáláshoz
static uint8_t   g_lastVolume         = 255;
static uint32_t  g_volumeUntil        = 0;
static uint32_t  g_flashUntil         = 0;
static uint32_t  g_lastFrame          = 0;
static uint32_t  g_lastPulse          = 0;
static uint16_t  g_rainbowIndex       = 0;
// sin-alapú breathing: 0..2*BREATH_STEPS-1 fázis
static uint16_t  g_breathPhase        = 0;
// régi pulse (connecting, buffering, boot)
static uint8_t   g_pulseBrightness    = 20;
static int8_t    g_pulseDir           = 1;
static uint8_t   g_peakL              = 0;
static uint8_t   g_peakR              = 0;
static uint8_t   g_flashR             = 0;
static uint8_t   g_flashG             = 0;
static uint8_t   g_flashB             = 0;
static bool      g_connectedSeen      = false;

// --- Nowe efekty PL ---
static bool      g_stationsList       = false;
static uint32_t  g_stationScrollUntil = 0;
static uint32_t  g_encPulseUntil      = 0;
static uint8_t   g_encPulseR          = 0;
static uint8_t   g_encPulseG          = 0;
static uint8_t   g_encPulseB          = 0;
static uint32_t  g_volPulseUntil      = 0;
static bool      g_volUp              = true;
//static uint16_t  g_vuRainbowIndex     = 0;

// --- Rainbow flow (model=1) ---
//static uint32_t  g_rainbowLastFrame   = 0;

// --- Fire (model=2) ---
// Csak fél szalag kell (tükrözve), max 72 pixel (144/2)
#define FIRE_HALF   (LEDSTRIP_COUNT / 2)
static uint8_t   g_heat[FIRE_HALF];     // hőmérkép 0..255
static uint32_t  g_fireLastFrame      = 0;

// --- Meter VU (model=3) ---
// Fehér sáv + piros csúcsmutató, L bal / R jobb, ~0.5mp visszaesés
static float     g_meterPeakL        = 0.0f;
static float     g_meterPeakR        = 0.0f;

// sin-tábla 0..BREATH_STEPS-1 → 0..255  (negyed periódus, szimmetrikusan tükrözve)
// Előre számolt, hogy ESP32-n ne kelljen float sin() minden frame-ben
static const uint8_t sinTable[81] PROGMEM = {
    0,  3,  6, 10, 13, 16, 19, 22, 25, 28,
   31, 34, 37, 40, 43, 46, 49, 52, 55, 58,
   60, 63, 66, 68, 71, 73, 76, 78, 80, 83,
   85, 87, 89, 91, 93, 95, 97, 99,100,102,
  104,105,107,108,110,111,112,113,115,116,
  117,118,119,119,120,121,121,122,122,123,
  123,124,124,124,124,125,125,125,125,125,
  125,125,125,124,124,124,124,123,123,122,
  122
};

// -----------------------------------------------------------------------------
// helper
// -----------------------------------------------------------------------------

static inline uint8_t clamp8(int v) {
  if (v < 0)   return 0;
  if (v > 255) return 255;
  return (uint8_t)v;
}

static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return strip.Color(r, g, b);
}

static void clearStrip() {
  strip.clear();
}

static void showStrip() {
  strip.show();
}

static void fillAll(uint8_t r, uint8_t g, uint8_t b) {
  for (uint16_t i = 0; i < LEDSTRIP_COUNT; i++) {
    strip.setPixelColor(i, rgb(r, g, b));
  }
}

// VU szín: cián→zöld→sárga→narancs→piros spektrum (180°, jól látható, kevés kék)
// idx=0 (közép/csend) → cián/zöld, idx=total-1 (max) → piros
// Adafruit hue: 0=piros, 10922=sárga, 21845=zöld, 32768=cián
static uint32_t vuHsvColor(uint16_t idx, uint16_t total, uint8_t brightness) {
  if (total == 0) return rgb(0, 0, 0);
  uint16_t n = (total > 1) ? total - 1 : 1;
  // cián(32768) → piros(0): csökkenő hue – szép zöld-sárga-piros ív
  uint32_t hue = 32768UL - ((uint32_t)idx * 32768UL / n);
  // Telítettség: középen kicsit pasztell (200), külsőn teli (255)
  uint8_t sat = (uint8_t)(200 + (uint16_t)(idx * 55) / n);
  return strip.gamma32(strip.ColorHSV((uint16_t)hue, sat, brightness));
}

static void decayPeaks() {
  if (g_peakL > 0) g_peakL--;
  if (g_peakR > 0) g_peakR--;
}

static void renderStereoVU(uint8_t vuL, uint8_t vuR) {
  const uint16_t half       = LEDSTRIP_COUNT / 2;
  const uint16_t leftCount  = half;
  const uint16_t rightCount = LEDSTRIP_COUNT - half;

  uint16_t litL = map(vuL, 0, 180, 0, leftCount);
  uint16_t litR = map(vuR, 0, 180, 0, rightCount);

  if (litL > g_peakL) g_peakL = litL;
  if (litR > g_peakR) g_peakR = litR;

  clearStrip();

  // Bal: középtől kifelé (i=0 közel/csend, i=leftCount-1 külső/max)
  for (uint16_t i = 0; i < leftCount; i++) {
    uint16_t rev = leftCount - 1 - i;
    if (i < litL) {
      // Fényesebb kifelé: 160 → 255
      uint8_t br = (uint8_t)(160 + (uint16_t)(i * 95) / (leftCount > 1 ? leftCount - 1 : 1));
      strip.setPixelColor(rev, vuHsvColor(i, leftCount, br));
    } else if (i == litL && litL > 0) {
      // Lecsengő él-pixel: halvány "farok"
      strip.setPixelColor(rev, vuHsvColor(i, leftCount, 55));
    }
  }

  // Jobb: középtől kifelé
  for (uint16_t i = 0; i < rightCount; i++) {
    uint16_t idx = half + i;
    if (i < litR) {
      uint8_t br = (uint8_t)(160 + (uint16_t)(i * 95) / (rightCount > 1 ? rightCount - 1 : 1));
      strip.setPixelColor(idx, vuHsvColor(i, rightCount, br));
    } else if (i == litR && litR > 0) {
      strip.setPixelColor(idx, vuHsvColor(i, rightCount, 55));
    }
  }

  // Peak marker: élénk fehér + halványabb glow szomszéd
  if (g_peakL > 0 && g_peakL <= leftCount) {
    uint16_t pidx = leftCount - g_peakL;
    strip.setPixelColor(pidx, rgb(255, 255, 255));
    if (pidx + 1 < leftCount)
      strip.setPixelColor(pidx + 1, rgb(60, 60, 60));
  }
  if (g_peakR > 0 && g_peakR <= rightCount) {
    uint16_t pidx = half + (g_peakR - 1);
    strip.setPixelColor(pidx, rgb(255, 255, 255));
    if (pidx > half)
      strip.setPixelColor(pidx - 1, rgb(60, 60, 60));
  }

  showStrip();
  decayPeaks();
}

static void flashNow(uint8_t r, uint8_t g, uint8_t b, uint16_t ms = LEDSTRIP_FLASH_MS) {
  g_flashR = r;
  g_flashG = g;
  g_flashB = b;
  g_flashUntil = millis() + ms;
}

static bool isScreensaverMode() {

    if (config.isScreensaver)
        return true;

    if (display.mode() == SCREENSAVER)
        return true;

    if (display.mode() == SCREENBLANK)
        return true;

    return false;
}

static void renderFlash() {
  fillAll(g_flashR, g_flashG, g_flashB);
  showStrip();
}


// Sima sin-breathing egyszínű töltéssel (connecting, buffering, boot)
static void renderPulse(uint8_t r, uint8_t g, uint8_t b, uint8_t minBr, uint8_t maxBr, uint8_t step, uint16_t speedMs) {
  // Módváltáskor reset
  if (g_mode != g_lastMode) {
    g_pulseBrightness = minBr;
    g_pulseDir        = 1;
    g_lastMode        = g_mode;
  }

  if (millis() - g_lastPulse < speedMs) return;
  g_lastPulse = millis();

  g_pulseBrightness += g_pulseDir * step;
  if (g_pulseBrightness >= maxBr) { g_pulseBrightness = maxBr; g_pulseDir = -1; }
  if (g_pulseBrightness <= minBr) { g_pulseBrightness = minBr; g_pulseDir =  1; }

  uint8_t rr = (uint8_t)((r * g_pulseBrightness) / 255U);
  uint8_t gg = (uint8_t)((g * g_pulseBrightness) / 255U);
  uint8_t bb = (uint8_t)((b * g_pulseBrightness) / 255U);
  fillAll(rr, gg, bb);
  showStrip();
}

// Sin-alapú, lélegzetenként más szín (fix 16 szín, egyenletesen elosztva)
static uint32_t  g_lastBreath  = 0;
static uint8_t   g_ssColorIdx  = 0;   // 0..15, lélegzetenként lép

static void renderSinBreath() {
  if (g_mode != g_lastMode) {
    g_breathPhase = 0;
    g_ssColorIdx  = 0;
    g_lastMode    = g_mode;
  }

  if (millis() - g_lastBreath < LEDSTRIP_SCREEN_PULSE_MS) return;
  g_lastBreath = millis();

  uint16_t p = g_breathPhase;
  uint8_t  s;
  if (p < BREATH_STEPS) {
    s = pgm_read_byte(&sinTable[p]);
  } else {
    uint16_t mirror = (2 * BREATH_STEPS - 1) - p;
    s = pgm_read_byte(&sinTable[mirror]);
  }
  uint8_t br = 20 + (uint8_t)((uint16_t)s * 180U / 122U);

  g_breathPhase++;
  if (g_breathPhase >= 2 * BREATH_STEPS) {
    g_breathPhase = 0;
    g_ssColorIdx  = (g_ssColorIdx + 1) & 0x0F;  // 0..15
  }

  // 16 egyenletes hue lépés a teljes körön
  uint16_t hue = (uint16_t)g_ssColorIdx * (65536U / 16);
  uint32_t c = strip.ColorHSV(hue, 255, br);
  for (uint16_t i = 0; i < LEDSTRIP_COUNT; i++) {
    strip.setPixelColor(i, c);
  }
  showStrip();
}

static void renderRainbow() {
  for (uint16_t i = 0; i < LEDSTRIP_COUNT; i++) {
    uint16_t hue = g_rainbowIndex + (i * (65536UL / LEDSTRIP_COUNT));
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue)));
  }
  showStrip();
  g_rainbowIndex += 220;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Rainbow flow (model=1)
// - Szivárvány középről kifelé szimmetrikusan
// - VU lökés: a megjelenített sáv hossza a VU alapján változik (többi fekete)
//             + erős hang hirtelen hue ugrást ad (ritmus érzet)
// - VU nélkül: teljes szalag, lassú forgás
// -----------------------------------------------------------------------------

//Jak bedzie potzreba to u góry linia 92

// static void renderRainbowFlow() {
//   const uint32_t FLOW_MS = 20;
//   if (millis() - g_rainbowLastFrame < FLOW_MS) return;
//   g_rainbowLastFrame = millis();

//   uint8_t vuL = fusion_led_vu_left();
//   uint8_t vuR = fusion_led_vu_right();
//   uint8_t vuAvg = (uint8_t)(((uint16_t)vuL + vuR) >> 1);
//   bool    hasVU = (vuAvg > 8);

//   const uint16_t half = LEDSTRIP_COUNT / 2;

//   if (hasVU) {
//     // Megjelenített sáv hossza: VU alapján 10%..100% a félszalagból
//     uint16_t litHalf = 3 + (uint16_t)((uint32_t)vuAvg * (half - 3) / 255);

//     // Erős ütem → hue lökés (minél erősebb, annál nagyobb ugrás)
//     if (vuAvg > 180) {
//       g_rainbowIndex += (uint16_t)((uint32_t)(vuAvg - 180) * 1200U / 75U);
//     }

//     // Kirajzolás: lit pixelek szivárvány, a többi fekete
//     clearStrip();
//     for (uint16_t i = 0; i < litHalf; i++) {
//       uint16_t hue = g_rainbowIndex + (uint32_t)i * 65536UL / litHalf;
//       uint32_t c = strip.gamma32(strip.ColorHSV(hue, 255, 220));
//       strip.setPixelColor(half - 1 - i, c);
//       strip.setPixelColor(half + i,     c);
//     }
//     g_rainbowIndex += 400;  // gyors forgás lejátszás közben
//   } else {
//     // VU nélkül: teljes szalag, lassú egyenletes forgás
//     for (uint16_t i = 0; i < half; i++) {
//       uint16_t hue = g_rainbowIndex + (uint32_t)i * 65536UL / half;
//       uint32_t c = strip.gamma32(strip.ColorHSV(hue, 255, 160));
//       strip.setPixelColor(half - 1 - i, c);
//       strip.setPixelColor(half + i,     c);
//     }
//     g_rainbowIndex += 150;
//   }
//   showStrip();
// }

// qadd8 helper (saturating add, ha nincs FastLED)
static inline uint8_t qadd8(uint8_t a, uint8_t b) {
  uint16_t s = (uint16_t)a + b;
  return s > 255 ? 255 : (uint8_t)s;
}

// -----------------------------------------------------------------------------
// Fire (model=2)
// Klasszikus heat-map tűz, FIRE_HALF méretű tömb, középről kifelé tükrözve.
// Heatcolor: fekete(0) → sötétvörös(85) → narancsvörös(170) → sárga(255)
// VU: erős hang → több szikra + kisebb hűtés → magasabb lángok
// -----------------------------------------------------------------------------
static inline uint32_t heatColor(uint8_t t) {
  // 3 egyenlő sávra osztva: 0..84, 85..169, 170..255
  uint8_t r, g, b;
  if (t < 85) {
    // fekete → sötétvörös
    r = t * 3;       g = 0;           b = 0;
  } else if (t < 170) {
    // sötétvörös → narancsvörös
    r = 255;         g = (t - 85) * 2; b = 0;
  } else {
    // narancsvörös → sárga (zöld csatorna nő, kék marad 0)
    r = 255;         g = 170 + (t - 170); b = 0;
  }
  return strip.Color(r, g, b);
}

static void renderFire() {
  const uint32_t FIRE_MS = 30;
  if (millis() - g_fireLastFrame < FIRE_MS) return;
  g_fireLastFrame = millis();

  uint8_t vuL   = fusion_led_vu_left();
  uint8_t vuR   = fusion_led_vu_right();
  uint8_t vuAvg = (uint8_t)(((uint16_t)vuL + vuR) >> 1);

  // 1. Hűtés – per pixel, frame-enkénti fix levonás
  //    VU erős → kisebb cooling → magasabb lángok
  //    cooling=8 → ~1 másodperc alatt hűl le egy 255-ös pixel (30fps-nél)
  uint8_t cooling = 12 - (uint8_t)((uint32_t)vuAvg * 7U / 255U);  // 5..12
  for (uint8_t i = 0; i < FIRE_HALF; i++) {
    uint8_t cool = (uint8_t)random(cooling / 2, cooling + 1);
    g_heat[i] = (g_heat[i] > cool) ? g_heat[i] - cool : 0;
  }

  // 2. Konvekció: hő terjed "felfelé" (közép felé)
  for (uint16_t i = FIRE_HALF - 1; i >= 2; i--) {
    g_heat[i] = ((uint16_t)g_heat[i-1] + g_heat[i-2] + g_heat[i-2]) / 3;
  }

  // 3. Szikrák az alján (szél = FIRE_HALF-1 index)
  //    VU nélkül: kis alaplángok, VU ütemre: nagy szikrák
  uint8_t sparkChance = (vuAvg > 20) ? 1 : 2;   // 1=50%, 2=33%
  uint8_t sparkMin    = 120 + (uint8_t)((uint32_t)vuAvg * 80U / 255U);   // 120..200
  uint8_t sparkMax    = 180 + (uint8_t)((uint32_t)vuAvg * 75U / 255U);   // 180..255
  if (random(0, sparkChance + 1) == 0) {
    uint8_t pos = random(0, min((uint8_t)5, (uint8_t)FIRE_HALF));
    g_heat[pos] = qadd8(g_heat[pos], random(sparkMin, sparkMax + 1));
  }
  // Erős ütem: extra szikra lökés
  if (vuAvg > 200) {
    uint8_t pos2 = random(0, min((uint8_t)3, (uint8_t)FIRE_HALF));
    g_heat[pos2] = qadd8(g_heat[pos2], random(200, 256));
  }

  // 4. Megjelenítés: i=0 a szél (forró), i=FIRE_HALF-1 a közép (hűvös)
  //    tükrözve: bal fél és jobb fél
  const uint16_t half = LEDSTRIP_COUNT / 2;
  for (uint8_t i = 0; i < FIRE_HALF; i++) {
    uint32_t c = heatColor(g_heat[i]);
    // i=0 → szél (half-1 és half index), i=FIRE_HALF-1 → közép
    strip.setPixelColor(half - 1 - i, c);
    strip.setPixelColor(half + i,     c);
  }
  showStrip();
}

// -----------------------------------------------------------------------------
// Analog (model=3)
// Analóg műszer feeling: fehér sáv + piros csúcsmutató
// L csatorna: bal fél (0 → half-1), R csatorna: jobb fél (half → end)
// Csúcs visszaesés: ~0.5mp (≈ 2.6 pixel/frame @ 18ms)
// -----------------------------------------------------------------------------
static void renderMeterVU() {
  uint8_t vuL = fusion_led_vu_left();
  uint8_t vuR = fusion_led_vu_right();

  const uint16_t half  = LEDSTRIP_COUNT / 2;
  const float    decay = (float)half / (500.0f / LEDSTRIP_FRAME_MS);  // px/frame

  // Aktuális sáv pixel-hossz
  float litLf = (float)vuL * half / 255.0f;
  float litRf = (float)vuR * half / 255.0f;
  uint16_t litL = (uint16_t)litLf;
  uint16_t litR = (uint16_t)litRf;

  // Peak: csak felfelé ugrik, lefelé lassan esik
  if (litLf > g_meterPeakL) g_meterPeakL = litLf;
  else                       g_meterPeakL -= decay;
  if (g_meterPeakL < 0.0f)  g_meterPeakL = 0.0f;

  if (litRf > g_meterPeakR) g_meterPeakR = litRf;
  else                       g_meterPeakR -= decay;
  if (g_meterPeakR < 0.0f)  g_meterPeakR = 0.0f;

  uint16_t peakL = (uint16_t)g_meterPeakL;
  uint16_t peakR = (uint16_t)g_meterPeakR;
  if (peakL >= half)  peakL = half - 1;
  if (peakR >= half)  peakR = half - 1;

  clearStrip();

  // Bal fél: 0 = csend (bal széle), half-1 = max → balról jobbra nő
  for (uint16_t i = 0; i < half; i++) {
    if (i < litL)
      strip.setPixelColor(i, rgb(220, 220, 220));
  }
  // Bal csúcs: piros mutató
  if (peakL > 0) {
    uint16_t pidx = peakL < half ? peakL : half - 1;
    strip.setPixelColor(pidx, rgb(255, 0, 0));
    if (pidx + 1 < half)
      strip.setPixelColor(pidx + 1, rgb(180, 0, 0));
  }

  // Jobb fél: half = csend (közép), LEDSTRIP_COUNT-1 = max → balról jobbra nő
  for (uint16_t i = 0; i < half; i++) {
    if (i < litR)
      strip.setPixelColor(half + i, rgb(220, 220, 220));
  }
  // Jobb csúcs: piros mutató
  if (peakR > 0) {
    uint16_t pidx = half + (peakR < half ? peakR : half - 1);
    strip.setPixelColor(pidx, rgb(255, 0, 0));
    if (pidx + 1 < (uint16_t)LEDSTRIP_COUNT)
      strip.setPixelColor(pidx + 1, rgb(180, 0, 0));
  }

  showStrip();
}

static void renderVolumeBar(uint8_t vol) {
  uint16_t lit = map(vol, 0, 100, 0, LEDSTRIP_COUNT);
  clearStrip();
  for (uint16_t i = 0; i < LEDSTRIP_COUNT; i++) {
    if (i < lit) {
      strip.setPixelColor(i, vuHsvColor(i, LEDSTRIP_COUNT, 220));
    }
  }
  showStrip();
}

static void renderStop() {
  clearStrip();
  showStrip();
}

static void renderConnecting() {
  renderPulse(0, 0, 255, 8, 90, 2, LEDSTRIP_CONNECT_PULSE_MS);
}

static void renderScreensaver() {
  if (!config.store.lsSsEnabled) {
    strip.clear();
    strip.show();
    return;
  }
  renderSinBreath();
}

static void renderBuffering() {
  renderPulse(255, 110, 0, 8, 120, 2, LEDSTRIP_IDLE_PULSE_MS);
}

// ============================================================
// NOWE EFEKTY
// ============================================================

static void renderVuRainbow() {
    static float currentBr = 0;
    static uint16_t currentHue = 0;

    uint8_t vuL = fusion_led_vu_left();
    uint8_t vuR = fusion_led_vu_right();
    uint8_t vu  = (vuL > vuR) ? vuL : vuR;

    // Próg odcięcia (Noise Gate) - poniżej 15 nic się nie dzieje (cisza w LEDach)
    if (vu < 15) vu = 0; 

    // REAKCJA IMPULSOWA:
    // Jeśli uderzenie jest silniejsze niż obecna jasność -> natychmiast skaczemy do nowej wartości
    if (vu > currentBr) {
        currentBr = vu;
        // Przy każdym mocnym uderzeniu (impulsie) zmieniamy gwałtownie kolor
        currentHue += 5000; 
    } else {
        // SZYBKIE WYGASZANIE (Linear decay) - dla efektu ostrego impulsu
        currentBr -= 8.0; 
        if (currentBr < 0) currentBr = 0;
    }

    uint8_t br = (uint8_t)currentBr;

    for (uint16_t i = 0; i < LEDSTRIP_COUNT; i++) {
        // Wszystkie 3 diody mrugają tym samym kolorem dla potężniejszego efektu
        // lub dodaj (i * 2000) jeśli chcesz lekki gradient w plastiku
        uint16_t h = currentHue + (i * 2000); 
        strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(h, 255, br)));
    }
    showStrip();
}



// static void renderVuRainbow() {
//     static uint16_t hueOffset = 0;   // Kolor tęczy
//     static float smoothedVu = 0;     // Płynność ruchu

//     uint8_t vuL = fusion_led_vu_left();
//     uint8_t vuR = fusion_led_vu_right();
//     uint8_t vu  = (vuL > vuR) ? vuL : vuR;

//     // Szybkie narastanie, powolne opadanie (efekt płynnego błysku gałki)
//     if (vu > smoothedVu) smoothedVu = vu;
//     else smoothedVu -= 1.5; // prędkość wygaszania
//     if (smoothedVu < 0) smoothedVu = 0;

//     // Mapowanie jasności: od lekkiej poświaty (10) do pełnej mocy (255)
//     uint8_t bright = map((uint8_t)smoothedVu, 0, 255, 10, 255);

//     // Im mocniejszy bas, tym szybciej gałka "kręci" kolorami
//     hueOffset += map((uint8_t)smoothedVu, 0, 255, 200, 2500);

//     for (uint16_t i = 0; i < LEDSTRIP_COUNT; i++) {
//         // i * 15000 sprawia, że kolory są rozłożone symetrycznie wokół gałki
//         uint16_t hue = hueOffset + (i * 15000); 
//         strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue, 255, bright)));
//     }
//     showStrip();
// }


// static void renderVuRainbow() {
//     static uint16_t j = 0;           // zmienna do przesuwania kolorów tęczy
//     static uint8_t smoothedVu = 0;   // wygładzone wartości VU, żeby nie migotało zbyt ostro

//     uint8_t vuL = fusion_led_vu_left();
//     uint8_t vuR = fusion_led_vu_right();
//     uint8_t vu  = (vuL > vuR) ? vuL : vuR;

//     // Wygładzanie (prosta filtracja), żeby ruch był płynniejszy
//     smoothedVu = (vu > smoothedVu) ? vu : (smoothedVu * 0.9); 

//     // Jeśli muzyka jest bardzo cicha, lekko żarzymy diody (jasność 5)
//     uint8_t bright = map(smoothedVu, 0, 255, 5, 255);

//     // Przesuwamy tęczę - im głośniej, tym szybciej zmieniają się kolory
//     j += map(smoothedVu, 0, 255, 100, 2000); 

//     for (uint16_t i = 0; i < LEDSTRIP_COUNT; i++) {
//         // Każda z 3 diod dostaje nieco inny odcień (i * 2000), tworząc przejście kolorów
//         uint16_t hue = j + (i * 2000); 
//         strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue, 255, bright)));
//     }

//     showStrip();
// }



// static void renderVuRainbow() {
//     uint8_t vuL = fusion_led_vu_left();
//     uint8_t vuR = fusion_led_vu_right();
//     uint8_t vu  = (vuL > vuR) ? vuL : vuR;

//     static uint32_t last = 0;
//     if (millis() - last < 80) return;  // co 80ms nowy kolor
//     last = millis();

//     for (uint16_t i = 0; i < LEDSTRIP_COUNT; i++) {
//         uint8_t bright = map(vu, 0, 255, 10, 255);
//         uint16_t hue = random(0, 65535);
//         strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue, 255, bright)));
//     }
//     showStrip();
// }


// static void renderVuRainbow() {
//     uint8_t vuL = fusion_led_vu_left();
//     uint8_t vuR = fusion_led_vu_right();
//     uint8_t vu  = (vuL > vuR) ? vuL : vuR;
//     g_vuRainbowIndex += 200;
//     for (uint16_t i = 0; i < LEDSTRIP_COUNT; i++) {
//         uint16_t hue = g_vuRainbowIndex + (i * (65536UL / LEDSTRIP_COUNT));
//         uint8_t bright = map(vu, 0, 255, 20, 255);
//         uint32_t color = strip.ColorHSV(hue, 255, bright);
//         strip.setPixelColor(i, strip.gamma32(color));
//     }
//     showStrip();
// }

static void renderPauseFade() {
    static uint8_t br  = 5;
    static int8_t  dir = 1;
    static uint32_t last = 0;
    if (millis() - last < 20) return;
    last = millis();
    br += dir * 3;
    if (br >= 180) { br = 180; dir = -1; }
    if (br <= 5)   { br = 5;   dir =  1; }
    fillAll(br, 0, 0);
    showStrip();
}

static void renderVolumePulse() {
    static uint8_t br  = 10;
    static int8_t  dir = 1;
    static uint32_t last = 0;
    if (millis() - last < 15) return;
    last = millis();
    br += dir * 8;
    if (br >= 220) { br = 220; dir = -1; }
    if (br <= 10)  { br = 10;  dir =  1; }
    if (g_volUp) {
        fillAll(br, 0, 0);
    } else {
        fillAll(0, br, 0);
    }
    showStrip();
}

static void renderStationsList() {
    fillAll(0, 0, 180);
    showStrip();
}

static void renderStationScrolling() {
    static uint8_t phase = 0;
    static uint32_t last = 0;
    if (millis() - last < 120) return;
    last = millis();
    phase = (phase + 1) % 3;
    switch (phase) {
        case 0: fillAll(0, 0, 200); break;
        case 1: fillAll(200, 0, 0); break;
        case 2: fillAll(0, 200, 0); break;
    }
    showStrip();
}

static void renderEncPulse() {
    static uint8_t br  = 255;
    static uint32_t last = 0;
    if (millis() - last < 10) return;
    last = millis();
    if (br > 8) br -= 8; else br = 0;
    uint8_t r = (g_encPulseR * br) / 255;
    uint8_t g = (g_encPulseG * br) / 255;
    uint8_t b = (g_encPulseB * br) / 255;
    fillAll(r, g, b);
    showStrip();
}

static void renderPlay() {
  switch (config.store.lsModel) {
    case 1:
      renderVuRainbow();  // nowy efekt - tęcza pod rytm muzyki
      break;
    case 2:
      renderFire();
      break;
    case 3:
      renderMeterVU();
      break;
    default:
    case 0: {
      // oryginalny efekt - VU stereo + tęcza gdy cicho
      uint8_t vuL = fusion_led_vu_left();
      uint8_t vuR = fusion_led_vu_right();
      if (vuL > 0 || vuR > 0) {
        renderStereoVU(vuL, vuR);
      } else {
        renderRainbow();
      }
      break;
    }
  }
}

static void updateModeFromRuntime() {
  if (isScreensaverMode()) {
    g_mode = LM_SCREENSAVER;
    return;
  }

  if (millis() < g_volumeUntil) {
    g_mode = LM_VOLUME;
    return;
  }

  if (network.status != CONNECTED && network.status != SDREADY) {
    g_mode = LM_CONNECTING;
    return;
  }

  int s = player.status();

  if (s == PR_PLAY) {
    g_mode = LM_PLAY;
  } else if (s == PR_STOP) {
    g_mode = LM_STOP;
  } else {
    g_mode = LM_BUFFERING;
  }
}

// -----------------------------------------------------------------------------
// plugin
// -----------------------------------------------------------------------------

LedStripPlugin ledStripPlugin;   // globális példány

LedStripPlugin::LedStripPlugin() {}

void ledstripPluginInit() {
    pm.add(&ledStripPlugin);
}

void ledstripOff() {
    // 1. Pobieramy aktualną jasność zapisaną w konfiguracji
    uint8_t currentBrt = strip.getBrightness();

    // 2. Stopniowo schodzimy do zera
    while (currentBrt > 0) {
        if (currentBrt > 5) currentBrt -= 5; // Zmniejszaj o 5 w każdym kroku
        else currentBrt = 0;

        strip.setBrightness(currentBrt);
        strip.show();
        delay(25); // Czas między krokami (ok. 15ms daje płynny efekt)
    }

    // 3. Blokujemy dalsze świecenie przez logikę pluginu
    config.store.lsEnabled = false; 
    
    // 4. Dla pewności czyścimy pasek i wysyłamy ostatni sygnał
    strip.clear();
    strip.show();
    
    Serial.println("[LED] Stopniowe wygaszanie zakończone");
}


// void ledstripOff() {
//     config.store.lsEnabled = false; // Wyłączamy pasek w ustawieniach (tymczasowo w RAM)
//     strip.clear();
//     strip.show();
//     Serial.println("[LED] Pasek wyłączony przez pilota IR");
// }

void LedStripPlugin::on_setup() {
  Serial.println("[LED] on_setup()");
  strip.begin();
  strip.setBrightness(map(config.store.lsBrightness, 0, 100, 0, 255));
  strip.clear();
  strip.show();

  g_mode = LM_BOOT;
  g_lastVolume = config.store.volume;

  fillAll(10, 10, 10);
  showStrip();
}

void LedStripPlugin::on_connect() {
  g_connectedSeen = true;
  flashNow(0, 80, 255, 220);
}

void LedStripPlugin::on_start_play() {
  Serial.println("[LED] on_start_play()");
  g_mode = LM_PLAY;
  flashNow(0, 180, 40, 140);
}

void LedStripPlugin::on_stop_play() {
  Serial.println("[LED] on_stop_play()");
  g_mode = LM_STOP;
  fillAll(180, 0, 0);
  showStrip();
}

void LedStripPlugin::on_station_change() {
  flashNow(0, 220, 180, 180);
}

void LedStripPlugin::on_track_change() {
  flashNow(255, 255, 255, 120);
}

void LedStripPlugin::on_ticker() {
  uint8_t vol = config.store.volume;
  if (vol != g_lastVolume) {
    g_volUp = (vol > g_lastVolume);  // kierunek zmiany
    g_lastVolume = vol;
    g_volumeUntil = millis() + LEDSTRIP_VOL_TIMEOUT_MS;
    g_volPulseUntil = millis() + 1500;  // 1.5 sek pulsowania
  }
}

void LedStripPlugin::on_btn_click(controlEvt_e& btnid) {
  // Kliknięcie enkodera - pulsowanie losowym kolorem
  if (btnid == EVT_ENCBTNB || btnid == EVT_BTNCENTER) {
    g_encPulseR = random(50, 255);
    g_encPulseG = random(50, 255);
    g_encPulseB = random(50, 255);
    g_encPulseUntil = millis() + 600;
  }
}

void LedStripPlugin::on_display_queue(requestParams_t& request, bool& result) {
  // Wykrywanie wejścia/wyjścia z listy stacji
  if (request.type == NEWMODE) {
    g_stationsList = (request.payload == STATIONS);
  }
  // Wykrywanie przewijania listy (DRAWPLAYLIST)
  if (request.type == DRAWPLAYLIST) {
    g_stationScrollUntil = millis() + 600;  // 600ms migania przy każdym ruchu
  }
}

void LedStripPlugin::on_loop() {
  // Plugin wyłączony z WebUI
  if (!config.store.lsEnabled) {
    strip.clear();
    strip.show();
    return;
  }

  // Aktualizacja jasności
  static uint8_t s_lastBr = 255;
  if (config.store.lsBrightness != s_lastBr) {
    s_lastBr = config.store.lsBrightness;
    strip.setBrightness(map(s_lastBr, 0, 100, 0, 255));
  }

  if (millis() - g_lastFrame < LEDSTRIP_FRAME_MS) return;
  g_lastFrame = millis();

  // Flash ma priorytet (zmiana stacji, utworu itp.)
  if (millis() < g_flashUntil) {
    renderFlash();
    return;
  }

  // Pulsowanie po kliknięciu enkodera
  if (millis() < g_encPulseUntil) {
    renderEncPulse();
    return;
  }

  // Lista stacji - niebieski lub miganie przy wyborze
  if (g_stationsList) {
    if (millis() < g_stationScrollUntil) {
      renderStationScrolling();
    } else {
      renderStationsList();
    }
    return;
  }

  // Pulsowanie przy zmianie głośności
  if (millis() < g_volPulseUntil) {
    renderVolumePulse();
    return;
  }

  updateModeFromRuntime();

  switch (g_mode) {
    case LM_BOOT:
      renderPulse(32, 32, 32, 4, 20, 1, 40);
      break;

    case LM_CONNECTING:
      renderConnecting();
      break;

    case LM_STOP:
      // Pauza - fade czerwony
      renderPauseFade();
      break;

    case LM_PLAY:
      renderPlay();
      break;

    case LM_BUFFERING:
      renderBuffering();
      break;

    case LM_VOLUME:
      renderVolumeBar(g_lastVolume);
      break;

    case LM_SCREENSAVER:
      renderScreensaver();
      break;

    default:
      renderStop();
      break;
  }
}
#endif