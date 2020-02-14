/*!
    \file
    \brief GITTA Tschenggins Lämpli: backend data handling (see \ref FF_BACKEND)

    - Copyright (c) 2018-2020 Philippe Kehl & flipflip industries (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/tschenggins-laempli
*/

#include <ArduinoJson.h>
#include <pgmspace.h>

#include "stuff.h"
#include "debug.h"
#include "config.h"
#include "cfg.h"
#include "status.h"
#include "jenkins.h"

#include "backend.h"

#define BACKEND_HEARTBEAT_INTERVAL 5000
#define BACKEND_HEARTBEAT_TIMEOUT (3 * BACKEND_HEARTBEAT_INTERVAL)

static uint32_t sLastHello;
static uint32_t sLastHeartbeat;
static uint32_t sBytesReceived;
static int      sBackendBufMax;
static uint32_t sBackendBufFull;
static BACKEND_STATUS_t sBackendStatus;

void backendDisconnect(const bool keepStatus)
{
    DEBUG("backend: disconnect");
    const uint32_t now = millis();
    if (keepStatus)
    {
        jenkinsStateUnknownAll();
    }
    else
    {
        jenkinsClearAll();
    }
    sLastHeartbeat = 0;
    sLastHello = 0;
    sBytesReceived = 0;
    sBackendBufMax = 0;
    sBackendBufFull = 0;
    sBackendStatus = BACKEND_STATUS_NONE;
}

static const char *sBackendStatusStr(const BACKEND_STATUS_t status)
{
    switch (status)
    {
        case BACKEND_STATUS_NONE:       return PSTR("NONE");
        case BACKEND_STATUS_CONNECTED:  return PSTR("CONNECTED");
        case BACKEND_STATUS_OKAY:       return PSTR("OKAY");
        case BACKEND_STATUS_RECONNECT:  return PSTR("RECONNECT");
        case BACKEND_STATUS_FAIL:       return PSTR("FAIL");
        case BACKEND_STATUS_RXBUF:      return PSTR("RXBUF");
        default:                        return PSTR("???");
    }
}

static void sBackendMonStatus(void)
{
    const uint32_t now = millis();
    DEBUG("mon: backend: status=%s, uptime=%u (%s), heartbeat=%u, bytes=%u, bufMax=%d, bufFull=%u",
        sBackendStatusStr(sBackendStatus),
        sLastHello ? now - sLastHello : 0,
        sLastHello ? ((now - sLastHello) > (1000 * CONFIG_STABLE_CONN_THRS) ? PSTR("stable") : PSTR("unstable") ) : PSTR("n/a"),
        sLastHeartbeat ? now - sLastHeartbeat : 0, sBytesReceived, sBackendBufMax, sBackendBufFull);
}


// forward declarations
void sBackendProcessStatus(const char *json);

static void sBackendHandleSetTime(const char *timestampStr)
{
    const uint32_t ts = (uint32_t)atoi(timestampStr);
    if (ts != 0)
    {
        setTime(ts);
    }
}

// public domain, from https://stackoverflow.com/questions/1634359/is-there-a-reverse-function-for-strstr, modified
static char *strrstr(char *haystack, const char *needle)
{
    if (*needle == '\0')
    {
        return haystack;
    }

    char *result = NULL;
    while (true)
    {
        char *p = strstr(haystack, needle);
        if (p == NULL)
        {
            break;
        }
        result = p;
        haystack = p + 1;
    }

    return result;
}

// process response from backend
BACKEND_STATUS_t backendHandle(char *resp, const int len)
{
    BACKEND_STATUS_t res = BACKEND_STATUS_OKAY;
    sBytesReceived += len;

    const uint32_t now = millis();

    //DEBUG("backendHandle() [%d] %s", len, resp);

    // backend buffer (we may or may not get full lines in resp from the socket
    static char sBackendBuf[4096];
    const int backendBufLen = strlen(sBackendBuf);
    const int backendBufLenNew = backendBufLen + len;
    if (sizeof(sBackendBuf) > backendBufLenNew)
    {
        strncat(sBackendBuf, resp, sizeof(sBackendBuf) - 2);
        if (backendBufLenNew > sBackendBufMax)
        {
            sBackendBufMax = backendBufLenNew;
        }
    }
    else
    {
        WARNING("backend: rx buf");
        sBackendBuf[0] = '\0';
        sBackendBufFull++;
        sBackendStatus = BACKEND_STATUS_RXBUF;
        return BACKEND_STATUS_RXBUF;
    }

    // look for known server data
    char *pHello     = strstr_P(sBackendBuf, PSTR("\r\n""hello "));
    char *pConfig    = strstr_P(sBackendBuf, PSTR("\r\n""config "));
    char *pStatus    = strstr_P(sBackendBuf, PSTR("\r\n""status "));
    char *pHeartbeat = strstr_P(sBackendBuf, PSTR("\r\n""heartbeat "));
    char *pError     = strstr_P(sBackendBuf, PSTR("\r\n""error "));
    char *pReconnect = strstr_P(sBackendBuf, PSTR("\r\n""reconnect "));
    char *pCommand   = strstr_P(sBackendBuf, PSTR("\r\n""command "));

    // end of lines, may or may not be followed by an incomplete next line (see below)
    const char *lastCrLf = strrstr(sBackendBuf, "\r\n");

    // \r\nhello 87e984 256 clientname\r\n
    if (pHello != NULL)
    {
        pHello += 2;
        char *endOfLine = strstr_P(pHello, PSTR("\r\n"));
        if (endOfLine != NULL)
        {
            *endOfLine = '\0';
            const char *pMsg = &pHello[6];
            DEBUG("backend: hello: %s", pMsg);
            if (sLastHello == 0)
            {
                sLastHello = now;
                res = BACKEND_STATUS_CONNECTED;
                PRINT("backend: connected");
            }
        }
    }

    // "\r\nerror 1491146601 WTF?\r\n"
    if (pError != NULL)
    {
        pError += 2;
        char *endOfLine = strstr_P(pError, PSTR("\r\n"));
        if (endOfLine != NULL)
        {
            *endOfLine = '\0';
            sBackendHandleSetTime(&pError[6]);
            const char *pMsg = &pError[6 + 10 + 1];
            ERROR("backend: error: %s", pMsg);
        }
    }

    // "\r\nreconnect 1491146601\r\n"
    if (pReconnect != NULL)
    {
        pReconnect += 2;
        char *endOfLine = strstr_P(pReconnect, PSTR("\r\n"));
        if (endOfLine != NULL)
        {
            *endOfLine = '\0';
            sBackendHandleSetTime(&pReconnect[10]);
            PRINT("backend: reconnect");
            res = BACKEND_STATUS_RECONNECT;
        }
    }

    // "\r\nheartbeat 1491146601 25\r\n"
    if (pHeartbeat != NULL)
    {
        pHeartbeat += 2;
        char *endOfLine = strstr_P(pHeartbeat, PSTR("\r\n"));
        if (endOfLine != NULL)
        {
            *endOfLine = '\0';
            sBackendHandleSetTime(&pHeartbeat[10]);
            sLastHeartbeat = now;
            const char *pCnt = &pHeartbeat[10 + 10 + 1];
            DEBUG("backend: heartbeat %s", pCnt);
        }
    }

    // "\r\nconfig 1491146576 json={"key":"value", ... }\r\n"
    if (pConfig != NULL)
    {
        pConfig += 2;
        char *endOfLine = strstr_P(pConfig, PSTR("\r\n"));
        if (endOfLine != NULL)
        {
            *endOfLine = '\0';
            sBackendHandleSetTime(&pConfig[7]);
            char *pJson = &pConfig[7 + 10 + 1];
            DEBUG("backend: config");
            statusNoise(STATUS_NOISE_OTHER);
            if (!cfgApply(pJson))
            {
                statusNoise(STATUS_NOISE_ERROR);
            }
        }
    }

    // "\r\nstatus 1491146576 json={"jobs": ... }\r\n"
    if (pStatus != NULL)
    {
        pStatus += 2;
        char *endOfLine = strstr_P(pStatus, PSTR("\r\n"));
        if (endOfLine != NULL)
        {
            *endOfLine = '\0';
            sBackendHandleSetTime(&pStatus[7]);
            DEBUG("backend: status");
            char *pJson = &pStatus[7 + 10 + 1];
            sBackendProcessStatus(pJson);
        }
    }

    // "\r\ncommand 1491146601 reset\r\n"
    if (pCommand != NULL)
    {
        pCommand += 2;
        char *endOfLine = strstr_P(pCommand, PSTR("\r\n"));
        if (endOfLine != NULL)
        {
            *endOfLine = '\0';
            sBackendHandleSetTime(&pCommand[8]);
            const char *pCmd = &pCommand[8 + 10 + 1];
            if (strcmp_P(pCmd, PSTR("reconnect")) == 0)
            {
                PRINT("backend: command reconnect");
                statusNoise(STATUS_NOISE_OTHER);
                res = BACKEND_STATUS_RECONNECT;
            }
            else if (strcmp_P(pCmd, PSTR("reset")) == 0)
            {
                PRINT("backend: command restart");
                FLUSH();
                statusNoise(STATUS_NOISE_BOMB);
                while (statusTonePlaying()) { delay(10); }
                ESP.restart();
            }
            else if (strcmp_P(pCmd, PSTR("identify")) == 0)
            {
                PRINT("backend: command identify");
                statusToneStop();
                // ignore noise config
                CFG_NOISE_t noise = cfgGetNoise();
                cfgSetNoise(CFG_NOISE_MORE);
                statusMelody(PSTR("PacMan"));
                cfgSetNoise(noise);
            }
            else if (strcmp_P(pCmd, PSTR("indy")) == 0)
            {
                PRINT("backend: command indy");
                statusToneStop();
                // ignore noise config
                CFG_NOISE_t noise = cfgGetNoise();
                cfgSetNoise(CFG_NOISE_MORE);
                statusMelody(PSTR("IndianaShort"));
                cfgSetNoise(noise);
            }
            else if (strcmp_P(pCmd, PSTR("random")) == 0)
            {
                PRINT("backend: command random");
                statusToneStop();
                statusMelody(PSTR("random"));
            }
            else if ( (strcmp_P(pCmd, PSTR("chewie")) == 0) || (strcmp_P(pCmd, PSTR("hello")) == 0) )
            {
                PRINT("backend: command chewie/hello");
                statusFx();
            }
            else
            {
                WARNING("backend: command %s ???", pCmd);
                statusToneStop();
                statusNoise(STATUS_NOISE_ERROR);
            }
        }
    }

    // we must always receive the "hello" in the first chunk of data
    if (sLastHello == 0)
    {
        ERROR("backend: no hello");
        res = BACKEND_STATUS_FAIL;
    }

    // check heartbeat
    if (res == BACKEND_STATUS_OKAY)
    {
        if ( (now - sLastHeartbeat) > BACKEND_HEARTBEAT_TIMEOUT )
        {
            ERROR("backend: lost heartbeat");
            res = BACKEND_STATUS_FAIL;
        }
    }

    // keep remaining string in buffer
    if (lastCrLf != NULL)
    {
        strcpy(sBackendBuf, lastCrLf);
    }

    if (sBackendStatus != res)
    {
        DEBUG("backend: status %s -> %s", sBackendStatusStr(sBackendStatus), sBackendStatusStr(res));
    }

    sBackendStatus = res;
    return res;
}

// JSON size
// [[99,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa","aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa","aaaaaaaaaa","aaaaaaaaaa",1111111111],...]
// https://arduinojson.org/v6/assistant/ --> 20x --> 4320
//DynamicJsonDocument jsonDoc(4500);
StaticJsonDocument<4500> jsonDoc;

void sBackendProcessStatus(const char *json)
{
    DEBUG("backend: [%d] %s", strlen(json), json);
    
    DeserializationError error = deserializeJson(jsonDoc, json);
    if (error)
    {
        ERROR("backend: bad json: %s", error.c_str());
        statusNoise(STATUS_NOISE_ERROR);
        return;
    }

    JsonVariant variant = jsonDoc.as<JsonVariant>();
    if (!variant.is<JsonArray>())
    {
        ERROR("backend: json not array");
        statusNoise(STATUS_NOISE_ERROR);
        return;
    }

    int numUpdate = 0;;
    JsonArray arr = variant.as<JsonArray>();
    for (int ix = 0; ix < arr.size(); ix++)
    {
        bool good = true;
        if (arr[ix].is<JsonArray>())
        {
            // full info
            if ( (arr[ix].size() == 6) && (arr[ix][0] >= 0) && (arr[ix][0] < CONFIG_NUM_CH) )
            {
                JENKINS_INFO_t info;
                info.active = true;
                info.chIx = arr[ix][0];
                snprintf(info.job, sizeof(info.job), "%s", arr[ix][1].as<char *>());
                snprintf(info.server, sizeof(info.server), "%s", arr[ix][2].as<char *>());
                info.state  = jenkinsStrToState(arr[ix][3]);
                info.result = jenkinsStrToResult(arr[ix][4]);
                info.time = arr[ix][5];
                jenkinsSetInfo(&info, false);
                numUpdate++;
            }
            // only ch number, clear data
            else if ( (arr[ix].size() == 1) && (arr[ix][0] >= 0) && (arr[ix][0] < CONFIG_NUM_CH) )
            {
                const int   ch     = arr[ix][0];
                JENKINS_INFO_t info;
                info.active = false;
                info.chIx = ch;
                jenkinsSetInfo(&info, false);
                numUpdate++;
            }
            else
            {
                good = false;
            }
        }
        else
        {
            good = false;
        }

        if (!good)
        {
            WARNING("backend: bad json[%d]", ix);
        }
    }

    // are we happy?
    if (numUpdate > 0)
    {
        statusNoise(STATUS_NOISE_OTHER);
        jenkinsUpdate();
    }
    else
    {
        statusNoise(STATUS_NOISE_ERROR);
    }
}

void backendInit(void)
{
    DEBUG("backend: init");
    debugRegisterMon(sBackendMonStatus);
}

/* ********************************************************************************************** */

// eof
