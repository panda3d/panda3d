/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file encrypt_string.cxx
 * @author drose
 * @date 2007-01-30
 */

#include "encrypt_string.h"

#ifdef HAVE_OPENSSL
#include "encryptStream.h"
#include "virtualFileSystem.h"
#include "config_express.h"
#include "stringStream.h"

using std::istream;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::string;

/**
 * Encrypts the indicated source string using the given password, and the
 * algorithm specified by encryption-algorithm.  Returns the encrypted data.
 */
vector_uchar
encrypt_string(const string &source, const string &password,
               const string &algorithm, int key_length, int iteration_count) {
  StringStream dest;

  {
    OEncryptStream encrypt;
    if (!algorithm.empty()) {
      encrypt.set_algorithm(algorithm);
    }
    if (key_length > 0) {
      encrypt.set_key_length(key_length);
    }
    if (iteration_count >= 0) {
      encrypt.set_iteration_count(iteration_count);
    }
    encrypt.open(&dest, false, password);
    encrypt.write(source.data(), source.length());

    if (encrypt.fail()) {
      return vector_uchar();
    }
  }

  vector_uchar result;
  dest.swap_data(result);
  return result;
}

/**
 * Decrypts the previously-encrypted string using the given password (which
 * must be the same password passed to encrypt()).  The return value is the
 * decrypted string.
 *
 * Note that a decryption error, including an incorrect password, cannot
 * easily be detected, and the return value may simply be a garbage string.
 */
string
decrypt_string(const vector_uchar &source, const string &password) {
  StringStream source_stream(source);
  ostringstream dest_stream;

  if (!decrypt_stream(source_stream, dest_stream, password)) {
    return string();
  }

  return dest_stream.str();
}

/**
 * Encrypts the data from the source file using the given password.  The
 * source file is read in its entirety, and the encrypted results are written
 * to the dest file, overwriting its contents.  The return value is bool on
 * success, or false on failure.
 */
EXPCL_PANDA_EXPRESS bool
encrypt_file(const Filename &source, const Filename &dest, const string &password,
             const string &algorithm, int key_length, int iteration_count) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename source_filename = source;
  if (!source_filename.is_binary_or_text()) {
    // The default is binary, if not specified otherwise.
    source_filename.set_binary();
  }
  istream *source_stream = vfs->open_read_file(source_filename, true);
  if (source_stream == nullptr) {
    express_cat.info() << "Couldn't open file " << source_filename << "\n";
    return false;
  }

  Filename dest_filename = Filename::binary_filename(dest);
  ostream *dest_stream = vfs->open_write_file(dest_filename, true, true);
  if (dest_stream == nullptr) {
    express_cat.info() << "Couldn't open file " << dest_filename << "\n";
    vfs->close_read_file(source_stream);
    return false;
  }

  bool result = encrypt_stream(*source_stream, *dest_stream, password,
                               algorithm, key_length, iteration_count);
  vfs->close_read_file(source_stream);
  vfs->close_write_file(dest_stream);
  return result;
}

/**
 * Decrypts the data from the source file using the given password (which must
 * match the same password passed to encrypt()).  The source file is read in
 * its entirety, and the decrypted results are written to the dest file,
 * overwriting its contents.  The return value is bool on success, or false on
 * failure.
 *
 * Note that a decryption error, including an incorrect password, cannot
 * easily be detected, and the output may simply be a garbage string.
 */
EXPCL_PANDA_EXPRESS bool
decrypt_file(const Filename &source, const Filename &dest, const string &password) {
  Filename source_filename = Filename::binary_filename(source);
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *source_stream = vfs->open_read_file(source_filename, false);
  if (source_stream == nullptr) {
    express_cat.info() << "Couldn't open file " << source_filename << "\n";
    return false;
  }

  Filename dest_filename = dest;
  if (!dest_filename.is_binary_or_text()) {
    // The default is binary, if not specified otherwise.
    dest_filename.set_binary();
  }
  ostream *dest_stream = vfs->open_write_file(dest_filename, true, true);
  if (dest_stream == nullptr) {
    express_cat.info() << "Couldn't open file " << dest_filename << "\n";
    vfs->close_read_file(source_stream);
    return false;
  }

  bool result = decrypt_stream(*source_stream, *dest_stream, password);
  vfs->close_read_file(source_stream);
  vfs->close_write_file(dest_stream);
  return result;
}

/**
 * Encrypts the data from the source stream using the given password.  The
 * source stream is read from its current position to the end-of-file, and the
 * encrypted results are written to the dest stream.  The return value is bool
 * on success, or false on failure.
 */
bool
encrypt_stream(istream &source, ostream &dest, const string &password,
               const string &algorithm, int key_length, int iteration_count) {
  OEncryptStream encrypt;
  if (!algorithm.empty()) {
    encrypt.set_algorithm(algorithm);
  }
  if (key_length > 0) {
    encrypt.set_key_length(key_length);
  }
  if (iteration_count >= 0) {
    encrypt.set_iteration_count(iteration_count);
  }
  encrypt.open(&dest, false, password);

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  source.read(buffer, buffer_size);
  size_t count = source.gcount();
  while (count != 0) {
    encrypt.write(buffer, count);
    source.read(buffer, buffer_size);
    count = source.gcount();
  }
  encrypt.close();

  return (!source.fail() || source.eof()) && (!encrypt.fail());
}

/**
 * Decrypts the data from the previously-encrypted source stream using the
 * given password (which must be the same password passed to encrypt()).  The
 * source stream is read from its current position to the end-of-file, and the
 * decrypted results are written to the dest stream.  The return value is bool
 * on success, or false on failure.
 *
 * Note that a decryption error, including an incorrect password, cannot
 * easily be detected, and the output may simply be a garbage string.
 */
bool
decrypt_stream(istream &source, ostream &dest, const string &password) {
  IDecryptStream decrypt(&source, false, password);

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  decrypt.read(buffer, buffer_size);
  size_t count = decrypt.gcount();
  while (count != 0) {
    dest.write(buffer, count);
    decrypt.read(buffer, buffer_size);
    count = decrypt.gcount();
  }

  return (!decrypt.fail() || decrypt.eof()) && (!dest.fail());
}

#endif // HAVE_OPENSSL
