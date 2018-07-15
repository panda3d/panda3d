/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file openSSLWrapper.cxx
 * @author drose
 * @date 2009-09-05
 */

#include "openSSLWrapper.h"

#ifdef HAVE_OPENSSL

#include "virtualFileSystem.h"
#include "ca_bundle_data_src.c"

OpenSSLWrapper *OpenSSLWrapper::_global_ptr = nullptr;

/**
 *
 */
OpenSSLWrapper::
OpenSSLWrapper() {
  // It is necessary to call this before making any other OpenSSL call, per
  // the docs.  Also, the docs say that making this call will seed the random
  // number generator.  Apparently you can get away with not calling it in
  // versions prior to 0.9.8, however.
  SSL_library_init();

  OpenSSL_add_all_algorithms();

  _x509_store = X509_STORE_new();
  X509_STORE_set_default_paths(_x509_store);

  // Load in the well-known certificate authorities compiled into this
  // program.
  load_certificates_from_der_ram((const char *)ca_bundle_data, ca_bundle_data_len);

  // Load in any default certificates listed in the Config.prc file.
  ConfigVariableFilename ca_bundle_filename
    ("ca-bundle-filename", "",
     PRC_DESC("This names the certificate authority file for OpenSSL "
              "to use to verify whether SSL certificates are trusted or not.  "
              "The file named by this setting should contain one or more "
              "PEM-formatted certificates from trusted certificate "
              "authorities.  This is a fairly standard file; a copy of "
              "ca-bundle.crt is included in the OpenSSL distribution, and "
              "is also included with Panda."));

  if (!ca_bundle_filename.empty()) {
    load_certificates(ca_bundle_filename);
  }

  ConfigVariableList ssl_certificates
    ("ssl-certificates",
     PRC_DESC("This variable lists additional filenames, on top of the file "
              "named by ca-bundle-filename, that contain trusted SSL "
              "certificates or certificate authorities."));

  int num_certs = ssl_certificates.get_num_unique_values();
  for (int ci = 0; ci < num_certs; ci++) {
    std::string cert_file = ssl_certificates.get_unique_value(ci);
    Filename filename = Filename::expand_from(cert_file);
    load_certificates(filename);
  }
}

/**
 *
 */
OpenSSLWrapper::
~OpenSSLWrapper() {
  // Actually, the destructor is never called.
  X509_STORE_free(_x509_store);
}

/**
 * Removes all the certificates from the global store, including the compiled-
 * in certificates loaded from ca_bundle_data.c.  You can add new certificates
 * by calling load_certificates().
 */
void OpenSSLWrapper::
clear_certificates() {
  // We do this by deleting the store and creating a new one.
  X509_STORE_free(_x509_store);
  _x509_store = X509_STORE_new();

  // We don't set the default path either.  We want a squeaky-clean store.
  // X509_STORE_set_default_paths(_x509_store);
}

/**
 * Reads the PEM-formatted certificate(s) (delimited by -----BEGIN
 * CERTIFICATE----- and -----END CERTIFICATE-----) from the indicated file and
 * adds them to the global store object, retrieved via get_x509_store().
 *
 * Returns the number of certificates read on success, or 0 on failure.
 *
 * You should call this only with trusted, locally-stored certificates; not
 * with certificates received from an untrusted source.
 */
int OpenSSLWrapper::
load_certificates(const Filename &filename) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  // First, read the complete file into memory.
  std::string data;
  if (!vfs->read_file(filename, data, true)) {
    // Could not find or read file.
    express_cat.info()
      << "Could not read " << filename << ".\n";
    return 0;
  }

  int result = load_certificates_from_pem_ram(data.data(), data.size());

  if (result <= 0) {
    express_cat.info()
      << "Could not load certificates from " << filename << ".\n";
    notify_ssl_errors();
    return 0;
  }

  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Appending " << result << " SSL certificates from "
      << filename << "\n";
  }

  return result;
}

/**
 * Reads a chain of trusted certificates from the indicated data buffer and
 * adds them to the X509_STORE object.  The data buffer should be PEM-
 * formatted.  Returns the number of certificates read on success, or 0 on
 * failure.
 *
 * You should call this only with trusted, locally-stored certificates; not
 * with certificates received from an untrusted source.
 */
int OpenSSLWrapper::
load_certificates_from_pem_ram(const char *data, size_t data_size) {
  STACK_OF(X509_INFO) *inf;

  // Create an in-memory BIO to read the "file" from the buffer, and call the
  // low-level routines to read the certificates from the BIO.
  BIO *mbio = BIO_new_mem_buf((void *)data, data_size);

  // We have to be sure and clear the OpenSSL error state before we call this
  // function, or it will get confused.
  ERR_clear_error();
  inf = PEM_X509_INFO_read_bio(mbio, nullptr, nullptr, nullptr);
  BIO_free(mbio);

  if (!inf) {
    // Could not scan certificates.
    express_cat.info()
      << "PEM_X509_INFO_read_bio() returned NULL.\n";
    notify_ssl_errors();
    return 0;
  }

  if (express_cat.is_spam()) {
    express_cat.spam()
      << "PEM_X509_INFO_read_bio() found " << sk_X509_INFO_num(inf)
      << " entries.\n";
  }

  // Now add the certificates to the store.

  int count = 0;
  int num_entries = sk_X509_INFO_num(inf);
  for (int i = 0; i < num_entries; i++) {
    X509_INFO *itmp = sk_X509_INFO_value(inf, i);

    if (itmp->x509) {
      int result = X509_STORE_add_cert(_x509_store, itmp->x509);
      if (result == 0) {
        notify_debug_ssl_errors();
      } else {
        ++count;
      }

      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Entry " << i << " is x509\n";
      }

    } else if (itmp->crl) {
      int result = X509_STORE_add_crl(_x509_store, itmp->crl);
      if (result == 0) {
        notify_debug_ssl_errors();
      } else {
        ++count;
      }

      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Entry " << i << " is crl\n";
      }

    } else if (itmp->x_pkey) {
      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Entry " << i << " is pkey\n";
      }

    } else {
      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Entry " << i << " is unknown type\n";
      }
    }
  }
  sk_X509_INFO_pop_free(inf, X509_INFO_free);

  if (express_cat.is_spam()) {
    express_cat.spam()
      << "successfully loaded " << count << " entries.\n";
  }

  return count;
}

/**
 * Reads a chain of trusted certificates from the indicated data buffer and
 * adds them to the X509_STORE object.  The data buffer should be DER-
 * formatted.  Returns the number of certificates read on success, or 0 on
 * failure.
 *
 * You should call this only with trusted, locally-stored certificates; not
 * with certificates received from an untrusted source.
 */
int OpenSSLWrapper::
load_certificates_from_der_ram(const char *data, size_t data_size) {
  if (express_cat.is_spam()) {
    express_cat.spam()
      << "load_certificates_from_der_ram(" << (void *)data
      << ", " << data_size << ")\n";
  }

  int count = 0;

#if OPENSSL_VERSION_NUMBER >= 0x00908000L
  // Beginning in 0.9.8, d2i_X509() accepted a const unsigned char **.
  const unsigned char *bp, *bp_end;
#else
  // Prior to 0.9.8, d2i_X509() accepted an unsigned char **.
  unsigned char *bp, *bp_end;
#endif

  bp = (unsigned char *)data;
  bp_end = bp + data_size;
  while (bp < bp_end) {
    X509 *x509 = d2i_X509(nullptr, &bp, bp_end - bp);
    if (x509 == nullptr) {
      notify_ssl_errors();
      break;
    }

    int result = X509_STORE_add_cert(_x509_store, x509);
    if (result == 0) {
      notify_debug_ssl_errors();
    } else {
      ++count;
    }
  }


  if (express_cat.is_spam()) {
    express_cat.spam()
      << "loaded " << count << " certificates\n";
  }

  return count;
}

/**
 * Returns the global X509_STORE object.
 *
 * It has to be a global object, because OpenSSL seems to store some global
 * pointers associated with this object whether you want it to or not, and
 * keeping independent copies of a local X509_STORE object doesn't seem to
 * work that well.  So, we have one store that keeps all certificates the
 * application might need.
 */
X509_STORE *OpenSSLWrapper::
get_x509_store() {
  return _x509_store;
}

/**
 * A convenience function that is itself a wrapper around the OpenSSL
 * convenience function to output the recent OpenSSL errors.  This function
 * sends the error string to express_cat.warning().  If REPORT_OPENSSL_ERRORS
 * is not defined, the function does nothing.
 */
void OpenSSLWrapper::
notify_ssl_errors() {
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
    express_cat.warning() << buffer << "\n";
    e = ERR_get_error();
  }
#endif  //  REPORT_OPENSSL_ERRORS
}

/**
 * As notify_ssl_errors(), but sends the output to debug instead of warning.
 */
void OpenSSLWrapper::
notify_debug_ssl_errors() {
#ifdef REPORT_OPENSSL_ERRORS
  static bool strings_loaded = false;
  if (!strings_loaded) {
    SSL_load_error_strings();
    strings_loaded = true;
  }

  unsigned long e = ERR_get_error();
  while (e != 0) {
    if (express_cat.is_debug()) {
      static const size_t buffer_len = 256;
      char buffer[buffer_len];
      ERR_error_string_n(e, buffer, buffer_len);
      express_cat.debug() << buffer << "\n";
    }
    e = ERR_get_error();
  }
#endif  //  REPORT_OPENSSL_ERRORS
}

/**
 *
 */
OpenSSLWrapper *OpenSSLWrapper::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new OpenSSLWrapper;
  }
  return _global_ptr;
}

#endif  // HAVE_OPENSSL
