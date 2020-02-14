/*!
    \file
    \brief flipflip's Tschenggins Lämpli: debugging output (see \ref FF_DEBUG)

    - Copyright (c) 2018-2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/tschenggins-laempli
*/

#include <Ticker.h>

#include "stuff.h"
#include "debug.h"

static Ticker sMonTicker;

static DEBUG_MON_FUNC_t sMonFuncs[10];
static int sMonFuncsNum;

void debugRegisterMon(DEBUG_MON_FUNC_t monFunc)
{
    if (sMonFuncsNum < NUMOF(sMonFuncs))
    {
        sMonFuncs[sMonFuncsNum++] = monFunc;
    }
}

static void sDebugMon(void)
{
    DEBUG("mon: -----------------------------------------------------------------------------------------------");
#if defined(ESP8266)
    uint32_t heapFree;
    uint16_t heapMax;
    uint8_t heapFrag;
    ESP.getHeapStats(&heapFree, &heapMax, &heapFrag);
    DEBUG("mon: debug: heap=%u/%u/%u%%", heapFree, heapMax, heapFrag);
#elif defined(ESP32)
    DEBUG("mon: debug: heap=%u/%u/%u",
        ESP.getMinFreeHeap(), ESP.getMaxAllocHeap(), ESP.getFreeHeap());
#endif
    for (int ix = 0; ix < sMonFuncsNum; ix++)
    {
        sMonFuncs[ix]();
    }
    DEBUG("mon: -----------------------------------------------------------------------------------------------");
}

void debugInit(void)
{
    Serial.begin(115200);
#ifdef ESP8266
    Serial.setDebugOutput(true);
#endif
    Serial.println();
    Serial.println();
    Serial.println();
    DEBUG("debug: init");
    
    sMonTicker.attach_ms(5000, sDebugMon);
}

void HEXDUMP(const void *pkData, int size)
{
    const char hexdigits[] = "0123456789abcdef";
    const char *data = (const char *)pkData;
    for (int ix = 0; ix < size; )
    {
        char buf[128];
        memset(buf, ' ', sizeof(buf));
        for (int ix2 = 0; ix2 < 16; ix2++)
        {
            const uint8_t c = data[ix + ix2];
            buf[3 * ix2    ] = hexdigits[ (c >> 4) & 0xf ];
            buf[3 * ix2 + 1] = hexdigits[  c       & 0xf ];
            buf[3 * 16 + 2 + ix2] = isprint((int)c) ? c : '.';
            buf[3 * 16 + 3 + ix2] = '\0';
        }
        DEBUG("0x%08x  %s", (uint32_t)data + ix, buf);
        ix += 16;
    }
}

