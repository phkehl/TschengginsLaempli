/*!
    \file
    \brief flipflip's Tschenggins Lämpli: system config (noises and LED) (see \ref FF_CFG)

    - Copyright (c) 2018-2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/tschenggins-laempli
*/

#include <ArduinoJson.h>

#include "debug.h"
#include "status.h"

#include "cfg.h"

static CFG_MODEL_t  sCfgModel;
static CFG_DRIVER_t sCfgDriver;
static CFG_ORDER_t  sCfgOrder;
static CFG_BRIGHT_t sCfgBright;
static CFG_NOISE_t  sCfgNoise;

CFG_MODEL_t  cfgGetModel(void)  { return sCfgModel; }
CFG_DRIVER_t cfgGetDriver(void) { return sCfgDriver; }
CFG_ORDER_t  cfgGetOrder(void)  { return sCfgOrder; }
CFG_BRIGHT_t cfgGetBright(void) { return sCfgBright; }
CFG_NOISE_t  cfgGetNoise(void)  { return sCfgNoise; }

void cfgSetNoise(CFG_NOISE_t noise)
{
    switch (noise)
    {
        case CFG_NOISE_NONE:
        case CFG_NOISE_SOME:
        case CFG_NOISE_MORE:
        case CFG_NOISE_MOST:
            sCfgNoise = noise;
            break;
        case CFG_NOISE_UNKNOWN:
            break;
    }
}

static const char *sCfgModelToStr(const CFG_MODEL_t model)
{
    switch (model)
    {
        case CFG_MODEL_STANDARD: return PSTR(CFG_MODEL_STANDARD_STR);
        case CFG_MODEL_CHEWIE:   return PSTR(CFG_MODEL_CHEWIE_STR);
        case CFG_MODEL_HELLO:    return PSTR(CFG_MODEL_HELLO_STR);
        case CFG_MODEL_GITTA:    return PSTR(CFG_MODEL_GITTA_STR);
        case CFG_MODEL_UNKNOWN:
        default:                 return PSTR(CFG_MODEL_UNKNOWN_STR);
    }
}

static const char *sCfgDriverToStr(const CFG_DRIVER_t driver)
{
    switch (driver)
    {
        case CFG_DRIVER_WS2801:  return PSTR(CFG_DRIVER_WS2801_STR);
      //case CFG_DRIVER_WS2812:  return PSTR(CFG_DRIVER_WS2812_STR);
        case CFG_DRIVER_SK9822:  return PSTR(CFG_DRIVER_SK9822_STR);
        case CFG_DRIVER_UNKNOWN:
        default:                 return PSTR(CFG_DRIVER_UNKNOWN_STR);
    }
}

static const char *sCfgOrderToStr(const CFG_ORDER_t order)
{
    switch (order)
    {
        case CFG_ORDER_RGB:      return PSTR(CFG_ORDER_RGB_STR);
        case CFG_ORDER_RBG:      return PSTR(CFG_ORDER_RBG_STR);
        case CFG_ORDER_GRB:      return PSTR(CFG_ORDER_GRB_STR);
        case CFG_ORDER_GBR:      return PSTR(CFG_ORDER_GBR_STR);
        case CFG_ORDER_BRG:      return PSTR(CFG_ORDER_BRG_STR);
        case CFG_ORDER_BGR:      return PSTR(CFG_ORDER_BGR_STR);
        case CFG_ORDER_UNKNOWN:
        default:                 return PSTR(CFG_ORDER_UNKNOWN_STR);
    }
}

static const char *sCfgBrightToStr(const CFG_BRIGHT_t bright)
{
    switch (bright)
    {
        case CFG_BRIGHT_LOW:     return PSTR(CFG_BRIGHT_LOW_STR);
        case CFG_BRIGHT_MEDIUM:  return PSTR(CFG_BRIGHT_MEDIUM_STR);
        case CFG_BRIGHT_HIGH:    return PSTR(CFG_BRIGHT_HIGH_STR);
        case CFG_BRIGHT_FULL:    return PSTR(CFG_BRIGHT_FULL_STR);
        case CFG_BRIGHT_UNKNOWN:
        default:                 return PSTR(CFG_BRIGHT_UNKNOWN_STR);
    }
}

static const char *sCfgNoiseToStr(const CFG_NOISE_t noise)
{
    switch (noise)
    {
        case CFG_NOISE_NONE:     return PSTR(CFG_NOISE_NONE_STR);
        case CFG_NOISE_SOME:     return PSTR(CFG_NOISE_SOME_STR);
        case CFG_NOISE_MORE:     return PSTR(CFG_NOISE_MORE_STR);
        case CFG_NOISE_MOST:     return PSTR(CFG_NOISE_MOST_STR);
        case CFG_NOISE_UNKNOWN:
        default:                 return PSTR(CFG_NOISE_UNKNOWN_STR);
    }
}

static CFG_MODEL_t sCfgStrToModel(const char *str)
{
    if      (strcmp(CFG_MODEL_STANDARD_STR, str) == 0) { return CFG_MODEL_STANDARD; }
    else if (strcmp(CFG_MODEL_CHEWIE_STR,   str) == 0) { return CFG_MODEL_CHEWIE; }
    else if (strcmp(CFG_MODEL_HELLO_STR,    str) == 0) { return CFG_MODEL_HELLO; }
    else if (strcmp(CFG_MODEL_GITTA_STR,    str) == 0) { return CFG_MODEL_GITTA; }
    else                                               { return CFG_MODEL_UNKNOWN; }
}

static CFG_DRIVER_t sCfgStrToDriver(const char *str)
{
    if      (strcmp(CFG_DRIVER_WS2801_STR, str) == 0) { return CFG_DRIVER_WS2801; }
  //else if (strcmp(CFG_DRIVER_WS2812_STR, str) == 0) { return CFG_DRIVER_WS2812; }
    else if (strcmp(CFG_DRIVER_SK9822_STR, str) == 0) { return CFG_DRIVER_SK9822; }
    else                                              { return CFG_DRIVER_UNKNOWN; }
}

static CFG_ORDER_t sCfgStrToOrder(const char *str)
{
    if      (strcmp(CFG_ORDER_RGB_STR, str) == 0) { return CFG_ORDER_RGB; }
    else if (strcmp(CFG_ORDER_RBG_STR, str) == 0) { return CFG_ORDER_RBG; }
    else if (strcmp(CFG_ORDER_GRB_STR, str) == 0) { return CFG_ORDER_GRB; }
    else if (strcmp(CFG_ORDER_GBR_STR, str) == 0) { return CFG_ORDER_GBR; }
    else if (strcmp(CFG_ORDER_BRG_STR, str) == 0) { return CFG_ORDER_BRG; }
    else if (strcmp(CFG_ORDER_BGR_STR, str) == 0) { return CFG_ORDER_BGR; }
    else                                          { return CFG_ORDER_UNKNOWN; }
}

static CFG_BRIGHT_t sCfgStrToBright(const char *str)
{
    if      (strcmp(CFG_BRIGHT_LOW_STR,    str) == 0) { return CFG_BRIGHT_LOW; }
    else if (strcmp(CFG_BRIGHT_MEDIUM_STR, str) == 0) { return CFG_BRIGHT_MEDIUM; }
    else if (strcmp(CFG_BRIGHT_HIGH_STR,   str) == 0) { return CFG_BRIGHT_HIGH; }
    else if (strcmp(CFG_BRIGHT_FULL_STR,   str) == 0) { return CFG_BRIGHT_FULL; }
    else                                              { return CFG_BRIGHT_UNKNOWN; }
}

static CFG_NOISE_t sCfgStrToNoise(const char *str)
{
    if      (strcmp(CFG_NOISE_NONE_STR, str) == 0) { return CFG_NOISE_NONE; }
    else if (strcmp(CFG_NOISE_SOME_STR, str) == 0) { return CFG_NOISE_SOME; }
    else if (strcmp(CFG_NOISE_MORE_STR, str) == 0) { return CFG_NOISE_MORE; }
    else if (strcmp(CFG_NOISE_MOST_STR, str) == 0) { return CFG_NOISE_MOST; }
    else                                           { return CFG_NOISE_UNKNOWN; }
}

bool cfgApply(const char *json)
{
    DEBUG("cfg: json=%s", json);
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error)
    {
        ERROR("cfg: bad json: %s", error.c_str());
        return false;
    }
    const char *strModel  = doc[F("model")];
    const char *strDriver = doc[F("driver")];
    const char *strOrder  = doc[F("order")];
    const char *strBright = doc[F("bright")];
    const char *strNoise  = doc[F("noise")];
    CFG_MODEL_t  cfgModel  = strModel  != NULL ? sCfgStrToModel(strModel)   : CFG_MODEL_UNKNOWN;
    CFG_DRIVER_t cfgDriver = strDriver != NULL ? sCfgStrToDriver(strDriver) : CFG_DRIVER_UNKNOWN;
    CFG_ORDER_t  cfgOrder  = strOrder  != NULL ? sCfgStrToOrder(strOrder)   : CFG_ORDER_UNKNOWN;
    CFG_BRIGHT_t cfgBright = strBright != NULL ? sCfgStrToBright(strBright) : CFG_BRIGHT_UNKNOWN;
    CFG_NOISE_t  cfgNoise  = strNoise  != NULL ? sCfgStrToNoise(strNoise)   : CFG_NOISE_UNKNOWN;
    DEBUG("cfg: json: model=%s (%s), driver=%s (%s), order=%s (%s), bright=%s (%s), noise=%s (%s)",
        strModel  != NULL ? strModel  : PSTR("(n/a)"), sCfgModelToStr(cfgModel),
        strDriver != NULL ? strDriver : PSTR("(n/a)"), sCfgDriverToStr(cfgDriver),
        strOrder  != NULL ? strOrder  : PSTR("(n/a)"), sCfgOrderToStr(cfgOrder),
        strBright != NULL ? strBright : PSTR("(n/a)"), sCfgBrightToStr(cfgBright),
        strNoise  != NULL ? strNoise  : PSTR("(n/a)"), sCfgNoiseToStr(cfgNoise));
    if ( (cfgModel  != CFG_MODEL_UNKNOWN)  &&
         (cfgDriver != CFG_DRIVER_UNKNOWN) &&
         (cfgOrder  != CFG_ORDER_UNKNOWN)  &&
         (cfgBright != CFG_BRIGHT_UNKNOWN) &&
         (cfgNoise  != CFG_NOISE_UNKNOWN) )
    {
        sCfgModel  = cfgModel;
        sCfgDriver = cfgDriver;
        sCfgOrder  = cfgOrder;
        sCfgBright = cfgBright;
        sCfgNoise  = cfgNoise;
        PRINT("cfg: okay");
        return true;
    }
    else
    {
        ERROR("cfg: bad");
        return false;
    }
}

static void sCfgMonStatus(void)
{
    DEBUG("mon: cfg: model=%s, driver=%s, order=%s, bright=%s, noise=%s",
        sCfgModelToStr(sCfgModel), sCfgDriverToStr(sCfgDriver),
        sCfgOrderToStr(sCfgOrder), sCfgBrightToStr(sCfgBright),
        sCfgNoiseToStr(sCfgNoise));
}

void cfgInit(void)
{
    DEBUG("cfg: init");
    sCfgModel  = CFG_MODEL_UNKNOWN;
    sCfgDriver = CFG_DRIVER_UNKNOWN;
    sCfgOrder  = CFG_ORDER_UNKNOWN;
    sCfgBright = CFG_BRIGHT_UNKNOWN;
    sCfgNoise  = CFG_NOISE_SOME;
    debugRegisterMon(sCfgMonStatus);
}

// eof
