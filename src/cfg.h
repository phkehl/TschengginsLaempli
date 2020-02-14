/*!
    \file
    \brief flipflip's Tschenggins Lämpli: system config (see \ref FF_CFG)

    - Copyright (c) 2018-2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/tschenggins-laempli

    \defgroup FF_CFG CFG
    \ingroup FF

    @{
*/
#ifndef __CFG_H__
#define __CFG_H__

#include <Arduino.h>

//! initialise
void cfgInit(void);

typedef enum CFG_MODEL_e
{
    CFG_MODEL_UNKNOWN,
    CFG_MODEL_STANDARD,
    CFG_MODEL_CHEWIE,
    CFG_MODEL_HELLO,
    CFG_MODEL_GITTA,
} CFG_MODEL_t;

#define CFG_MODEL_UNKNOWN_STR    "unknown"
#define CFG_MODEL_STANDARD_STR   "standard"
#define CFG_MODEL_CHEWIE_STR     "chewie"
#define CFG_MODEL_HELLO_STR      "hello"
#define CFG_MODEL_GITTA_STR      "gitta"

typedef enum CFG_DRIVER_e
{
    CFG_DRIVER_UNKNOWN,
    CFG_DRIVER_WS2801,
  //CFG_DRIVER_WS2812,
    CFG_DRIVER_SK9822,
} CFG_DRIVER_t;

#define CFG_DRIVER_UNKNOWN_STR   "unknown"
#define CFG_DRIVER_WS2801_STR    "WS2801"
#define CFG_DRIVER_WS2812_STR    "WS2812"
#define CFG_DRIVER_SK9822_STR    "SK9822"

typedef enum CFG_ORDER_e
{
    CFG_ORDER_UNKNOWN,
    CFG_ORDER_RGB,
    CFG_ORDER_RBG,
    CFG_ORDER_GRB,
    CFG_ORDER_GBR,
    CFG_ORDER_BRG,
    CFG_ORDER_BGR,
} CFG_ORDER_t;

#define CFG_ORDER_UNKNOWN_STR    "unknown"
#define CFG_ORDER_RGB_STR        "RGB"
#define CFG_ORDER_RBG_STR        "RBG"
#define CFG_ORDER_GRB_STR        "GRB"
#define CFG_ORDER_GBR_STR        "GBR"
#define CFG_ORDER_BRG_STR        "BRG"
#define CFG_ORDER_BGR_STR        "BGR"

typedef enum CFG_BRIGHT_e
{
    CFG_BRIGHT_UNKNOWN,
    CFG_BRIGHT_LOW,
    CFG_BRIGHT_MEDIUM,
    CFG_BRIGHT_HIGH,
    CFG_BRIGHT_FULL,
} CFG_BRIGHT_t;

#define CFG_BRIGHT_UNKNOWN_STR   "unknown"
#define CFG_BRIGHT_LOW_STR       "low"
#define CFG_BRIGHT_MEDIUM_STR    "medium"
#define CFG_BRIGHT_HIGH_STR      "high"
#define CFG_BRIGHT_FULL_STR      "full"

typedef enum CFG_NOISE_e
{
    CFG_NOISE_UNKNOWN,
    CFG_NOISE_NONE,
    CFG_NOISE_SOME,
    CFG_NOISE_MORE,
    CFG_NOISE_MOST,
} CFG_NOISE_t;

#define CFG_NOISE_UNKNOWN_STR    "unknown"
#define CFG_NOISE_NONE_STR       "none"
#define CFG_NOISE_SOME_STR       "some"
#define CFG_NOISE_MORE_STR       "more"
#define CFG_NOISE_MOST_STR       "most"

CFG_MODEL_t  cfgGetModel(void);
CFG_DRIVER_t cfgGetDriver(void);
CFG_ORDER_t  cfgGetOrder(void);
CFG_BRIGHT_t cfgGetBright(void);
CFG_NOISE_t  cfgGetNoise(void);

void cfgSetNoise(CFG_NOISE_t noise);

bool cfgApply(const char *json);


#endif // __CFG_H__
//@}
// eof
