// Filename: ssl_utils.h
// Created by:  drose (15Dec03)
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

#ifndef SSL_UTILS_H
#define SSL_UTILS_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL
#define OPENSSL_NO_KRB5

#include "openssl/ssl.h"

EXPCL_PANDAEXPRESS void notify_ssl_errors();

#endif  // HAVE_OPENSSL

#endif


