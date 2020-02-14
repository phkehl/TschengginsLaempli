/*!
    \file
    \brief flipflip's Tschenggins Lämpli: Jenkins (see \ref FF_JENKINS)

    - Copyright (c) 2018-2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/tschenggins-laempli
*/

#include <pgmspace.h>
#include <sys/string.h>

#include "stuff.h"
#include "debug.h"
#include "leds.h"
#include "cfg.h"
#include "status.h"

#include "jenkins.h"

/* ***** external interface ********************************************************************* */

JENKINS_STATE_t jenkinsStrToState(const char *str)
{
    JENKINS_STATE_t state = JENKINS_STATE_UNKNOWN;
    if (strcmp_P(str, PSTR(JENKINS_STATE_IDLE_STR)) == 0)
    {
        state = JENKINS_STATE_IDLE;
    }
    else if (strcmp_P(str, PSTR(JENKINS_STATE_RUNNING_STR)) == 0)
    {
        state = JENKINS_STATE_RUNNING;
    }
    else if (strcmp_P(str, PSTR(JENKINS_STATE_OFF_STR)) == 0)
    {
        state = JENKINS_STATE_OFF;
    }
    return state;
}

JENKINS_RESULT_t jenkinsStrToResult(const char *str)
{
    JENKINS_RESULT_t result = JENKINS_RESULT_UNKNOWN;
    if (strcmp_P(str, PSTR(JENKINS_RESULT_FAILURE_STR)) == 0)
    {
        result = JENKINS_RESULT_FAILURE;
    }
    else if (strcmp_P(str, PSTR(JENKINS_RESULT_UNSTABLE_STR)) == 0)
    {
        result = JENKINS_RESULT_UNSTABLE;
    }
    else if (strcmp_P(str, PSTR(JENKINS_RESULT_SUCCESS_STR)) == 0)
    {
        result = JENKINS_RESULT_SUCCESS;
    }
    return result;
}

const char *sJenkinsStateToStr(const JENKINS_STATE_t state)
{
    switch (state)
    {
        case JENKINS_STATE_UNKNOWN: return PSTR(JENKINS_STATE_UNKNOWN_STR);
        case JENKINS_STATE_OFF:     return PSTR(JENKINS_STATE_OFF_STR);
        case JENKINS_STATE_RUNNING: return PSTR(JENKINS_STATE_RUNNING_STR);
        case JENKINS_STATE_IDLE:    return PSTR(JENKINS_STATE_IDLE_STR);
        default:                    return PSTR("???");
    }
}

const char *sJenkinsResultToStr(const JENKINS_RESULT_t result)
{
    switch (result)
    {
        case JENKINS_RESULT_UNKNOWN:  return PSTR(JENKINS_RESULT_UNKNOWN_STR);
        case JENKINS_RESULT_SUCCESS:  return PSTR(JENKINS_RESULT_SUCCESS_STR);
        case JENKINS_RESULT_UNSTABLE: return PSTR(JENKINS_RESULT_UNSTABLE_STR);
        case JENKINS_RESULT_FAILURE:  return PSTR(JENKINS_RESULT_FAILURE_STR);
        default:                      return PSTR("???");
    }
}

typedef enum JENKINS_MSG_TYPE_e
{
    JENKINS_MSG_TYPE_INFO,
    JENKINS_MSG_TYPE_CLEAR_ALL,
    JENKINS_MSG_TYPE_UNKNOWN_ALL,
} JENKINS_MSG_TYPE_t;

typedef struct JENKINS_MSG_s
{
    JENKINS_MSG_TYPE_t type;
    union
    {
        JENKINS_INFO_t info;
    };
} JENKINS_MSG_t;

// forward declaration
void sJenkinsUpdate(void);


/* ***** internal things ************************************************************************ */

// convert Jenkins state and result to LED colour and mode
static const LEDS_PARAM_t *sJenkinsLedStateFromJenkins(const JENKINS_STATE_t state, const JENKINS_RESULT_t result)
{
    static const LEDS_PARAM_t skLedStateOff = { 0 };
    const LEDS_PARAM_t *pRes = &skLedStateOff;

    switch (state)
    {
        case JENKINS_STATE_RUNNING:
        {
            switch (result)
            {
                case JENKINS_RESULT_SUCCESS:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(85, 255, 255, PULSE, 0);
                    pRes = &skLedState;
                    break;
                }
                case JENKINS_RESULT_UNSTABLE:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(38, 255, 255, PULSE, 0);
                    pRes = &skLedState;
                    break;
                }
                case JENKINS_RESULT_FAILURE:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(0, 255, 255, PULSE, 0);
                    pRes = &skLedState;
                    break;
                }
                case JENKINS_RESULT_UNKNOWN:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(0, 0, 255, PULSE, 0);
                    pRes = &skLedState;
                    break;
                }
            }
            break;
        }
        case JENKINS_STATE_IDLE:
        {
            switch (result)
            {
                case JENKINS_RESULT_SUCCESS:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(85, 255, 255, STILL, 0);
                    pRes = &skLedState;
                    break;
                }
                case JENKINS_RESULT_UNSTABLE:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(38, 255, 255, STILL, 0);
                    pRes = &skLedState;
                    break;
                }
                case JENKINS_RESULT_FAILURE:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(0, 255, 255, STILL, 0);
                    pRes = &skLedState;
                    break;
                }
                case JENKINS_RESULT_UNKNOWN:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(0, 0, 200, STILL, 0);
                    pRes = &skLedState;
                    break;
                }
            }
            break;
        }
        case JENKINS_STATE_UNKNOWN:
        {
            switch (result)
            {
                case JENKINS_RESULT_SUCCESS:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(85, 180, 128, FLICKER, 0);
                    pRes = &skLedState;
                    break;
                }
                case JENKINS_RESULT_UNSTABLE:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(38, 220, 128, FLICKER, 0);
                    pRes = &skLedState;
                    break;
                }
                case JENKINS_RESULT_FAILURE:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(0, 180, 128, FLICKER, 0);
                    pRes = &skLedState;
                    break;
                }
                case JENKINS_RESULT_UNKNOWN:
                {
                    static const LEDS_PARAM_t skLedState = LEDS_MAKE_PARAM(0, 0, 128, FLICKER, 0);
                    pRes = &skLedState;
                    break;
                }
            }
        }
        default:
            break;
    }

    return pRes;
}

// current info for all channels
static JENKINS_INFO_t sJenkinsInfo[CONFIG_NUM_CH];

// current dirty flag for all channels
static bool sJenkinsInfoDirty[NUMOF(sJenkinsInfo)];

// curently worst result
static JENKINS_RESULT_t sJenkinsWorstResult;

// currently most active state
static JENKINS_STATE_t sJenkinsActiveState;

// store info
void jenkinsSetInfo(const JENKINS_INFO_t *pkInfo, bool update)
{
    // store new info
    JENKINS_INFO_t *pInfo = NULL;
    if (pkInfo->chIx < NUMOF(sJenkinsInfo))
    {
        pInfo = &sJenkinsInfo[pkInfo->chIx];
        memcpy(pInfo, pkInfo, sizeof(*pInfo));
        if (!pInfo->active)
        {
            const int ix = pInfo->chIx;
            memset(pInfo, 0, sizeof(*pInfo));
            pInfo->chIx = ix;
        }
        sJenkinsInfoDirty[pkInfo->chIx] = true;
    }

    // inform
    if (pInfo != NULL)
    {
        if (pInfo->active)
        {
            const char *state  = sJenkinsStateToStr(pInfo->state);
            const char *result = sJenkinsResultToStr(pInfo->result);
            const uint32_t now = getTime();
            const uint32_t age = now - pInfo->time;
            PRINT("jenkins: info: #%02d %-" STRINGIFY(JENKINS_JOBNAME_LEN) "s %-" STRINGIFY(JENKINS_SERVER_LEN) "s %-7s %-8s %6.1fh",
                pInfo->chIx, pInfo->job, pInfo->server, state, result, (double)age / 3600.0);
        }
        else
        {
            PRINT("jenkins: info: #%02d <unused>", pInfo->chIx);
        }
    }
    if (update)
    {
        jenkinsUpdate();
    }
}

// clear all info
void jenkinsClearAll(void)
{
    PRINT("jenkins: clear all");
    memset(&sJenkinsInfo, 0, sizeof(sJenkinsInfo));
    for (int ix = 0; ix < NUMOF(sJenkinsInfo); ix++)
    {
        sJenkinsInfoDirty[ix] = true;
    }
    jenkinsUpdate();
}

// set all channels to state unknown
void jenkinsStateUnknownAll(void)
{
    PRINT("jenkins: state unknown all");
    for (int ix = 0; ix < NUMOF(sJenkinsInfo); ix++)
    {
        if (sJenkinsInfo[ix].active)
        {
            sJenkinsInfo[ix].state = JENKINS_STATE_UNKNOWN;
            sJenkinsInfoDirty[ix] = true;
        }
    }
    jenkinsUpdate();
}

// update all dirty channels, re-calculate worst result, play sounds
void jenkinsUpdate(void)
{
    DEBUG("jenkins: update");

    // update LEDs (for Lämplis with individual LEDs)
    if (cfgGetModel() != CFG_MODEL_HELLO)
    {
        for (int ix = 0; ix < NUMOF(sJenkinsInfo); ix++)
        {
            if (sJenkinsInfoDirty[ix])
            {
                sJenkinsInfoDirty[ix] = false;
                const JENKINS_INFO_t *pkInfo = &sJenkinsInfo[ix];
                if (pkInfo->active)
                {
                    ledsSetState(ix, sJenkinsLedStateFromJenkins(pkInfo->state, pkInfo->result));
                }
                else
                {
                    ledsSetState(ix, sJenkinsLedStateFromJenkins(JENKINS_STATE_UNKNOWN, JENKINS_RESULT_UNKNOWN));
                }
            }
        }
    }

    // find worst result, state
    JENKINS_RESULT_t worstResult = JENKINS_RESULT_UNKNOWN;
    JENKINS_STATE_t activeState = JENKINS_STATE_UNKNOWN;
    for (int ix = 0; ix < NUMOF(sJenkinsInfo); ix++)
    {
        const JENKINS_INFO_t *pkInfo = &sJenkinsInfo[ix];
        if (pkInfo->result >= worstResult)
        {
            worstResult = pkInfo->result;
        }
        if (pkInfo->state > activeState)
        {
            activeState = pkInfo->state;
        }
    }
    DEBUG("jenkins: worst is now %s (was %s), most active is now %s (was %s)",
        sJenkinsResultToStr(worstResult), sJenkinsResultToStr(sJenkinsWorstResult),
        sJenkinsStateToStr(activeState), sJenkinsStateToStr(sJenkinsActiveState));

    // update LEDs (for the "Hello Jenkins" model)
    if (cfgGetModel() == CFG_MODEL_HELLO)
    {
        const LEDS_PARAM_t *pkState = NULL;
        switch (activeState)
        {
            // R=0, RG=43, G=85, BG=128, B=170,
            case JENKINS_STATE_UNKNOWN:
            {
                static const LEDS_PARAM_t skState = LEDS_MAKE_PARAM(43, 255, 255, BLINK,  10);
                pkState = &skState;
                break;
            }
            case JENKINS_STATE_OFF:
            {
                static const LEDS_PARAM_t skState = LEDS_MAKE_PARAM(43, 255, 255, BLINK, 100);
                pkState = &skState;
                break;
            }
            case JENKINS_STATE_IDLE:
            {
                static const LEDS_PARAM_t skState = LEDS_MAKE_PARAM(43, 255, 255, FLICKER, 0);
                pkState = &skState;
                break;
            }
            case JENKINS_STATE_RUNNING:
            {
                static const LEDS_PARAM_t skState = LEDS_MAKE_PARAM(43, 255, 255, BLINK,  30);
                pkState = &skState;
                break;
            }
        }
        ledsSetStateHello(sJenkinsLedStateFromJenkins(activeState, worstResult), pkState);
    }

    // play sound if we changed from failure/warning to success or from success/warning to failure
    // TODO: play more sounds if CONFIG_NOISE_MORE
    // FIXME: also check for state == idle?
    if (sJenkinsWorstResult != JENKINS_RESULT_UNKNOWN)
    {
        if (worstResult != sJenkinsWorstResult)
        {
            switch (worstResult)
            {
                case JENKINS_RESULT_FAILURE:
                    PRINT("jenkins: failure!");
                    if (cfgGetNoise() >= CFG_NOISE_MORE)
                    {
                        statusToneStop();
                        switch (cfgGetModel())
                        {
                            // "chewie" and "hello" models have a separate sound fx,
                            // so trigger that and play the melody only if wanted
                            case CFG_MODEL_CHEWIE:
                                statusFx();
                                if (cfgGetNoise() >= CFG_NOISE_MOST)
                                {
                                    statusMelody(PSTR("ImperialShort"));
                                }
                                break;
                            case CFG_MODEL_HELLO:
                                statusFx();
                                if (cfgGetNoise() >= CFG_NOISE_MOST)
                                {
                                    statusMelody(PSTR("ImperialShort"));
                                }
                                break;
                            // other models play only the melody
                            default:
                                statusMelody(PSTR("ImperialShort"));
                                break;
                        }

                    }
                    break;
                case JENKINS_RESULT_SUCCESS:
                    PRINT("jenkins: success!");
                    if (cfgGetNoise() >= CFG_NOISE_MORE)
                    {
                        statusToneStop();
                        statusMelody(PSTR("IndianaShort"));
                    }
                    break;
                case JENKINS_RESULT_UNSTABLE:
                    PRINT("jenkins: unstable!");
                    break;
                default:
                    break;
            }
        }
    }
    sJenkinsWorstResult = worstResult;
    sJenkinsActiveState = activeState;
}

/* ***** init and monitoring stuff ************************************************************** */

static void sJenkinsMonStatus(void)
{
    static char str[ (CONFIG_NUM_CH * 6) + 2 ];
    char *pStr = str;
    int len = sizeof(str) - 1;
    bool last = false;
    int ix = 0;
    while (!last)
    {
        const JENKINS_INFO_t *pkInfo = &sJenkinsInfo[ix];
        const char *stateStr  = sJenkinsStateToStr(pkInfo->state);
        const char *resultStr = sJenkinsResultToStr(pkInfo->result);
        const char stateChar  = pkInfo->state  == JENKINS_STATE_UNKNOWN  ? '?' : (char)pgm_read_byte(&stateStr[0]);
        const char resultChar = pkInfo->result == JENKINS_RESULT_UNKNOWN ? '?' : (char)pgm_read_byte(&resultStr[0]);
        const int n = snprintf(pStr, len, " %02i%c%c%c", ix, pkInfo->active ? '=' : '-',
            toupper((int)stateChar), pkInfo->state > JENKINS_STATE_OFF ? toupper((int)resultChar) : resultChar);
        len -= n;
        pStr += n;
        ix++;
        last = (ix >= NUMOF(sJenkinsInfo)) || (len < 1);
        if ( ((ix % 10) == 0) || last )
        {
            DEBUG("mon: jenkins:%s", str);
            pStr = str;
            len = sizeof(str) - 1;
        }
    }
    DEBUG("mon: jenkins: worst=%s active=%s",
        sJenkinsResultToStr(sJenkinsWorstResult), sJenkinsStateToStr(sJenkinsActiveState));
}

#define JENKINS_MSG_QUEUE_LEN 5

void jenkinsInit(void)
{
    DEBUG("jenkins: init");

    debugRegisterMon(sJenkinsMonStatus);
    jenkinsClearAll();
}

// eof
