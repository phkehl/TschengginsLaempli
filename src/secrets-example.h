// copy this file to secrets.h and fill in you secrets...
#ifndef __SECRETS_H__
#define __SECRETS_H__

#define SECRET_WIFI_SSID    "ssid1"
#define SECRET_WIFI_PASS    "pass"

// optionally add up to two more networks
//#define SECRET_WIFI2_SSID   "ssid2"
//#define SECRET_WIFI2_PASS   "pass2"

//#define SECRET_WIFI3_SSID   "ssid3"
//#define SECRET_WIFI3_PASS   "pass3"

// Note: currently only https:// will work (see wifi.cpp)
#define SECRET_BACKEND_URL "https://user:pass@hostname.domain/path/to/backend/"

#endif // __SECRETS_H__
