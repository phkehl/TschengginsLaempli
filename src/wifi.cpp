/*!
    \file
    \brief flipflip's Tschenggins Lämpli: wifi and network things (see \ref FF_WIFI)

    - Copyright (c) 2018-2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/tschenggins-laempli
*/

#if defined(ESP8266)
#  include <ESP8266WiFi.h>
#  include <ESP8266WiFiMulti.h>
#  include <ESP8266HTTPClient.h>
#  include <WiFiClient.h>
#  include <WiFiClientSecure.h>
#elif defined(ESP32)
#  include <WiFi.h>
#  include <WiFiMulti.h>
#  include <HTTPClient.h>
#  include <WiFiClient.h>
#  include <WiFiClientSecure.h>
#endif

#include "stuff.h"
#include "config.h"
#include "secrets.h"

#include "debug.h"
#include "backend.h"
#include "status.h"

#include "wifi.h"

#if defined(ESP8266)
static ESP8266WiFiMulti wifiMulti;
#elif defined(ESP32)
static WiFiMulti wifiMulti;
#endif
static char sClientName[8];
static char sUserAgent[100];

static const char *sWifiWlStatusStr(const wl_status_t status)
{
    switch (status)
    {
        case WL_NO_SHIELD:       return PSTR("NO_SHIELD");
        case WL_IDLE_STATUS:     return PSTR("IDLE_STATUS");
        case WL_NO_SSID_AVAIL:   return PSTR("NO_SSID_AVAIL");
        case WL_SCAN_COMPLETED:  return PSTR("CAN_COMPLETED");
        case WL_CONNECTED:       return PSTR("CONNECTED");
        case WL_CONNECT_FAILED:  return PSTR("CONNECT_FAILED");
        case WL_CONNECTION_LOST: return PSTR("CONNECTION_LOST");
        case WL_DISCONNECTED:    return PSTR("DISCONNECTED");
        default:                 return PSTR("???");
    }
}

void sWifiMon(void)
{
    DEBUG("mon: wifi: status=%s, ssid=%s, rssi=%d, client=%s",
        sWifiWlStatusStr(WiFi.status()), WiFi.SSID().c_str(), WiFi.RSSI(), sClientName);
    DEBUG("mon: wifi: ip=%s, mask=%s, gw=%s, dns=%s",
        WiFi.localIP().toString().c_str(),
        WiFi.subnetMask().toString().c_str(),
        WiFi.gatewayIP().toString().c_str(),
        WiFi.dnsIP().toString().c_str());
}

void wifiInit(void)
{
    WiFi.mode(WIFI_STA);
    
    snprintf(sClientName, sizeof(sClientName), "%06x",
#if defined(ESP8266)
         (uint32_t)ESP.getChipId() & 0x00ffffff);
#elif defined (ESP32)
         (uint32_t)ESP.getEfuseMac() & 0x00ffffff);
#endif
    
    char staName[33];
    snprintf(staName, NUMOF(staName), PSTR("tschenggins-laempli-%s"), sClientName);

#if defined(ESP8266)
    WiFi.hostname(staName);
#elif defined (ESP32)
    WiFi.setHostname(staName);
#endif

    DEBUG("wifi: init (staName=%s, staMac=%s, ssid=" SECRET_WIFI_SSID ", ssid2="
#ifdef SECRET_WIFI2_SSID
        SECRET_WIFI2_SSID
#else
        "n/a"
#endif
        ", ssid3="
#ifdef SECRET_WIFI3_SSID
        SECRET_WIFI3_SSID
#else
        "n/a"
#endif
        ")", staName, WiFi.macAddress().c_str());
    debugRegisterMon(sWifiMon);
    
    //WiFi.setAutoConnect(true);
    //WiFi.setAutoReconnect(true);
    //WiFi.begin();

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    wifiMulti.addAP(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
#if defined(SECRET_WIFI2_SSID) && defined(SECRET_WIFI2_PASS)
    wifiMulti.addAP(SECRET_WIFI2_SSID, SECRET_WIFI2_PASS);
#endif
#if defined(SECRET_WIFI3_SSID) && defined(SECRET_WIFI3_PASS)
    wifiMulti.addAP(SECRET_WIFI3_SSID, SECRET_WIFI3_PASS);
#endif

    snprintf(sUserAgent, NUMOF(sUserAgent), PSTR("tschenggins-laempli/" CONFIG_SOFTWARE_VERSION
        " (" CONFIG_VERSION_GIT_HASH "; " CONFIG_PLATFORM_NAME "; %s; " CONFIG_VERSION_YYYYMMDD 
        "; " CONFIG_VERSION_HHMMSS ")"), sClientName);
    DEBUG("wifi: http user agent=%s", sUserAgent);
}

static wl_status_t sWifiStatus = (wl_status_t)254; 

bool wifiWaitForConnect(void)
{
#if defined(ESP8266)
    const wl_status_t status = wifiMulti.run();
#elif defined(ESP32)
    const wl_status_t status = (wl_status_t)wifiMulti.run();
#endif
    if (status != sWifiStatus)
    {
        DEBUG("wifi: status %s -> %s", sWifiWlStatusStr(sWifiStatus), sWifiWlStatusStr(status));
        sWifiStatus = status;
        switch (status)
        {
            case WL_CONNECTED:
                //statusNoise(STATUS_NOISE_ONLINE);
                statusLed(STATUS_LED_UPDATE);
                break;
            case WL_DISCONNECTED:
            case WL_NO_SSID_AVAIL:
            case WL_CONNECT_FAILED:
            case WL_CONNECTION_LOST:
                statusNoise(STATUS_NOISE_ABORT);
                statusLed(STATUS_LED_OFFLINE);
                break;
            default:
                statusLed(STATUS_LED_OFFLINE);
                break;
        }
    }
    return status == WL_CONNECTED;
}

// query parameters for the backend
#define BACKEND_QUERY "cmd=realtime;ascii=1;client=%s;name=%s;stassid=%s;staip=%s;version=" CONFIG_VERSION_GIT_HASH";maxch="STRINGIFY(CONFIG_NUM_CH)
#define BACKEND_ARGS(client, name, stassid, staip) client, name, stassid, staip

bool wifiConnectBackend(void)
{
    PRINT("wifi: connecting to backend");
    HTTPClient       http;
    WiFiClientSecure client;
    //WiFiClient client; // TODO: allow http://

    http.setUserAgent(sUserAgent);
    http.setTimeout(10000); // [ms]
    //http.setFollowRedirects(true); // FIXME: does it even work for POST requests?
    //http.setRedirectLimit(5);
#if defined(ESP8266)
    client.setInsecure();
    client.setBufferSizes(4096, 2048); // FIXME: good? seems to work fine...
    client.setTimeout(10000); // [ms]
#endif

    const char backendUrl[] = SECRET_BACKEND_URL;
    DEBUG("wifi: backend=%s", backendUrl);

    if (!http.begin(client, backendUrl))
    {
        ERROR("wifi: fail connect");
        return false;
    }
    
    static char param[200];
    snprintf_P(param, NUMOF(param), PSTR(BACKEND_QUERY), // FIXME: handle too long query string
#if defined(ESP8266)
        BACKEND_ARGS(sClientName, WiFi.hostname().c_str(), WiFi.SSID().c_str(), WiFi.localIP().toString().c_str()));
#elif defined (ESP32)
        BACKEND_ARGS(sClientName, WiFi.getHostname(),      WiFi.SSID().c_str(), WiFi.localIP().toString().c_str()));

#endif
    DEBUG("wifi: param[%d]=%s", strlen(param), param);
    http.addHeader(PSTR("Content-Type"), PSTR("application/x-www-form-urlencoded"));
    const int respStatus = http.POST((uint8_t *)param, strlen(param));
    const int respSize = http.getSize();
    
    if ( (respStatus < 0) || (respStatus != HTTP_CODE_OK) )
    {
        ERROR("wifi: POST fail (status=%d, size=%d) %s", respStatus, respSize,
            respStatus < 0 ? http.errorToString(respStatus).c_str() : PSTR("unexpected response status"));
        http.end();
        return false;
    }

    DEBUG("wifi: POST okay (status=%d, size=%d)", respStatus, respSize);

    // get data
    bool abort = false;
    bool res = true;
    uint32_t connectedSince = millis();
    while ( !abort && http.connected() && ( (respSize == -1) || (respSize > 0) ) )
    {
        const int sizeAvail = client.available();
        static uint8_t data[101];
        if (sizeAvail > 0)
        {
            const int sizeRead = sizeAvail > (sizeof(data) - 1) ? (sizeof(data) - 1) : sizeAvail;
            const int dataSize = client.readBytes(data, sizeRead);

            // process data
            data[dataSize] = '\0'; // nul terminate it
            //DEBUG("wifi: resp [%d] %s", sizeRead, buf);
            const BACKEND_STATUS_t status = backendHandle((char *)data, dataSize);

            switch (status)
            {
                // connection successfully started, handshake complete
                case BACKEND_STATUS_CONNECTED:
                    statusNoise(STATUS_NOISE_ONLINE);
                    statusLed(STATUS_LED_HEARTBEAT);
                    connectedSince = millis();
                    break;
                    
                // connection ongoing..
                case BACKEND_STATUS_OKAY:
                case BACKEND_STATUS_RXBUF:
                    break;
                
                // connection failed (no handshake, heartbeat lost)
                case BACKEND_STATUS_FAIL:
                    statusNoise(STATUS_NOISE_OTHER);
                    abort = true;
                    res = false;
                    break;

                // forced reset (by the user/backend)
                case BACKEND_STATUS_RECONNECT:
                    statusNoise(STATUS_NOISE_FAIL);
                    abort = true;
                    break;
            }
        }
        else
        {
            delay(11);
        }
    }
    
    // server disconnected us (or network connection lost?)
    if (!http.connected())
    {
        ERROR("wifi: connection lost");
        statusNoise(STATUS_NOISE_FAIL);
        res = false;
    }
    client.stop();
    http.end();

    // clear jenkins state (and status if we had only a short connection)
    backendDisconnect( res || ((millis() - connectedSince) > (1000 * CONFIG_STABLE_CONN_THRS)) );

    sWifiStatus = (wl_status_t)254;

    PRINT("wifi: disconnected from backend");

    return res;
}

// eof
