/*!
    \file
    \brief flipflip's Tschenggins Lämpli: documentation (see \ref mainpage)

    - Copyright (c) 2017-2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/tschenggins-laempli
*/

#define DO_TESTS 0

/* ********************************************************************************************** */

#include <Arduino.h>
#if defined(ESP8266)
#  include <ESP8266WiFi.h>
#elif defined(ESP32)
#  include <WiFi.h>
#else
#  error Unsupported target!
#endif

#if DO_TESTS
#  include <Ticker.h>
#  include <SPI.h>
#endif

#include "src/stuff.h"
#include "src/config.h"

#include "src/debug.h"
#include "src/status.h"
#include "src/leds.h"
#include "src/cfg.h"
#include "src/wifi.h"
#include "src/backend.h"
#include "src/jenkins.h"

/* ********************************************************************************************** */

void setup()
{
    // initialise stuff
    debugInit();
    statusInit();
    cfgInit();
    ledsInit();
    jenkinsInit();
    backendInit();
    randomSeed(ESP.getCycleCount());

    // say hello
    NOTICE("--------------------------------------------------------------------------------------------");
    NOTICE("Tschenggins Lämpli " CONFIG_SOFTWARE_VERSION
        " (" CONFIG_VERSION_GIT_HASH ", " CONFIG_PLATFORM_NAME ", " CONFIG_VERSION_YYYYMMDD " " CONFIG_VERSION_HHMMSS ")");
    NOTICE("Copyright (c) 2018-2020 Philippe Kehl & flipflip industries <flipflip at oinkzwurgl dot org>");
    NOTICE("Parts copyright by others. See source code.");
    NOTICE("https://oinkzwurgl.org/projaeggd/tschenggins-laempli");
    NOTICE("--------------------------------------------------------------------------------------------");
#if defined(ESP8266)
    DEBUG("ESP8266: chip=0x%08x, bootVer=%u, bootMode=%u, cpu=%uMHz", 
        ESP.getChipId(), ESP.getBootVersion(), ESP.getBootMode(), ESP.getCpuFreqMHz());
    DEBUG("ESP8266: %s", ESP.getFullVersion().c_str());
    DEBUG("ESP8266: flash: id=0x%08x, size=%u (%uKiB), real=%u (%uKiB), speed=%ukHz",
        ESP.getFlashChipId(), ESP.getFlashChipSize(), ESP.getFlashChipSize() >> 10, 
        ESP.getFlashChipRealSize(), ESP.getFlashChipRealSize() >> 10,
        ESP.getFlashChipSpeed() / 1000);
    DEBUG("ESP8266: heap: free=%u, maxBlock=%u, frag=%u%%",
        ESP.getFreeHeap(), ESP.getMaxFreeBlockSize(), ESP.getHeapFragmentation());
    switch ((enum rst_reason)ESP.getResetInfoPtr()->reason)
    {
        case REASON_WDT_RST:
        case REASON_EXCEPTION_RST:
        case REASON_SOFT_WDT_RST:
            ERROR("ESP8266: reset: %s (%s)", ESP.getResetReason().c_str(), ESP.getResetInfo().c_str());
            break;
        case REASON_DEFAULT_RST:
            WARNING("ESP8266: reset: %s (%s)", ESP.getResetReason().c_str(), ESP.getResetInfo().c_str());
            break;
        default:
            DEBUG("ESP8266: reset: %s (%s)", ESP.getResetReason().c_str(), ESP.getResetInfo().c_str());
            break;
    }
    //ESP.resetFreeContStack(); uint32_t maxStack = ESP.getFreeContStack();
#elif defined(ESP32)
    DEBUG("ESP32: rev=0x%02x, cpu=%uMHz", ESP.getChipRevision(), ESP.getCpuFreqMHz());
    DEBUG("ESP32: %s", ESP.getSdkVersion());
    DEBUG("ESP32: flash: size=%u (%uKiB)", ESP.getFlashChipSize(), ESP.getFlashChipSize() >> 10);
    DEBUG("ESP32: heap: "/*size=%u, "*/"free=%u, maxBlock=%u, minFree=%u",
        /*EPS.getHeapSize(), */ESP.getFreeHeap(), ESP.getMaxAllocHeap(), ESP.getMinFreeHeap());
#endif
    DEBUG("GCC: " __VERSION__); // /*", FreeRTOS " tskKERNEL_VERSION_NUMBER */ 
    //DEBUG("Arduino: sketch=%u (%uKiB), free=%u (%uKiB)",
    //    ESP.getSketchSize(), ESP.getSketchSize() >> 10,
    //    ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace() >> 10);

    // initialise more things
    wifiInit();

    NOTICE("Here we go...");
    statusLed(STATUS_LED_OFFLINE);
    statusNoise(STATUS_NOISE_FFI);
}

void loop()
{
#if DO_TESTS
    NOTICE("\n\n\n\nDO_TESTS start\n");
#  if 1
    NOTICE("\n\n\n\nTEST config\n");
    const char jsonConfigBad[] = "{\"driver\":\"WS2801\",\"model\":\"badword\",\"noise\":\"some\",\"order\":\"RGB\"}";
    cfgApply(jsonConfigBad);
    const char jsonConfigGood[] = "{\"driver\":\"WS2801\",\"model\":\"standard\",\"noise\":\"most\",\"order\":\"RGB\",\"bright\":\"medium\"}";
    cfgApply(jsonConfigGood);
    delay(1500);
#  endif
#  if 1
    NOTICE("\n\n\n\nTEST leds\n");
    for (uint8_t o = 0; o < 4; o++)
    {
        for (uint8_t i = 0; i < CONFIG_NUM_CH; i++)
        {
            switch (rand() % 4)
            {
               case 0: {
                    LEDS_PARAM_t ledState1 = LEDS_MAKE_PARAM((o * 30) + (i * 45), 255, 255, STILL, 0);
                    ledsSetState(i, &ledState1);
                    break; }
                case 1: {
                    LEDS_PARAM_t ledState2 = LEDS_MAKE_PARAM((o * 30) + (i * 45), 255, 255, PULSE, 0);
                    ledsSetState(i, &ledState2);
                    break; }
                case 2: {
                    LEDS_PARAM_t ledState3 = LEDS_MAKE_PARAM((o * 30) + (i * 45), 255, 255, FLICKER, 0);
                    ledsSetState(i, &ledState3);
                    break; }
                case 3: {
                    LEDS_PARAM_t ledState4 = LEDS_MAKE_PARAM((o * 30) + (i * 45), 255, 255, BLINK, LEDS_FPS / 3);
                    ledsSetState(i, &ledState4);
                    break; }
            }
        }
        delay(3500);
    }
    for (uint8_t i = 0; i < CONFIG_NUM_CH; i++)
    {
        LEDS_PARAM_t ledState = LEDS_MAKE_PARAM(0, 0, 0, STILL, 0);
        ledsSetState(i, &ledState);
    }
#  endif
#  if 1
    NOTICE("\n\n\n\nTEST status noise\n");
    statusNoise(STATUS_NOISE_ABORT);  while (statusTonePlaying()) { delay(100); }; delay(1000);
    statusNoise(STATUS_NOISE_FAIL);   while (statusTonePlaying()) { delay(100); }; delay(1000);
    statusNoise(STATUS_NOISE_ONLINE); while (statusTonePlaying()) { delay(100); }; delay(1000);
    statusNoise(STATUS_NOISE_OTHER);  while (statusTonePlaying()) { delay(100); }; delay(1000);
    statusNoise(STATUS_NOISE_TICK);   while (statusTonePlaying()) { delay(100); }; delay(1000);
    statusNoise(STATUS_NOISE_ERROR);  while (statusTonePlaying()) { delay(100); }; delay(1000);
    statusNoise(STATUS_NOISE_BOMB);   while (statusTonePlaying()) { delay(100); }; delay(1000);
    statusNoise(STATUS_NOISE_FFI);    while (statusTonePlaying()) { delay(100); }; delay(1000);
#  endif
#  if 1
    NOTICE("\n\n\n\nTEST melody\n");
    statusMelody(PSTR("random")); while (statusTonePlaying()) {delay(100); }; delay(1000);
#  endif
#  if 1
    NOTICE("\n\n\n\nTEST status led\n");
    statusLed(STATUS_LED_HEARTBEAT); delay(5000);
    statusLed(STATUS_LED_OFFLINE);   delay(5000);
    statusLed(STATUS_LED_FAIL);      delay(5000);
    statusLed(STATUS_LED_UPDATE);    delay(5000);
    statusLed(STATUS_LED_NONE);      delay(5000);
#  endif
    DEBUG("\n\n\n\nDO_TESTS done\n");
    delay(10000);
#endif // DO_TESTS

#if !DO_TESTS
    
    // wait for wifi connection
    PRINT("main: waiting for wifi connection");
    while (!wifiWaitForConnect())
    {
        delay(100);
    }
    
    // connect to backend
    PRINT("main: attempting connection to backend");
    const bool res = wifiConnectBackend();
    statusLed(STATUS_LED_FAIL);

    // planned reconnect
    if (res)
    {
        delay(1000);
        return;
    }

    // connection failed, determine how long we wait to attempt a reconnect
    static uint32_t lastFail;
    const uint32_t now = millis();
    int waitTime = (now - lastFail) > (1000 * CONFIG_STABLE_CONN_THRS) ?
        CONFIG_RECONNECT_DELAY : CONFIG_RECONNECT_DELAY_SLOW;
    lastFail = now;
    PRINT("main: failure... waiting %us", waitTime);
    
    // wait until timeout and wifi ready
    while (waitTime > 0)
    {
        while (!wifiWaitForConnect())
        {
            delay(100);
        }
        // announce countdown
        if ( (waitTime < 10) || ((waitTime % 10) == 0) )
        {
            DEBUG("main: wait... %d", waitTime);
        }
        if (waitTime <= 3)
        {
            statusNoise(STATUS_NOISE_TICK);
            while (statusTonePlaying()) { delay(1); } // or the delay(1000) below won't work (on ESP8266) FIXME: wtf?!
        }
        waitTime--;
        delay(1000);
    }
    
    // and keep going...

#endif // !DO_TESTS
}
