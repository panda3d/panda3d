/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file encryptStreamBuf.cxx
 * @author drose
 * @date 2004-09-01
 */

#include "encryptStreamBuf.h"
#include "config_prc.h"
#include "streamReader.h"
#include "streamWriter.h"
#include "configVariableInt.h"
#include "configVariableString.h"

#ifdef HAVE_OPENSSL

#include <openssl/rand.h>
#include <openssl/evp.h>

// The iteration count is scaled by this factor for writing to the stream.
static const int iteration_count_factor = 1000;

/**
 *
 */
EncryptStreamBuf::
EncryptStreamBuf() {
  _source = nullptr;
  _owns_source = false;
  _dest = nullptr;
  _owns_dest = false;

  ConfigVariableString encryption_algorithm
    ("encryption-algorithm", "bf-cbc",
     PRC_DESC("This defines the OpenSSL encryption algorithm which is used to "
              "encrypt any streams created by the current runtime.  The default is "
              "Blowfish; the complete set of available algorithms is defined by "
              "the current version of OpenSSL.  This value is used only to control "
              "encryption; the correct algorithm will automatically be selected on "
              "decryption."));

  ConfigVariableInt encryption_key_length
    ("encryption-key-length", 0,
     PRC_DESC("This defines the key length, in bits, for the selected encryption "
              "algorithm.  Some algorithms have a variable key length.  Specifying "
              "a value of 0 here means to use the default key length for the "
              "algorithm as defined by OpenSSL.  This value is used only to "
              "control encryption; the correct key length will automatically be "
              "selected on decryption."));

  ConfigVariableInt encryption_iteration_count
    ("encryption-iteration-count", 100000,
     PRC_DESC("This defines the number of times a password is hashed to generate a "
              "key when encrypting.  Its purpose is to make it computationally "
              "more expensive for an attacker to search the key space "
              "exhaustively.  This should be a multiple of 1,000 and should not "
              "exceed about 65 million; the value 0 indicates just one application "
              "of the hashing algorithm.  This value is used only to control "
              "encryption; the correct count will automatically be selected on "
              "decryption."));

  _algorithm = encryption_algorithm;
  _key_length = encryption_key_length;
  _iteration_count = encryption_iteration_count;

  _read_ctx = nullptr;
  _write_ctx = nullptr;

  _read_overflow_buffer = nullptr;
  _in_read_overflow_buffer = 0;

#ifdef PHAVE_IOSTREAM
  char *buf = new char[4096];
  char *ebuf = buf + 4096;
  setg(buf, ebuf, ebuf);
  setp(buf, ebuf);

#else
  allocate();
  setg(base(), ebuf(), ebuf());
  setp(base(), ebuf());
#endif
}

/**
 *
 */
EncryptStreamBuf::
~EncryptStreamBuf() {
  close_read();
  close_write();
}

/**
 *
 */
void EncryptStreamBuf::
open_read(std::istream *source, bool owns_source, const std::string &password) {
  OpenSSL_add_all_algorithms();

  _source = source;
  _owns_source = owns_source;

  if (_read_ctx != nullptr) {
    EVP_CIPHER_CTX_free(_read_ctx);
    _read_ctx = nullptr;
  }

  // Now read the header information.
  StreamReader sr(_source, false);
  int nid = sr.get_uint16();
  int key_length = sr.get_uint16();
  int count = sr.get_uint16();

  const EVP_CIPHER *cipher = EVP_get_cipherbynid(nid);

  if (cipher == nullptr) {
    prc_cat.error()
      << "Unknown encryption algorithm in stream.\n";
    return;
  }

  _algorithm = OBJ_nid2sn(nid);
  _key_length = key_length * 8;
  _iteration_count = count * iteration_count_factor;

  if (prc_cat.is_debug()) {
    prc_cat.debug()
      << "Using decryption algorithm " << _algorithm << " with key length "
      << _key_length << " bits.\n";
    prc_cat.debug()
      << "Key is hashed " << _iteration_count << " extra times.\n";
  }

  int iv_length = EVP_CIPHER_iv_length(cipher);
  _read_block_size = EVP_CIPHER_block_size(cipher);

  unsigned char *iv = (unsigned char *)alloca(iv_length);
  iv_length = (int)sr.extract_bytes(iv, iv_length);

  _read_ctx = EVP_CIPHER_CTX_new();
  nassertv(_read_ctx != nullptr);

  // Initialize the context
  int result;
  result = EVP_DecryptInit(_read_ctx, cipher, nullptr, (unsigned char *)iv);
  nassertv(result > 0);

  result = EVP_CIPHER_CTX_set_key_length(_read_ctx, key_length);
  if (result <= 0) {
    prc_cat.error()
      << "Invalid key length " << key_length * 8 << " bits for algorithm "
      << OBJ_nid2sn(nid) << "\n";
    EVP_CIPHER_CTX_free(_read_ctx);
    _read_ctx = nullptr;
    return;
  }

  // Hash the supplied password into a key of the appropriate length.
  unsigned char *key = (unsigned char *)alloca(key_length);
  result =
    PKCS5_PBKDF2_HMAC_SHA1((const char *)password.data(), password.length(),
                           iv, iv_length,
                           count * iteration_count_factor + 1,
                           key_length, key);
  nassertv(result > 0);

  // Store the key within the context.
  result = EVP_DecryptInit(_read_ctx, nullptr, key, nullptr);
  nassertv(result > 0);

  _read_overflow_buffer = new unsigned char[_read_block_size];
  _in_read_overflow_buffer = 0;
  _finished = false;
  thread_consider_yield();
}

/**
 *
 */
void EncryptStreamBuf::
close_read() {
  if (_read_ctx != nullptr) {
    EVP_CIPHER_CTX_free(_read_ctx);
    _read_ctx = nullptr;
  }

  if (_read_overflow_buffer != nullptr) {
    delete[] _read_overflow_buffer;
    _read_overflow_buffer = nullptr;
  }

  if (_source != nullptr) {
    if (_owns_source) {
      delete _source;
      _owns_source = false;
    }
    _source = nullptr;
  }
}

/**
 *
 */
void EncryptStreamBuf::
open_write(std::ostream *dest, bool owns_dest, const std::string &password) {
  OpenSSL_add_all_algorithms();

  close_write();
  _dest = dest;
  _owns_dest = owns_dest;

  const EVP_CIPHER *cipher =
    EVP_get_cipherbyname(_algorithm.c_str());

  if (cipher == nullptr) {
    prc_cat.error()
      << "Unknown encryption algorithm: " << _algorithm << "\n";
    return;
  }

  int nid = EVP_CIPHER_nid(cipher);

  int iv_length = EVP_CIPHER_iv_length(cipher);
  _write_block_size = EVP_CIPHER_block_size(cipher);

  // Generate a random IV.  It doesn't need to be cryptographically secure,
  // just unique.
  unsigned char *iv = (unsigned char *)alloca(iv_length);
  RAND_bytes(iv, iv_length);

  _write_ctx = EVP_CIPHER_CTX_new();
  nassertv(_write_ctx != nullptr);

  int result;
  result = EVP_EncryptInit(_write_ctx, cipher, nullptr, iv);
  nassertv(result > 0);

  // Store the appropriate key length in the context.
  int key_length = (_key_length + 7) / 8;
  if (key_length == 0) {
    key_length = EVP_CIPHER_key_length(cipher);
  }
  result = EVP_CIPHER_CTX_set_key_length(_write_ctx, key_length);
  if (result <= 0) {
    prc_cat.error()
      << "Invalid key length " << key_length * 8 << " bits for algorithm "
      << OBJ_nid2sn(nid) << "\n";
    EVP_CIPHER_CTX_free(_write_ctx);
    _write_ctx = nullptr;
    return;
  }

  int count = _iteration_count / iteration_count_factor;

  if (prc_cat.is_debug()) {
    prc_cat.debug()
      << "Using encryption algorithm " << OBJ_nid2sn(nid) << " with key length "
      << key_length * 8 << " bits.\n";
    prc_cat.debug()
      << "Hashing key " << count * iteration_count_factor
      << " extra times.\n";
  }

  // Hash the supplied password into a key of the appropriate length.
  unsigned char *key = (unsigned char *)alloca(key_length);
  result =
    PKCS5_PBKDF2_HMAC_SHA1((const char *)password.data(), password.length(),
                           iv, iv_length, count * iteration_count_factor + 1,
                           key_length, key);
  nassertv(result > 0);

  // Store the key in the context.
  result = EVP_EncryptInit(_write_ctx, nullptr, key, nullptr);
  nassertv(result > 0);

  // Now write the header information to the stream.
  StreamWriter sw(_dest, false);
  nassertv((uint16_t)nid == nid);
  sw.add_uint16((uint16_t)nid);
  nassertv((uint16_t)key_length == key_length);
  sw.add_uint16((uint16_t)key_length);
  nassertv((uint16_t)count == count);
  sw.add_uint16((uint16_t)count);
  sw.append_data(iv, iv_length);

  thread_consider_yield();
}

/**
 *
 */
void EncryptStreamBuf::
close_write() {
  if (_dest != nullptr) {
    size_t n = pptr() - pbase();
    write_chars(pbase(), n);
    pbump(-(int)n);

    if (_write_ctx != nullptr) {
      unsigned char *write_buffer = (unsigned char *)alloca(_write_block_size);
      int bytes_written = 0;
      EVP_EncryptFinal(_write_ctx, write_buffer, &bytes_written);
      thread_consider_yield();

      _dest->write((const char *)write_buffer, bytes_written);

      EVP_CIPHER_CTX_free(_write_ctx);
      _write_ctx = nullptr;
    }

    if (_owns_dest) {
      delete _dest;
      _owns_dest = false;
    }
    _dest = nullptr;
  }
}

/**
 * Implements seeking within the stream.  EncryptStreamBuf only allows seeking
 * back to the beginning of the stream.
 */
std::streampos EncryptStreamBuf::
seekoff(std::streamoff off, ios_seekdir dir, ios_openmode which) {
  if (which != std::ios::in) {
    // We can only do this with the input stream.
    return -1;
  }

  if (off != 0 || dir != std::ios::beg) {
    // We only know how to reposition to the beginning.
    return -1;
  }

  size_t n = egptr() - gptr();
  gbump(n);

  if (_source->rdbuf()->pubseekpos(0, std::ios::in) == (std::streampos)0) {
    int result = EVP_DecryptInit(_read_ctx, nullptr, nullptr, nullptr);
    nassertr_always(result > 0, -1);

    _source->clear();
    _in_read_overflow_buffer = 0;
    _finished = false;

    // Skip past the header.
    int iv_length = EVP_CIPHER_CTX_iv_length(_read_ctx);
    _source->ignore(6 + iv_length);

    // Ignore the magic bytes.
    size_t magic_length = get_magic_length();
    char *buffer = (char *)alloca(magic_length);
    if (read_chars(buffer, magic_length) == magic_length) {
      return 0;
    }
  }

  return -1;
}

/**
 * Implements seeking within the stream.  EncryptStreamBuf only allows seeking
 * back to the beginning of the stream.
 */
std::streampos EncryptStreamBuf::
seekpos(std::streampos pos, ios_openmode which) {
  return seekoff(pos, std::ios::beg, which);
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int EncryptStreamBuf::
overflow(int ch) {
  size_t n = pptr() - pbase();
  if (n != 0) {
    write_chars(pbase(), n);
    pbump(-(int)n);
  }

  if (ch != EOF) {
    // Write one more character.
    char c = (char)ch;
    write_chars(&c, 1);
  }

  return 0;
}

/**
 * Called by the system iostream implementation to implement a flush
 * operation.
 */
int EncryptStreamBuf::
sync() {
  if (_source != nullptr) {
    size_t n = egptr() - gptr();
    gbump((int)n);
  }

  if (_dest != nullptr) {
    size_t n = pptr() - pbase();
    write_chars(pbase(), n);
    pbump(-(int)n);
  }

  _dest->flush();
  return 0;
}

/**
 * Called by the system istream implementation when its internal buffer needs
 * more characters.
 */
int EncryptStreamBuf::
underflow() {
  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    size_t buffer_size = egptr() - eback();
    gbump(-(int)buffer_size);

    size_t num_bytes = buffer_size;
    size_t read_count = read_chars(gptr(), buffer_size);

    if (read_count != num_bytes) {
      // Oops, we didn't read what we thought we would.
      if (read_count == 0) {
        gbump((int)num_bytes);
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      nassertr(read_count < num_bytes, EOF);
      size_t delta = num_bytes - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump((int)delta);
    }
  }

  return (unsigned char)*gptr();
}


/**
 * Gets some characters from the source stream.
 */
size_t EncryptStreamBuf::
read_chars(char *start, size_t length) {
  if (length == 0) {
    return 0;
  }

  if (_in_read_overflow_buffer != 0) {
    // Take from the overflow buffer.
    length = std::min(length, _in_read_overflow_buffer);
    memcpy(start, _read_overflow_buffer, length);
    _in_read_overflow_buffer -= length;
    memcpy(_read_overflow_buffer + length, _read_overflow_buffer, _in_read_overflow_buffer);
    return length;
  }

  unsigned char *source_buffer = (unsigned char *)alloca(length);
  size_t max_read_buffer = length + _read_block_size;
  unsigned char *read_buffer = (unsigned char *)alloca(max_read_buffer);

  int bytes_read = 0;

  do {
    // Get more bytes from the stream.
    if (_read_ctx == nullptr || _finished) {
      return 0;
    }

    _source->read((char *)source_buffer, length);
    size_t source_length = _source->gcount();

    bytes_read = 0;
    int result;
    if (source_length != 0) {
      result =
        EVP_DecryptUpdate(_read_ctx, read_buffer, &bytes_read,
                          source_buffer, source_length);
    } else {
      result =
        EVP_DecryptFinal(_read_ctx, read_buffer, &bytes_read);
      _finished = true;
    }

    if (result <= 0) {
      prc_cat.error()
        << "Error decrypting stream.\n";
      if (_read_ctx != nullptr) {
        EVP_CIPHER_CTX_free(_read_ctx);
        _read_ctx = nullptr;
      }
    }
    thread_consider_yield();

  } while (bytes_read == 0);

  // Now store the read bytes in the output stream.
  if ((size_t)bytes_read <= length) {
    // No overflow.
    memcpy(start, read_buffer, bytes_read);
    return bytes_read;

  } else {
    // We have to save some of the returned bytes in the overflow buffer.
    _in_read_overflow_buffer = bytes_read - length;
    nassertr(_in_read_overflow_buffer <= _read_block_size, 0);

    memcpy(_read_overflow_buffer, read_buffer + length,
           _in_read_overflow_buffer);
    memcpy(start, read_buffer, length);
    return length;
  }
}

/**
 * Sends some characters to the dest stream.
 */
void EncryptStreamBuf::
write_chars(const char *start, size_t length) {
  if (_write_ctx != nullptr && length != 0) {
    size_t max_write_buffer = length + _write_block_size;
    unsigned char *write_buffer = (unsigned char *)alloca(max_write_buffer);

    int bytes_written = 0;
    int result =
      EVP_EncryptUpdate(_write_ctx, write_buffer, &bytes_written,
                        (unsigned char *)start, length);
    if (result <= 0) {
      prc_cat.error()
        << "Error encrypting stream.\n";
    }
    thread_consider_yield();
    _dest->write((const char *)write_buffer, bytes_written);
  }
}

#endif  // HAVE_OPENSSL
