/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file encryptStreamBuf.I
 * @author drose
 * @date 2004-12-09
 */

/**
 * Specifies the encryption algorithm that should be used for future calls to
 * open_write().  The default is whatever is specified by the encryption-
 * algorithm config variable.  The complete set of available algorithms is
 * defined by the current version of OpenSSL.
 *
 * If an invalid algorithm is specified, there is no immediate error return
 * code, but open_write() will fail.
 */
INLINE void EncryptStreamBuf::
set_algorithm(const std::string &algorithm) {
  _algorithm = algorithm;
}

/**
 * Returns the encryption algorithm that was specified by set_algorithm(), or
 * was read from the stream by the last successful open_read().
 */
INLINE const std::string &EncryptStreamBuf::
get_algorithm() const {
  return _algorithm;
}

/**
 * Specifies the length of the key, in bits, that should be used to encrypt
 * the stream in future calls to open_write().  The default is whatever is
 * specified by the encryption-key-length config variable.
 *
 * If an invalid key_length for the chosen algorithm is specified, there is no
 * immediate error return code, but open_write() will fail.
 */
INLINE void EncryptStreamBuf::
set_key_length(int key_length) {
  _key_length = key_length;
}

/**
 * Returns the encryption key length, in bits, that was specified by
 * set_key_length(), or was read from the stream by the last successful
 * open_read().
 */
INLINE int EncryptStreamBuf::
get_key_length() const {
  return _key_length;
}

/**
 * Specifies the number of times to repeatedly hash the key before writing it
 * to the stream in future calls to open_write().  Its purpose is to make it
 * computationally more expensive for an attacker to search the key space
 * exhaustively.  This should be a multiple of 1,000 and should not exceed
 * about 65 million; the value 0 indicates just one application of the hashing
 * algorithm.
 *
 * The default is whatever is specified by the encryption-iteration-count
 * config variable.
 */
INLINE void EncryptStreamBuf::
set_iteration_count(int iteration_count) {
  _iteration_count = iteration_count;
}

/**
 * Returns the value that was specified by set_iteration_count(), or was read
 * from the stream by the last successful open_read().
 */
INLINE int EncryptStreamBuf::
get_iteration_count() const {
  return _iteration_count;
}

/**
 * Sets the amount of the encrypted data at the beginning that are skipped
 * when seeking back to zero.
 */
INLINE void EncryptStreamBuf::
set_magic_length(size_t length) {
  _magic_length = length;
}

/**
 * Sets the amount of the encrypted data at the beginning that are skipped
 * when seeking back to zero.
 */
INLINE size_t EncryptStreamBuf::
get_magic_length() const {
  return _magic_length;
}
