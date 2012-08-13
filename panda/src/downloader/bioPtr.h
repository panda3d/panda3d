// Filename: bioPtr.h
// Created by:  drose (15Oct02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef BIOPTR_H
#define BIOPTR_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL
#define OPENSSL_NO_KRB5

#include "referenceCount.h"
#include "openSSLWrapper.h"  // must be included before any other openssl.
#include "openssl/ssl.h"

class URLSpec;

////////////////////////////////////////////////////////////////////
//       Class : BioPtr
// Description : A wrapper around an OpenSSL BIO object to make a
//               reference-counting pointer to it.  It appears that
//               the OpenSSL library already uses reference counts on
//               these things internally, but the interface doesn't
//               appear to be public; so we might as well wrap the
//               whole thing at the high level.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS BioPtr : public ReferenceCount {
public:
  INLINE BioPtr(BIO *bio);
  BioPtr(const URLSpec &url);
  virtual ~BioPtr();

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
};

#include "bioPtr.I"

#endif  // HAVE_OPENSSL


#endif


