/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socket_fdset.h
 */


#ifndef __SOCKET_FDSET_H__
#define __SOCKET_FDSET_H__

#include "pandabase.h"
#include "numeric_types.h"
#include "time_base.h"
#include "socket_ip.h"

class Socket_fdset {
PUBLISHED:
  inline Socket_fdset();
  inline void setForSocket(const Socket_IP &incon);
  inline bool IsSetFor(const Socket_IP &incon) const;
  inline int WaitForRead(bool zeroFds, PN_uint32 sleep_time = 0xffffffff);
  inline int WaitForWrite(bool zeroFds, PN_uint32 sleep_time = 0xffffffff);
  inline int WaitForError(bool zeroFds, PN_uint32 sleep_time = 0xffffffff);

  inline int WaitForRead(bool zeroFds, const Time_Span & timeout);
  inline void clear();

private:
  inline void setForSocketNative(const SOCKET inid);
  inline bool isSetForNative(const SOCKET inid) const;

  friend struct Socket_Selector;

  SOCKET _maxid;

#ifndef CPPPARSER
  mutable fd_set _the_set;
#endif
};

/**
 * The constructor
 */
inline Socket_fdset::Socket_fdset() {
    clear();
}

/**
 * This does the physical manipulation of the set getting read for the base call
 */
inline void Socket_fdset::setForSocketNative(SOCKET inid)
{
    assert( inid >= 0);
#ifndef WIN32
    assert(inid < FD_SETSIZE);
#endif

    FD_SET(inid, &_the_set);
    if (_maxid < inid)
        _maxid = inid;
}

/**
 * Answer the question: was the socket marked for reading?  there's a subtle
 * difference in the NSPR version: it will respond if the socket had an error
 */
inline bool Socket_fdset::isSetForNative(SOCKET inid) const
{
    assert( inid >= 0);
#ifndef WIN32
    assert(inid < FD_SETSIZE);
#endif

    return (FD_ISSET(inid, &_the_set) != 0);
}

/**
 * check to see if a socket object has been marked for reading
 */
inline bool Socket_fdset::IsSetFor(const Socket_IP & incon) const
{
    return isSetForNative(incon.GetSocket());
}

/**

 */
inline int Socket_fdset::WaitForRead(bool zeroFds, PN_uint32 sleep_time)
{
    int retVal = 0;
    if (sleep_time == 0xffffffff) {
        retVal = DO_SELECT(_maxid + 1, &_the_set, NULL, NULL, NULL);
    } else {
        timeval timeoutValue;
        timeoutValue.tv_sec = sleep_time / 1000;
        timeoutValue.tv_usec = (sleep_time % 1000) * 1000;

        retVal = DO_SELECT(_maxid + 1, &_the_set, NULL, NULL, &timeoutValue);
    }
    if (zeroFds)
        clear();

    return retVal;
}

/**

 */
inline int Socket_fdset::WaitForRead(bool zeroFds, const Time_Span & timeout)
{
    timeval localtv = timeout.GetTval();

    int retVal = DO_SELECT(_maxid + 1, &_the_set, NULL, NULL, &localtv);
    if (zeroFds)
        clear();

    return retVal;
}

/**
 * Marks the content as empty
 */
inline void Socket_fdset::clear()
{
    _maxid = 0;
    FD_ZERO(&_the_set);
}

/**

 */
inline void Socket_fdset::setForSocket(const Socket_IP &incon)
{
    setForSocketNative(incon.GetSocket());
}

/**
 * This is the function that will wait till one of the sockets is ready for
 * writing
 */
inline int Socket_fdset::WaitForWrite(bool zeroFds, PN_uint32 sleep_time)
{
    int retVal = 0;
    if (sleep_time == 0xffffffff)
    {
        retVal = DO_SELECT(_maxid + 1, NULL, &_the_set, NULL, NULL);
    }
    else
    {
        timeval timeoutValue;
        timeoutValue.tv_sec = sleep_time / 1000;
        timeoutValue.tv_usec = (sleep_time % 1000) * 1000;

        retVal = DO_SELECT(_maxid + 1, NULL, &_the_set, NULL, &timeoutValue);
    }
    if (zeroFds)
        clear();

    return retVal;
}

/**
 * This is the function that will wait till one of the sockets is in error state
 */
inline int Socket_fdset::WaitForError(bool zeroFds, PN_uint32 sleep_time)
{
    int retVal = 0;
    if (sleep_time == 0xffffffff)
    {
        retVal = DO_SELECT(_maxid + 1, NULL, NULL, &_the_set, NULL);
    }
    else
    {
        timeval timeoutValue;
        timeoutValue.tv_sec = sleep_time / 1000;
        timeoutValue.tv_usec = (sleep_time % 1000) * 1000;

        retVal = DO_SELECT(_maxid + 1, NULL, NULL, &_the_set, &timeoutValue);
    }
    if (zeroFds)
        clear();

    return retVal;
}


#endif //__SOCKET_FDSET_H__
