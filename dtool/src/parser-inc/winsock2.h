#ifndef _WINSOCK2API_
#define _WINSOCK2API_
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */

#include <wtypes.h>

typedef unsigned long SOCKET;

struct sockaddr;
struct sockaddr_in;
struct sockaddr_in6;
struct sockaddr_storage;

#endif
