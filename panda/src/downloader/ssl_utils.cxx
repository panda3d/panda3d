// Filename: ssl_utils.cxx
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

#include "ssl_utils.h"
#include "config_downloader.h"

#ifdef HAVE_OPENSSL

#ifdef REPORT_OPENSSL_ERRORS
#include <openssl/err.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: notify_ssl_errors
//  Description: A convenience function that is itself a wrapper
//               around the OpenSSL convenience function to output the
//               recent OpenSSL errors.  This function sends the error
//               string to downloader_cat.warning().  If
//               REPORT_OPENSSL_ERRORS is not defined, the function
//               does nothing.
////////////////////////////////////////////////////////////////////
void notify_ssl_errors() {
#ifdef REPORT_OPENSSL_ERRORS
  static bool strings_loaded = false;
  if (!strings_loaded) {
    SSL_load_error_strings();
    strings_loaded = true;
  }

  unsigned long e = ERR_get_error();
  while (e != 0) {
    static const size_t buffer_len = 256;
    char buffer[buffer_len];
    ERR_error_string_n(e, buffer, buffer_len);
    downloader_cat.warning() << buffer << "\n";
    e = ERR_get_error();
  }
#endif  //  REPORT_OPENSSL_ERRORS
}

#endif  // HAVE_OPENSSL
