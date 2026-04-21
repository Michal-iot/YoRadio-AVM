/* Wygenerowano przez generator myoptions.h - v0.9.0 */
/* https://trip5.github.io/ehRadio_myoptions/generator.html */
/* https://github.com/e2002/yoradio */

/* esp32-S3-devkit-C1 44 piny https://randomnerdtutorials.com/esp32-s3-devkitc-pinout-guide */

/* HSPI (SPI 2) GPIO 11 - MOSI, GPIO 12 - CLK, GPIO 13 - MISO, GPIO 10 - CS */

#ifndef myoptions_h
#define myoptions_h

#define L10N_LANGUAGE PL // HU NL PL RU EN EL Wpisz tutaj swój język
#define NAMEDAYS_FILE PL // HU, PL, NL
#define CLOCK_TTS_LANGUAGE "pl" // Domyślny język TTS np. pl, en, de, ru, fr, hu
//#define USDATE   // Format daty USA
//#define IMPERIALUNIT

// --- DLNA / Synology ---
///#define USE_DLNA
// #define dlnaHost "192.168.180.122" // Adres IP serwera DLNA
// #define dlnaIDX  21  // Indeks kontenera MUZYKA, na Synology NAS zazwyczaj 21

#define USE_LEDSTRIP_PLUGIN
#define LEDSTRIP_PIN    8
//#define LEDSTRIP_COUNT  33 // 3 //35
//#define LED_BRIGHTNESS 64   // Jasność (0-255) - 64 to bezpieczne 25%

// Wspólny pin (gałka + matryca w jednym łańcuchu):
#define VU_MATRIX_SHARED_PIN
#define LEDSTRIP_COUNT  35    // 3 + 32 (4x8 testowo)
#define VU_MATRIX_COLS  4

// Lub osobny pin:
// #define LEDSTRIP2_PIN   15
// #define VU_MATRIX_COLS  4

/******************************************/
// #define LED_BUILTIN_S3    48     /* Pin wbudowanej diody RGB S3 */
#define USE_BUILTIN_LED true /* Wbudowana dioda RGB nie świeci */
/*****************************************/
//#define DSP_MODEL DSP_ILI9486
//#define DSP_MODEL DSP_ILI9488
//#define DSP_MODEL DSP_ILI9341
//#define DSP_MODEL DSP_ST7735
//#define DSP_MODEL DSP_ST7789_76
//#define DSP_MODEL DSP_1602I2C    // Wyświetlacz 2x16
//#define DSP_MODEL DSP_2004I2C    // Wyświetlacz 4x20
#define DSP_MODEL DSP_ST7789
//#define DSP_MODEL DSP_SH1106
//#define SH1106_GRAYSCALE  true
//#define DSP_MODEL DSP_SSD1306
//#define DSP_MODEL DSP_GC9A01A
//#define HUN_LCD
/*****************************************/

#define TFT_DC         9
#define TFT_CS         10
#define TFT_RST        -1
#define BRIGHTNESS_PIN 14
/*#define TFT_SCK   12
#define TFT_MOSI  11
#define TFT_MISO  13*/

/*****************************************/
// #define NEXTION_RX      15   // Nextion - odbiór
// #define NEXTION_TX      16   // Nextion - nadawanie
/*****************************************/
/* Panel dotykowy */

#define TS_MODEL TS_MODEL_XPT2046
#define TS_CS 3
/*****************************************/
/* Karta SD */
// #define SDC_CS    39
/****************************************/
/* Przetwornik cyfrowo-analogowy I2S */

/* DAC PCM5102A */
#define I2S_DOUT 4
#define I2S_BCLK 5
#define I2S_LRC  6

/* ENKODER 1 */
#define ENC_BTNR 41 // S2
#define ENC_BTNL 40 // S1
#define ENC_BTNB 7 // KEY 39
// #define ENC_INTERNALPULLUP		true

/* ENKODER 2 */
// #define ENC2_BTNR 7 // S2
// #define ENC2_BTNL 16 // S1
// #define ENC2_BTNB 15 // Przycisk
// #define ENC2_INTERNALPULLUP		true
/********************************************/

/* Moduł zegara RTC DS3231 */
// #define RTC_SCL     9
// #define RTC_SDA     8
// #define RTC_MODULE DS3231
/********************************************/

/* ============================================================
   PEŁNOEKRANOWY ZEGAR (zajmuje cały ekran)
   ============================================================ */
 #define FULL_SCR_CLOCK  true

/* Odbiornik podczerwieni pilota */
#define IR_PIN 2
#define WAKE_PIN1      2   // Pin wybudzenia 1
#define WAKE_PIN2      7  // Pin wybudzenia 2

/********************************************/

/********************************************/
/* Pozostałe ustawienia */
#define RSSI_DIGIT       true
//#define MUTE_PIN     2            /* Pin wyciszenia */
//#define MUTE_VAL    LOW           /* Stan pinu wyciszenia gdy odtwarzanie zatrzymane */
//#define PLAYER_FORCE_MONO false   /* Mono przy starcie - false stereo, true mono */
#define I2S_INTERNAL    false     /* Jeśli true - używa wewnętrznego DAC ESP32 */
//#define ROTATE_90   false         /* Opcjonalny obrót o 90 stopni dla kwadratowych wyświetlaczy */
//#define TFT_ROTATE      0         /* Obrót wyświetlacza: 0 - 0 stopni, 1 - 90, 2 - 180, 3 - 270 */
//#define HIDE_VOLPAGE              /* Ukryj ekran głośności, nawiguj paskiem postępu głośności */
//#define WAKE_PIN      1
//#define LIGHT_SENSOR      40      /* Czujnik światła */
//#define AUTOBACKLIGHT(x)  *funkcja* /* Funkcja automatycznej regulacji podświetlenia - patrz options.h */
//#define NAME_STRIM              /* Wyświetlaj nazwę stacji ze strumienia */

//#define DOWN_LEVEL           2      /* Minimalny poziom jasności (od 0 do 255) */
//#define DOWN_INTERVAL        60     /* Czas do przyciemnienia w sekundach (60 sek = 1 min) */
/* ***************************************** */

#endif
