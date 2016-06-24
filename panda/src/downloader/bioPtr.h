/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bioPtr.h
 * @author drose
 * @date 2002-10-15
 */

#ifndef BIOPTR_H
#define BIOPTR_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

#ifndef OPENSSL_NO_KRB5
#define OPENSSL_NO_KRB5
#endif

#include "referenceCount.h"
#include "openSSLWrapper.h"  // must be included before any other openssl.
#include "openssl/ssl.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

class URLSpec;

/**
 * A wrapper around an OpenSSL BIO object to make a reference-counting pointer
 * to it.  It appears that the OpenSSL library already uses reference counts
 * on these things internally, but the interface doesn't appear to be public;
 * so we might as well wrap the whole thing at the high level.
 */
class EXPCL_PANDAEXPRESS BioPtr : public ReferenceCount {
public:
  INLINE BioPtr(BIO *bio);
  BioPtr(const URLSpec &url);
  virtual ~BioPtr();

  void set_nbio(bool nbio);
  bool connect();

  INLINE bool should_retry() const;

  INLINE BIO &operator *() const;
  INLINE BIO *operator -> () const;
  INLINE operator BIO * () const;

  INLINE void set_bio(BIO *bio);
  INLINE BIO *get_bio() const;

  INLINE const string &get_server_name() const;
  INLINE int get_port() const;

private:
  BIO *_bio;
  string _server_name;
  int _port;
  struct sockaddr_storage _addr;
  socklen_t _addrlen;
  bool _connecting;
};

#include "bioPtr.I"

#endif  // HAVE_OPENSSL


#endif
