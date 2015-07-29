#ifndef __SOCKET_PORTABLE_H__
#define __SOCKET_PORTABLE_H__ 
//////////////////////////////////////////////////////////////////
// Lots of stuff to make network socket-based io transparent across multiple
//  platforms
//////////////////////////////////////////////////////////////////

const int ALL_OK = 0;
const int BASIC_ERROR = -1;

/************************************************************************
* HP SOCKET LIBRARY STUFF
************************************************************************/
#if defined(HP_SOCK)

#ifndef _INCLUDE_HPUX_SOURCE
#define _INCLUDE_HPUX_SOURCE
#define _INCLUDE_POSIX_SOURCE
#define _INCLUDE_XOPEN_SOURCE
#endif
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define socket_read read
#define socket_write write
#define socket_close close

#define DO_CONNECT(a,b,c)   connect(a,b,c)
#define DO_SOCKET_READ(a,b,c,d)     socket_read(a,b,c)
#define DO_SOCKET_WRITE(a,b,c,d)    socket_write(a,b,c)

#define GETERROR()                  errno


#define  SOCKIOCTL       ioctl
typedef unsigned long SOCKET;
#define BAD_SOCKET 0xffffffff


/************************************************************************
* WINSOCK 32 bit STUFF
************************************************************************/
#elif defined(WIN32) || defined(WIN32_VC) || defined(WIN64_VC)
#include <winsock2.h>
#include <Ws2tcpip.h>


inline int DO_SELECT(SOCKET n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,struct timeval *timeout)
{
    return select((int) n, readfds, writefds, exceptfds,timeout);
}

inline int DO_CONNECT( const SOCKET a, const struct sockaddr_in *b)
{
    return connect(a, reinterpret_cast<const struct ::sockaddr *>(b), sizeof(sockaddr));
}
inline int DO_SOCKET_READ(const SOCKET a, char * buf, const int size)
{
    return recv(a, buf, size, 0);
}
inline int DO_SOCKET_WRITE(const SOCKET a, const char * buff, const int len)
{
    return send(a, buff, len, 0);
}
inline int DO_SOCKET_WRITE_TO(const SOCKET a, const char * buffer, const int buf_len, const sockaddr_in * addr)
{
    return sendto(a, buffer, buf_len, 0, reinterpret_cast<const struct ::sockaddr *>(addr), sizeof(sockaddr));
}
inline SOCKET DO_NEWUDP()
{
    return socket(AF_INET, SOCK_DGRAM, 0);
}
inline SOCKET DO_NEWTCP()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}
inline int DO_BIND(const SOCKET a, const sockaddr_in *b)
{
    return ::bind(a, reinterpret_cast<const struct ::sockaddr *>(b), sizeof(sockaddr));
}
inline int DO_CLOSE(const SOCKET a)
{
    return closesocket(a);
}
inline SOCKET DO_ACCEPT(SOCKET sck, sockaddr_in * adr)
{
    int adrlen = sizeof(sockaddr);
    return accept(sck, reinterpret_cast<sockaddr *>(adr), &adrlen);
};
inline int DO_RECV_FROM(SOCKET sck, char * data, int len, sockaddr_in * addr)
{
    int plen = sizeof(sockaddr);
    return recvfrom(sck, data, len, 0, reinterpret_cast<sockaddr *>(addr), &plen);
}
inline int DO_LISTEN(const SOCKET a, const int size)
{
    return listen(a, size);
}

inline int GETERROR()
{
    return WSAGetLastError();
}

inline int SOCKIOCTL(const SOCKET s, const long flags, unsigned long * val)
{
    return ioctlsocket(s, flags, val);
}

inline int init_network()
{
    static struct WSAData mydata;
    int answer = WSAStartup(0x0101, &mydata);
    if (answer != 0)
        return BASIC_ERROR;
    
    return ALL_OK;
}

inline bool do_shutdown_send(SOCKET s)
{
    return (shutdown(s,SD_SEND) == 0);
};

typedef  int socklen_t  ;
const long LOCAL_NONBLOCK = 1;
const long LOCAL_FL_SET = FIONBIO ;
const int LOCAL_BLOCKING_ERROR = WSAEWOULDBLOCK;
const int LOCAL_CONNECT_BLOCKING = WSAEWOULDBLOCK;
const int LOCAL_NOTCONNECTED_ERROR = WSAENOTCONN;
const int LOCAL_TIMEOUT_ERROR = WSAETIMEDOUT;
const SOCKET BAD_SOCKET = (SOCKET)-1;

/************************************************************************
* Solaris 2.6 and Irix 6.4 STUFF
************************************************************************/
#elif defined(SunOS) || defined(SUNNEW) || defined(IRIX64)

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/filio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef int SOCKET;
const SOCKET BAD_SOCKET = 0xffffffff;

//#define DO_CONNECT(a,b)               connect(a,(sockaddr *)b,sizeof(sockaddr))
//#define DO_SOCKET_READ(a,b,c)         recv(a,b,c,0)
//#define DO_SOCKET_WRITE(a,b,c)        send(a,b,c,0)

inline int DO_CONNECT(const SOCKET a, const sockaddr_in *b)
{
    return connect(a, reinterpret_cast<const struct ::sockaddr *>(b), sizeof(sockaddr));
}
inline int DO_SOCKET_READ(const SOCKET a, char * buf, const int size)
{
    return recv(a, buf, size, 0);
}
inline int DO_SOCKET_WRITE(const SOCKET a, const char * buff, const int len)
{
    return send(a, buff, len, 0);
}

//#define DO_SOCKET_WRITE_TO(a,b,c,d)   sendto(a,b,c,0,(sockaddr *)d,sizeof(sockaddr))
//#define DO_NEWUDP()          socket(AF_INET, SOCK_DGRAM, 0)
//#define DO_NEWTCP()       socket(AF_INET, SOCK_STREAM, 0)
//#define DO_BIND(a,b)      ::bind(a,(sockaddr *)b,sizeof(sockaddr))
//#/define DO_CLOSE(a)       close(a)
inline int DO_SOCKET_WRITE_TO(const SOCKET a, const char * buffer, const int buf_len, const sockaddr_in * addr)
{
    return sendto(a, buffer, buf_len, 0, reinterpret_cast<const struct ::sockaddr *>(addr), sizeof(sockaddr));
}
inline SOCKET DO_NEWUDP()
{
    return socket(AF_INET, SOCK_DGRAM, 0);
}
inline SOCKET DO_NEWTCP()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}
inline int DO_BIND(const SOCKET a, const sockaddr_in *b)
{
    return ::bind(a, reinterpret_cast<const struct ::sockaddr *>(b), sizeof(sockaddr));
}
inline int DO_CLOSE(const SOCKET a)
{
    return close(a);
}
inline int DO_ACCEPT(SOCKET sck, sockaddr_in * adr)
{
    int adrlen = sizeof(sockaddr);
    return accept(sck, ( sockaddr *)adr, &adrlen);
};

inline int DO_RECV_FROM(SOCKET sck, char * data, int len, sockaddr_in * addr)
{
    int plen = sizeof(sockaddr);
    return recvfrom(sck, data, len, 0, (sockaddr *)addr, &plen);
}
inline int DO_LISTEN(const SOCKET a, const int size)
{
    return listen(a, size);
}

inline int GETERROR()
{
    return errno;
}

inline int SOCKIOCTL(const SOCKET s, const long flags, void * val)
{
    return ioctl(s, flags, val);
}

inline int init_network()
{
    return ALL_OK;
}
#ifndef INADDR_NONE
const INADDR_NONE = -1;
#endif

const long LOCAL_NONBLOCK = 1;
const long LOCAL_FL_SET = FIONBIO ;
const int LOCAL_BLOCKING_ERROR = EAGAIN;
const int LOCAL_CONNECT_BLOCKING = EINPROGRESS;

/************************************************************************
* LINUX and FreeBSD STUFF
************************************************************************/

#elif defined(IS_LINUX) || defined(IS_OSX) || defined(IS_FREEBSD)

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
//#include <netinet/in_systm.h>
#include <netinet/tcp.h>
//#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct sockaddr_in AddressType; 

typedef int SOCKET;
const SOCKET BAD_SOCKET = -1;
inline int DO_SELECT(SOCKET n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,struct timeval *timeout)
{
    return select((int) n, readfds, writefds, exceptfds,timeout);
}

inline int DO_CONNECT(const SOCKET a, const sockaddr_in *b)
{
    return connect(a, reinterpret_cast<const struct ::sockaddr *>(b), sizeof(sockaddr));
}
inline int DO_SOCKET_READ(const SOCKET a, char * buf, const int size)
{
    return recv(a, buf, size, 0);
}
inline int DO_SOCKET_WRITE(const SOCKET a, const char * buff, const int len)
{
    return send(a, buff, len, 0);
}
///////////////////////////////////////////////
inline int DO_SOCKET_WRITE_TO(const SOCKET a, const char * buffer, const int buf_len, const sockaddr_in * addr)
{
    return sendto(a, buffer, buf_len, 0, reinterpret_cast<const struct ::sockaddr *>(addr), sizeof(sockaddr));
}
inline SOCKET DO_NEWUDP()
{
    return socket(AF_INET, SOCK_DGRAM, 0);
}
inline SOCKET DO_NEWTCP()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}
inline int DO_BIND(const SOCKET a, const sockaddr_in *b)
{
    return ::bind(a, reinterpret_cast<const struct ::sockaddr *>(b), sizeof(sockaddr));
}
inline int DO_CLOSE(const SOCKET a)
{
    return close(a);
}

inline int DO_ACCEPT(SOCKET sck, sockaddr_in * adr)
{
    socklen_t adrlen = sizeof(sockaddr);
    return accept(sck, ( sockaddr *)adr, &adrlen);
};

inline int DO_RECV_FROM(SOCKET sck, char * data, int len, sockaddr_in * addr)
{
    socklen_t plen = sizeof(sockaddr);
    return recvfrom(sck, data, len, 0, (sockaddr *)addr, &plen);
}


inline int init_network()
{
    signal(SIGPIPE, SIG_IGN); // hmm do i still need this ...
    return ALL_OK;
}

inline int DO_LISTEN(const SOCKET a, const int size)
{
    return listen(a, size);
}

inline int GETERROR()
{
    return errno;
}

inline int SOCKIOCTL(const SOCKET s, const long flags, void * val)
{
    return ioctl(s, flags, val);
}

inline bool do_shutdown_send(SOCKET s)
{
    return (shutdown(s,SHUT_WR) == 0);
};


#define  BSDBLOCK


const long LOCAL_NONBLOCK = 1;
// With BSDBLOCK defined, we don't need FIONBIO.  Solaris doesn't provide it.
//const long LOCAL_FL_SET = FIONBIO ;
const int LOCAL_BLOCKING_ERROR = EAGAIN;
const int LOCAL_CONNECT_BLOCKING = EINPROGRESS;

#else 
/************************************************************************
* NO DEFINITION => GIVE COMPILATION ERROR
************************************************************************/
No Host Type defined !!
#error  Fatal
#endif



#endif //__SOCKET_PORTABLE_H__
