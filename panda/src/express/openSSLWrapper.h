/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file openSSLWrapper.h
 * @author drose
 * @date 2009-09-05
 */

#ifndef OPENSSLWRAPPER_H
#define OPENSSLWRAPPER_H

#include "pandabase.h"

#ifdef HAVE_OPENSSL
#include "filename.h"

#ifdef _WIN32
#include <winsock2.h>  // must be included prior to including OpenSSL.
#endif

#ifndef OPENSSL_NO_KRB5
#define OPENSSL_NO_KRB5
#endif

#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

// Windows may define this macro inappropriately.
#ifdef X509_NAME
#undef X509_NAME
#endif

/**
 * Provides an interface wrapper around the OpenSSL library, to ensure that
 * the library is properly initialized in the application, and to provide some
 * hooks into global OpenSSL context data.
 */
class EXPCL_PANDA_EXPRESS OpenSSLWrapper {
private:
  OpenSSLWrapper();
  ~OpenSSLWrapper();

PUBLISHED:
  void clear_certificates();
  int load_certificates(const Filename &filename);
  int load_certificates_from_pem_ram(const char *data, size_t data_size);
  int load_certificates_from_der_ram(const char *data, size_t data_size);

  INLINE int load_certificates_from_pem_ram(const std::string &data);
  INLINE int load_certificates_from_der_ram(const std::string &data);

  X509_STORE *get_x509_store();

  void notify_ssl_errors();
  void notify_debug_ssl_errors();

  static OpenSSLWrapper *get_global_ptr();

private:
  X509_STORE *_x509_store;

  static OpenSSLWrapper *_global_ptr;
};

#include "openSSLWrapper.I"

#endif  // HAVE_OPENSSL
#endif
