// Filename: ssl_utils.h
// Created by:  drose (15Dec03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef SSL_UTILS_H
#define SSL_UTILS_H

#include "pandabase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL
#define OPENSSL_NO_KRB5

#include <openssl/ssl.h>

EXPCL_PANDAEXPRESS void notify_ssl_errors();

#endif  // HAVE_OPENSSL

#endif


