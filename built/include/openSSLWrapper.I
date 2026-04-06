/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file openSSLWrapper.I
 * @author drose
 * @date 2009-09-05
 */

/**
 * Reads a chain of trusted certificates from the indicated data buffer and
 * adds them to the X509_STORE object.  The data buffer should be PEM-
 * formatted.  Returns the number of certificates read on success, or 0 on
 * failure.
 *
 * You should call this only with trusted, locally-stored certificates; not
 * with certificates received from an untrusted source.
 */
INLINE int OpenSSLWrapper::
load_certificates_from_pem_ram(const std::string &data) {
  return load_certificates_from_pem_ram(data.data(), data.size());
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
INLINE int OpenSSLWrapper::
load_certificates_from_der_ram(const std::string &data) {
  return load_certificates_from_der_ram(data.data(), data.size());
}
