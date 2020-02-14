/*!
    \file
    \brief flipflip's Tschenggins Lämpli: wifi and network things  (see \ref WIFI)

    - Copyright (c) 2018-2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/tschenggins-laempli

    \defgroup FF_JSON JSON
    \ingroup FF

    @{
*/
#ifndef __WIFI_H__
#define __WIFI_H__

#include <Arduino.h>

//! initialise
void wifiInit(void);

bool wifiWaitForConnect(void);

bool wifiConnectBackend(void);

#endif // __WIFI_H__
//@}
// eof
