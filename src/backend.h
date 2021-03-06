/*!
    \file
    \brief GITTA Tschenggins Lämpli: backend data handling (see \ref FF_BACKEND)

    - Copyright (c) 2018-2020 Philippe Kehl & flipflip industries (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/tschenggins-laempli

    \defgroup FF_BACKEND BACKEND
    \ingroup FF

    @{
*/
#ifndef __BACKEND_H__
#define __BACKEND_H__

#include <Arduino.h>

//! initialise
void backendInit(void);

typedef enum BACKEND_STATUS_e
{
    BACKEND_STATUS_NONE,
    BACKEND_STATUS_CONNECTED,
    BACKEND_STATUS_OKAY,
    BACKEND_STATUS_RECONNECT,
    BACKEND_STATUS_FAIL,
    BACKEND_STATUS_RXBUF,

} BACKEND_STATUS_t;

BACKEND_STATUS_t backendHandle(char *resp, const int len);

void backendDisconnect(const bool keepStatus);

#endif // __BACKEND_H__
//@}
// eof
