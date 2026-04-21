//----------------------------------------------------------------------------------------------------------------
//    This file was generated on the website https://vip-cxema.org/
//    Program version: 1.2.0_03.06.2025
//    File last modified: 10:45 29.03.2026
//----------------------------------------------------------------------------------------------------------------
//    Project home       https://github.com/e2002/yoradio
//    Wiki               https://github.com/e2002/yoradio/wiki
//    Описание на 4PDA   https://4pda.to/forum/index.php?s=&showtopic=1010378&view=findpost&p=112992611
//    Как это прошить?   https://4pda.to/forum/index.php?act=findpost&pid=112992611&anchor=Spoil-112992611-2
//----------------------------------------------------------------------------------------------------------------
#ifndef _my_theme_h
#define _my_theme_h
//----------------------------------------------------------------------------------------------------------------
//    Theming of color displays
//    DSP_ST7735, DSP_ST7789, DSP_ILI9341, DSP_GC9106, DSP_ILI9225, DSP_ST7789_240
//----------------------------------------------------------------------------------------------------------------
//    *    !!! This file must be in the root directory of the sketch !!!    *
//----------------------------------------------------------------------------------------------------------------
//    Uncomment (remove double slash //) from desired line to apply color
//----------------------------------------------------------------------------------------------------------------
#define ENABLE_THEME
#ifdef  ENABLE_THEME
/*----------------------------------------------------------------------------------------------------------------*/
/*       | COLORS             |   values (0-255)  |                                                               */
/*       | color name         |    R    G    B    |                                                               */
/*----------------------------------------------------------------------------------------------------------------*/
#define COLOR_BACKGROUND        0,0,0     /*  background                                                  */
#define COLOR_STATION_NAME        23,54,255     /*  station name                                                */
#define COLOR_STATION_BG         0,0,0     /*  station name background                                     */
#define COLOR_STATION_FILL       0,0,0     /*  station name fill background                                */
#define COLOR_SNG_TITLE_1       80,255,150     /*  first title                                                 */
#define COLOR_SNG_TITLE_2         181,230,29    /*  second title                                                */
#define COLOR_WEATHER           255,124,84     /*  weather string                                              */
#define COLOR_VU_MAX            236,28,35     /*  max of VU meter                                             */
#define COLOR_VU_MIN            45,236,100     /*  min of VU meter                                             */
#define COLOR_CLOCK              61,216,184     /*  clock color                                                 */
#define COLOR_CLOCK_BG           7,7,7     /*  clock color background                                      */
#define COLOR_SECONDS             236,28,35     /*  seconds color (DSP_ST7789, DSP_ILI9341, DSP_ILI9225)        */
#define COLOR_DAY_OF_W          5,13,255     /*  day of week color (DSP_ST7789, DSP_ILI9341, DSP_ILI9225)    */
#define COLOR_DATE                255,126,38     /*  date color (DSP_ST7789, DSP_ILI9341, DSP_ILI9225)           */
#define COLOR_HEAP              85,139,221     /*  heap string                                                 */
#define COLOR_BUFFER            61,216,184     /*  buffer line                                                 */
#define COLOR_IP                 195,195,195    /*  ip address                                                  */
#define COLOR_VOLUME_VALUE      236,28,35     /*  volume string (DSP_ST7789, DSP_ILI9341, DSP_ILI9225)        */
#define COLOR_RSSI              63,71,204     /*  rssi                                                        */
#define COLOR_VOLBAR_OUT        24,45,231     /*  volume bar outline                                          */
#define COLOR_VOLBAR_IN         236,28,35     /*  volume bar fill                                             */
#define COLOR_DIGITS            44,155,132     /*  volume / station number                                     */
#define COLOR_DIVIDER             59,182,34     /*  divider color (DSP_ST7789, DSP_ILI9341, DSP_ILI9225)        */
#define COLOR_BITRATE           255,126,38     /*  bitrate                                                     */
#define COLOR_PL_CURRENT          0,0,0     /*  playlist current item                                       */
#define COLOR_PL_CURRENT_BG      0,0,0     /*  playlist current item background                            */
#define COLOR_PL_CURRENT_FILL    0,0,0     /*  playlist current item fill background                       */
#define COLOR_PLAYLIST_0        31,27,255     /*  playlist string 0                                           */
#define COLOR_PLAYLIST_1        27,75,255     /*  playlist string 1                                           */
#define COLOR_PLAYLIST_2        56,56,56     /*  playlist string 2                                           */
#define COLOR_PLAYLIST_3          35,35,35     /*  playlist string 3                                           */
#define COLOR_PLAYLIST_4          25,25,25     /*  playlist string 4                                           */


#endif  /* #ifdef  ENABLE_THEME */
#endif  /* #define _my_theme_h  */
