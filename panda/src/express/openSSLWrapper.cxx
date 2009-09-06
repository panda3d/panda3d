// Filename: openSSLWrapper.cxx
// Created by:  drose (05Sep09)
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

#include "openSSLWrapper.h"

#ifdef HAVE_OPENSSL

#include "virtualFileSystem.h"

OpenSSLWrapper *OpenSSLWrapper::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: OpenSSLWrapper::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
OpenSSLWrapper::
OpenSSLWrapper() {
  // It is necessary to call this before making any other OpenSSL
  // call, per the docs.  Also, the docs say that making this call
  // will seed the random number generator.  Apparently you can get
  // away with not calling it in versions prior to 0.9.8, however.
  SSL_library_init();

  OpenSSL_add_all_algorithms();

  _x509_store = X509_STORE_new();
  X509_STORE_set_default_paths(_x509_store);

  // Load in any default certificates listed in the Config.prc file.
  int num_certs = ssl_certificates.get_num_unique_values();
  for (int ci = 0; ci < num_certs; ci++) {
    string cert_file = ssl_certificates.get_unique_value(ci);
    Filename filename = Filename::expand_from(cert_file);
    load_certificates(filename);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: OpenSSLWrapper::Destructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
OpenSSLWrapper::
~OpenSSLWrapper() {
  // Actually, the destructor is never called.
  X509_STORE_free(_x509_store);
}

////////////////////////////////////////////////////////////////////
//     Function: OpenSSLWrapper::load_certificates
//       Access: Public
//  Description: Reads the certificate(s) (delimited by -----BEGIN
//               CERTIFICATE----- and -----END CERTIFICATE-----) from
//               the indicated file and adds them to the global store
//               object, retrieved via get_x509_store().  
//
//               Returns the number of certificates read on success,
//               or 0 on failure.
//
//               You should call this only with trusted,
//               locally-stored certificates; not with certificates
//               received from an untrusted source.
////////////////////////////////////////////////////////////////////
int OpenSSLWrapper::
load_certificates(const Filename &filename) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  // First, read the complete file into memory.
  string data;
  if (!vfs->read_file(filename, data, true)) {
    // Could not find or read file.
    express_cat.info()
      << "Could not read " << filename << ".\n";
    return 0;
  }

  int result = load_certificates_from_ram(data.data(), data.size());

  if (result <= 0) {
    express_cat.info()
      << "Could not load certificates from " << filename << ".\n";
    notify_ssl_errors();
    return 0;
  }

  express_cat.info()
    << "Appending " << result << " SSL certificates from "
    << filename << "\n";

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenSSLWrapper::load_certificates_from_ram
//       Access: Public
//  Description: Reads a chain of trusted certificates from the
//               indicated data buffer and adds them to the X509_STORE
//               object.  Returns the number of certificates read on
//               success, or 0 on failure.
//
//               You should call this only with trusted,
//               locally-stored certificates; not with certificates
//               received from an untrusted source.
////////////////////////////////////////////////////////////////////
int OpenSSLWrapper::
load_certificates_from_ram(const char *data, size_t data_size) {
  STACK_OF(X509_INFO) *inf;

  // Create an in-memory BIO to read the "file" from the buffer, and
  // call the low-level routines to read the certificates from the
  // BIO.
  BIO *mbio = BIO_new_mem_buf((void *)data, data_size);

  // We have to be sure and clear the OpenSSL error state before we
  // call this function, or it will get confused.
  ERR_clear_error();
  inf = PEM_X509_INFO_read_bio(mbio, NULL, NULL, NULL);
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
      X509_STORE_add_cert(_x509_store, itmp->x509);
      count++;
      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Entry " << i << " is x509\n";
      }

    } else if (itmp->crl) {
      X509_STORE_add_crl(_x509_store, itmp->crl);
      count++;
      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Entry " << i << " is crl\n";
      }

    } else if (itmp->x_pkey) {
      //      X509_STORE_add_crl(_x509_store, itmp->x_pkey);
      //      count++;
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

  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenSSLWrapper::get_x509_store
//       Access: Public
//  Description: Returns the global X509_STORE object.
//
//               It has to be a global object, because OpenSSL seems
//               to store some global pointers associated with this
//               object whether you want it to or not, and keeping
//               independent copies of a local X509_STORE object
//               doesn't seem to work that well.  So, we have one
//               store that keeps all certificates the application
//               might need.
////////////////////////////////////////////////////////////////////
X509_STORE *OpenSSLWrapper::
get_x509_store() {
  return _x509_store;
}

////////////////////////////////////////////////////////////////////
//     Function: OpenSSLWrapper::notify_ssl_errors
//       Access: Public
//  Description: A convenience function that is itself a wrapper
//               around the OpenSSL convenience function to output the
//               recent OpenSSL errors.  This function sends the error
//               string to express_cat.warning().  If
//               REPORT_OPENSSL_ERRORS is not defined, the function
//               does nothing.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: OpenSSLWrapper::get_global_ptr
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
OpenSSLWrapper *OpenSSLWrapper::
get_global_ptr() {
  if (_global_ptr == NULL) {
    _global_ptr = new OpenSSLWrapper;
  }
  return _global_ptr;
}

#endif  // HAVE_OPENSSL
