#ifndef _WINSOCK2API_
#define _WINSOCK2API_
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */

typedef int SOCKET ;

struct sockaddr_in
{
};


typedef struct fd_set {
        unsigned int fd_count;               /* how many are SET? */
        SOCKET  fd_array[10];   /* an array of SOCKETs */
} fd_set;

#endif
