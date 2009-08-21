// Filename: encryptStreamBuf.cxx
// Created by:  drose (01Sep04)
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

#include "encryptStreamBuf.h"
#include "config_prc.h"
#include "streamReader.h"
#include "streamWriter.h"
#include "configVariableInt.h"
#include "configVariableString.h"

#ifdef HAVE_OPENSSL

#include "openssl/rand.h"

#ifndef HAVE_STREAMSIZE
// Some compilers (notably SGI) don't define this for us
typedef int streamsize;
#endif /* HAVE_STREAMSIZE */

// The iteration count is scaled by this factor for writing to the
// stream.
static const int iteration_count_factor = 1000;

////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EncryptStreamBuf::
EncryptStreamBuf() {
  _source = (istream *)NULL;
  _owns_source = false;
  _dest = (ostream *)NULL;
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

  _read_valid = false;
  _write_valid = false;

  _read_overflow_buffer = NULL;
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

////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
EncryptStreamBuf::
~EncryptStreamBuf() {
  close_read();
  close_write();
}

////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::open_read
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EncryptStreamBuf::
open_read(istream *source, bool owns_source, const string &password) {
  OpenSSL_add_all_algorithms();

  _source = source;
  _owns_source = owns_source;
  _read_valid = false;

  // Now read the header information.
  StreamReader sr(_source, false);
  int nid = sr.get_uint16();
  int key_length = sr.get_uint16();
  int count = sr.get_uint16();

  const EVP_CIPHER *cipher = EVP_get_cipherbynid(nid);

  if (cipher == NULL) {
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

  string iv = sr.extract_bytes(iv_length);

  // Initialize the context
  int result;
  result = EVP_DecryptInit(&_read_ctx, cipher, NULL, (unsigned char *)iv.data());
  nassertv(result > 0);

  result = EVP_CIPHER_CTX_set_key_length(&_read_ctx, key_length);
  if (result <= 0) {
    prc_cat.error()
      << "Invalid key length " << key_length * 8 << " bits for algorithm "
      << OBJ_nid2sn(nid) << "\n";
    EVP_CIPHER_CTX_cleanup(&_read_ctx);
    return;
  }

  // Hash the supplied password into a key of the appropriate length.
  unsigned char *key = (unsigned char *)alloca(key_length);
  result =
    PKCS5_PBKDF2_HMAC_SHA1((const char *)password.data(), password.length(),
                           (unsigned char *)iv.data(), iv.length(), 
                           count * iteration_count_factor + 1, 
                           key_length, key);
  nassertv(result > 0);

  // Store the key within the context.
  result = EVP_DecryptInit(&_read_ctx, NULL, key, NULL);
  nassertv(result > 0);

  _read_valid = true;

  _read_overflow_buffer = new unsigned char[_read_block_size];
  _in_read_overflow_buffer = 0;
  thread_consider_yield();
}

////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::close_read
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EncryptStreamBuf::
close_read() {
  if (_read_valid) {
    EVP_CIPHER_CTX_cleanup(&_read_ctx);
    _read_valid = false;
  }

  if (_read_overflow_buffer != (unsigned char *)NULL) {
    delete[] _read_overflow_buffer;
    _read_overflow_buffer = NULL;
  }

  if (_source != (istream *)NULL) {
    if (_owns_source) {
      delete _source;
      _owns_source = false;
    }
    _source = (istream *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::open_write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EncryptStreamBuf::
open_write(ostream *dest, bool owns_dest, const string &password) {
  OpenSSL_add_all_algorithms();

  close_write();
  _dest = dest;
  _owns_dest = owns_dest;
  _write_valid = false;

  const EVP_CIPHER *cipher = 
    EVP_get_cipherbyname(_algorithm.c_str());

  if (cipher == NULL) {
    prc_cat.error()
      << "Unknown encryption algorithm: " << _algorithm << "\n";
    return;
  };

  int nid = EVP_CIPHER_nid(cipher);
    
  int iv_length = EVP_CIPHER_iv_length(cipher);
  _write_block_size = EVP_CIPHER_block_size(cipher);

  unsigned char *iv = (unsigned char *)alloca(iv_length);

  // Generate a random IV.  It doesn't need to be cryptographically
  // secure, just unique.
  RAND_pseudo_bytes(iv, iv_length);

  int result;
  result = EVP_EncryptInit(&_write_ctx, cipher, NULL, iv);
  nassertv(result > 0);

  // Store the appropriate key length in the context.
  int key_length = (_key_length + 7) / 8;
  if (key_length == 0) {
    key_length = EVP_CIPHER_key_length(cipher);
  }
  result = EVP_CIPHER_CTX_set_key_length(&_write_ctx, key_length);
  if (result <= 0) {
    prc_cat.error()
      << "Invalid key length " << key_length * 8 << " bits for algorithm "
      << OBJ_nid2sn(nid) << "\n";
    EVP_CIPHER_CTX_cleanup(&_write_ctx);
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
  result = EVP_EncryptInit(&_write_ctx, NULL, key, NULL);
  nassertv(result > 0);

  // Now write the header information to the stream.
  StreamWriter sw(_dest, false);
  nassertv((PN_uint16)nid == nid);
  sw.add_uint16(nid);
  nassertv((PN_uint16)key_length == key_length);
  sw.add_uint16(key_length);
  nassertv((PN_uint16)count == count);
  sw.add_uint16(count);
  sw.append_data(iv, iv_length);

  _write_valid = true;
  thread_consider_yield();
}

////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::close_write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EncryptStreamBuf::
close_write() {
  if (_dest != (ostream *)NULL) {
    size_t n = pptr() - pbase();
    write_chars(pbase(), n);
    pbump(-(int)n);
    
    if (_write_valid) {
      unsigned char *write_buffer = (unsigned char *)alloca(_write_block_size);
      int bytes_written = 0;
      EVP_EncryptFinal(&_write_ctx, write_buffer, &bytes_written);
      thread_consider_yield();
      
      _dest->write((const char *)write_buffer, bytes_written);
      
      _write_valid = false;
    }

    if (_owns_dest) {
      delete _dest;
      _owns_dest = false;
    }
    _dest = (ostream *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::overflow
//       Access: Protected, Virtual
//  Description: Called by the system ostream implementation when its
//               internal buffer is filled, plus one character.
////////////////////////////////////////////////////////////////////
int EncryptStreamBuf::
overflow(int ch) {
  size_t n = pptr() - pbase();
  if (n != 0) {
    write_chars(pbase(), n);
    pbump(-(int)n);
  }

  if (ch != EOF) {
    // Write one more character.
    char c = ch;
    write_chars(&c, 1);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::sync
//       Access: Protected, Virtual
//  Description: Called by the system iostream implementation to
//               implement a flush operation.
////////////////////////////////////////////////////////////////////
int EncryptStreamBuf::
sync() {
  if (_source != (istream *)NULL) {
    size_t n = egptr() - gptr();
    gbump(n);
  }

  if (_dest != (ostream *)NULL) {
    size_t n = pptr() - pbase();
    write_chars(pbase(), n);
    pbump(-(int)n);
  }
  
  _dest->flush();
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::underflow
//       Access: Protected, Virtual
//  Description: Called by the system istream implementation when its
//               internal buffer needs more characters.
////////////////////////////////////////////////////////////////////
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
        gbump(num_bytes);
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      nassertr(read_count < num_bytes, EOF);
      size_t delta = num_bytes - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump(delta);
    }
  }

  return (unsigned char)*gptr();
}


////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::read_chars
//       Access: Private
//  Description: Gets some characters from the source stream.
////////////////////////////////////////////////////////////////////
size_t EncryptStreamBuf::
read_chars(char *start, size_t length) {
  if (length == 0) {
    return 0;
  }

  if (_in_read_overflow_buffer != 0) {
    // Take from the overflow buffer.
    length = min(length, _in_read_overflow_buffer);
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
    if (!_read_valid) {
      return 0;
    }
    
    _source->read((char *)source_buffer, length);
    size_t source_length = _source->gcount();

    bytes_read = 0;
    int result;
    if (source_length != 0) {
      result =
        EVP_DecryptUpdate(&_read_ctx, read_buffer, &bytes_read,
                          source_buffer, source_length);
    } else {
      result =
        EVP_DecryptFinal(&_read_ctx, read_buffer, &bytes_read);
      _read_valid = false;
    }

    if (result <= 0) {
      prc_cat.error()
        << "Error decrypting stream.\n";
      if (_read_valid) {
        EVP_CIPHER_CTX_cleanup(&_read_ctx);
        _read_valid = false;
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
    // We have to save some of the returned bytes in the overflow
    // buffer.
    _in_read_overflow_buffer = bytes_read - length;
    nassertr(_in_read_overflow_buffer <= _read_block_size, 0);

    memcpy(_read_overflow_buffer, read_buffer + length, 
           _in_read_overflow_buffer);
    memcpy(start, read_buffer, length);
    return length;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EncryptStreamBuf::write_chars
//       Access: Private
//  Description: Sends some characters to the dest stream.
////////////////////////////////////////////////////////////////////
void EncryptStreamBuf::
write_chars(const char *start, size_t length) {
  if (_write_valid && length != 0) {
    size_t max_write_buffer = length + _write_block_size;
    unsigned char *write_buffer = (unsigned char *)alloca(max_write_buffer);
    
    int bytes_written = 0;
    int result = 
      EVP_EncryptUpdate(&_write_ctx, write_buffer, &bytes_written,
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
