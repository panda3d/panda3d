/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file multifile.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "multifile.h"

#include "config_express.h"
#include "streamWriter.h"
#include "streamReader.h"
#include "datagram.h"
#include "zStream.h"
#include "encryptStream.h"
#include "virtualFileSystem.h"
#include "virtualFile.h"

#include <algorithm>
#include <iterator>
#include <time.h>

#include "openSSLWrapper.h"

using std::ios;
using std::iostream;
using std::istream;
using std::max;
using std::min;
using std::ostream;
using std::ostringstream;
using std::streamoff;
using std::streampos;
using std::streamsize;
using std::stringstream;
using std::string;

// This sequence of bytes begins each Multifile to identify it as a Multifile.
const char Multifile::_header[] = "pmf\0\n\r";
const size_t Multifile::_header_size = 6;

// These numbers identify the version of the Multifile.  Generally, a change
// in the major version is intolerable; while a Multifile with an older minor
// version may still be read.
const int Multifile::_current_major_ver = 1;

const int Multifile::_current_minor_ver = 1;
// Bumped to version 1.1 on 6806 to add timestamps.

// To confirm that the supplied password matches, we write the Mutifile magic
// header at the beginning of the encrypted stream.  I suppose this does
// compromise the encryption security a tiny bit by making it easy for
// crackers to validate that a particular password guess matches or doesn't
// match, but the encryption algorithm doesn't depend on this being difficult
// anyway.
const char Multifile::_encrypt_header[] = "crypty";
const size_t Multifile::_encrypt_header_size = 6;



/*
 * A Multifile consists of the following elements: (1) A header.  This is
 * always the first n bytes of the Multifile, and contains a magic number to
 * identify the file, as well as version numbers and any file-specific
 * parameters.  char[6]    The string Multifile::_header, a magic number.
 * int16      The file's major version number int16      The file's minor
 * version number uint32     Scale factor.  This scales all address references
 * within the file.  Normally 1, this may be set larger to support Multifiles
 * larger than 4GB. uint32     An overall modification timestamp for the
 * entire multifile.
 */

/*
 * (2) Zero or more index entries, one for each subfile within the Multifile.
 * These entries are of variable length.  The first one of these immediately
 * follows the header, and the first word of each index entry contains the
 * address of the next index entry.  A zero "next" address marks the end of
 * the chain.  These may appear at any point within the Multifile; they do not
 * necessarily appear in sequential order at the beginning of the file
 * (although they will after the file has been "packed"). uint32     The
 * address of the next entry.  0 to mark the end.  uint32     The address of
 * this subfile's data record.  uint32     The length in bytes of this
 * subfile's data record.  uint16     The Subfile::_flags member.  [uint32]
 * The original, uncompressed and unencrypted length of the subfile, if it is
 * compressed or encrypted.  This field is only present if one or both of the
 * SF_compressed or SF_encrypted bits are set in _flags.  uint32     A
 * modification timestamp for the subfile.  uint16     The length in bytes of
 * the subfile's name.  char[n]    The subfile's name.  (3) Zero or more data
 * entries, one for each subfile.  These may appear at any point within the
 * Multifile; they do not necessarily follow each index entry, nor are they
 * necessarily all grouped together at the end (although they will be all
 * grouped together at the end after the file has been "packed").  These are
 * just blocks of literal data.
 */

/**
 *
 */
Multifile::
Multifile() :
  _read_filew(_read_file),
  _read_write_filew(_read_write_file)
{
  ConfigVariableInt multifile_encryption_iteration_count
    ("multifile-encryption-iteration-count", 0,
     PRC_DESC("This is a special value of encryption-iteration-count used to encrypt "
              "subfiles within a multifile.  It has a default value of 0 (just one "
              "application), on the assumption that the files from a multifile must "
              "be loaded quickly, without paying the cost of an expensive hash on "
              "each subfile in order to decrypt it."));

  _read = nullptr;
  _write = nullptr;
  _offset = 0;
  _owns_stream = false;
  _next_index = 0;
  _last_index = 0;
  _last_data_byte = 0;
  _needs_repack = false;
  _timestamp = 0;
  _timestamp_dirty = false;
  _record_timestamp = true;
  _scale_factor = 1;
  _new_scale_factor = 1;
  _encryption_flag = false;
  _encryption_iteration_count = multifile_encryption_iteration_count;
  _file_major_ver = 0;
  _file_minor_ver = 0;

#ifdef HAVE_OPENSSL
  // Get these values from the config file via an EncryptStreamBuf.
  EncryptStreamBuf tbuf;
  _encryption_algorithm = tbuf.get_algorithm();
  _encryption_key_length = tbuf.get_key_length();
#endif
}

/**
 *
 */
Multifile::
~Multifile() {
  close();
}

/**
 * Opens the named Multifile on disk for reading.  The Multifile index is read
 * in, and the list of subfiles becomes available; individual subfiles may
 * then be extracted or read, but the list of subfiles may not be modified.
 *
 * Also see the version of open_read() which accepts an istream.  Returns true
 * on success, false on failure.
 */
bool Multifile::
open_read(const Filename &multifile_name, const streampos &offset) {
  close();
  Filename fname = multifile_name;
  fname.set_binary();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vfile = vfs->get_file(fname);
  if (vfile == nullptr) {
    return false;
  }
  istream *multifile_stream = vfile->open_read_file(false);
  if (multifile_stream == nullptr) {
    return false;
  }

  _timestamp = vfile->get_timestamp();
  _timestamp_dirty = true;
  _read = new IStreamWrapper(multifile_stream, true);
  _owns_stream = true;
  _multifile_name = multifile_name;
  _offset = offset;
  return read_index();
}

/**
 * Opens an anonymous Multifile for reading using an istream.  There must be
 * seek functionality via seekg() and tellg() on the istream.
 *
 * If owns_pointer is true, then the Multifile assumes ownership of the stream
 * pointer and will delete it when the multifile is closed, including if this
 * function returns false.
 */
bool Multifile::
open_read(IStreamWrapper *multifile_stream, bool owns_pointer,
          const streampos &offset) {
  close();
  _timestamp = time(nullptr);
  _timestamp_dirty = true;
  _read = multifile_stream;
  _owns_stream = owns_pointer;
  _offset = offset;
  return read_index();
}

/**
 * Opens the named Multifile on disk for writing.  If there already exists a
 * file by that name, it is truncated.  The Multifile is then prepared for
 * accepting a brand new set of subfiles, which will be written to the
 * indicated filename.  Individual subfiles may not be extracted or read.
 *
 * Also see the version of open_write() which accepts an ostream.  Returns
 * true on success, false on failure.
 */
bool Multifile::
open_write(const Filename &multifile_name) {
  close();
  Filename fname = multifile_name;
  fname.set_binary();
  if (!fname.open_write(_write_file, true)) {
    return false;
  }
  _timestamp = time(nullptr);
  _timestamp_dirty = true;
  _write = &_write_file;
  _multifile_name = multifile_name;
  return true;
}

/**
 * Opens an anonymous Multifile for writing using an ostream.  There must be
 * seek functionality via seekp() and tellp() on the pstream.
 *
 * If owns_pointer is true, then the Multifile assumes ownership of the stream
 * pointer and will delete it when the multifile is closed, including if this
 * function returns false.
 */
bool Multifile::
open_write(ostream *multifile_stream, bool owns_pointer) {
  close();
  _timestamp = time(nullptr);
  _timestamp_dirty = true;
  _write = multifile_stream;
  _owns_stream = owns_pointer;
  _write->seekp(0, ios::beg);
  return true;
}

/**
 * Opens the named Multifile on disk for reading and writing.  If there
 * already exists a file by that name, its index is read.  Subfiles may be
 * added or removed, and the resulting changes will be written to the named
 * file.
 *
 * Also see the version of open_read_write() which accepts an iostream.
 * Returns true on success, false on failure.
 */
bool Multifile::
open_read_write(const Filename &multifile_name) {
  close();
  Filename fname = multifile_name;
  fname.set_binary();
  bool exists = fname.exists();
  if (!fname.open_read_write(_read_write_file)) {
    return false;
  }
  if (exists) {
    _timestamp = fname.get_timestamp();
  } else {
    _timestamp = time(nullptr);
  }
  _timestamp_dirty = true;
  _read = &_read_write_filew;
  _write = &_read_write_file;
  _multifile_name = multifile_name;

  if (exists) {
    return read_index();
  } else {
    return true;
  }
}

/**
 * Opens an anonymous Multifile for reading and writing using an iostream.
 * There must be seek functionality via seekg()/seekp() and tellg()/tellp() on
 * the iostream.
 *
 * If owns_pointer is true, then the Multifile assumes ownership of the stream
 * pointer and will delete it when the multifile is closed, including if this
 * function returns false.
 */
bool Multifile::
open_read_write(iostream *multifile_stream, bool owns_pointer) {
  close();
  _timestamp = time(nullptr);
  _timestamp_dirty = true;

  // We don't support locking when opening a file in read-write mode, because
  // we don't bother with locking on write.  But we need to have an
  // IStreamWrapper to assign to the _read member, so we create one on-the-fly
  // here.
  _read = new StreamWrapper(multifile_stream, owns_pointer);
  _write = multifile_stream;
  _owns_stream = true;  // Because we own the StreamWrapper, above.
  _write->seekp(0, ios::beg);

  // Check whether the read stream is empty.
  multifile_stream->seekg(0, ios::end);
  if (multifile_stream->tellg() == (streampos)0) {
    // The read stream is empty, which is always valid.
    return true;
  }

  // The read stream is not empty, so we'd better have a valid Multifile.
  return read_index();
}

/**
 * Closes the Multifile if it is open.  All changes are flushed to disk, and
 * the file becomes invalid for further operations until the next call to
 * open().
 */
void Multifile::
close() {
  if (_new_scale_factor != _scale_factor) {
    // If we have changed the scale factor recently, we need to force a
    // repack.
    repack();
  } else {
    flush();
  }

  if (_owns_stream) {
    // We prefer to delete the IStreamWrapper over the ostream, if possible.
    if (_read != nullptr) {
      // Only delete it if no SubStream is still referencing it.
      if (!_read->unref()) {
        delete _read;
      }
    } else if (_write != nullptr) {
      delete _write;
    }
  }

  _read = nullptr;
  _write = nullptr;
  _offset = 0;
  _owns_stream = false;
  _next_index = 0;
  _last_index = 0;
  _needs_repack = false;
  _timestamp = 0;
  _timestamp_dirty = false;
  _scale_factor = 1;
  _new_scale_factor = 1;
  _encryption_flag = false;
  _file_major_ver = 0;
  _file_minor_ver = 0;

  _read_file.close();
  _write_file.close();
  _read_write_file.close();
  _multifile_name = Filename();

  clear_subfiles();
}

/**
 * Changes the internal scale factor for this Multifile.
 *
 * This is normally 1, but it may be set to any arbitrary value (greater than
 * zero) to support Multifile archives that exceed 4GB, if necessary.
 * (Individual subfiles may still not exceed 4GB.)
 *
 * All addresses within the file are rounded up to the next multiple of
 * _scale_factor, and zeros are written to the file to fill the resulting
 * gaps.  Then the address is divided by _scale_factor and written out as a
 * 32-bit integer.  Thus, setting a scale factor of 2 supports up to 8GB
 * files, 3 supports 12GB files, etc.
 *
 * Calling this function on an already-existing Multifile will have no
 * immediate effect until a future call to repack() or close() (or until the
 * Multifile is destructed).
 */
void Multifile::
set_scale_factor(size_t scale_factor) {
  nassertv(is_write_valid());
  nassertv(scale_factor != (size_t)0);

  if (_next_index == (streampos)0) {
    // If it's a brand new Multifile, we can go ahead and set it immediately.
    _scale_factor = scale_factor;
  } else {
    // Otherwise, we'd better have read access so we can repack it later.
    nassertv(is_read_valid());
  }

  // Setting the _new_scale_factor different from the _scale_factor will force
  // a repack operation on close.
  _new_scale_factor = scale_factor;
}

/**
 * Adds a file on disk as a subfile to the Multifile.  The file named by
 * filename will be read and added to the Multifile at the next call to
 * flush().  If there already exists a subfile with the indicated name, it is
 * replaced without examining its contents (but see also update_subfile).
 *
 * Either Filename:::set_binary() or set_text() must have been called
 * previously to specify the nature of the source file.  If set_text() was
 * called, the text flag will be set on the subfile.
 *
 * Returns the subfile name on success (it might have been modified slightly),
 * or empty string on failure.
 */
string Multifile::
add_subfile(const string &subfile_name, const Filename &filename,
            int compression_level) {
  nassertr(is_write_valid(), string());

  Filename fname = filename;
  if (multifile_always_binary) {
    fname.set_binary();
  }

  nassertr(fname.is_binary_or_text(), string());

  if (!fname.exists()) {
    return string();
  }
  string name = standardize_subfile_name(subfile_name);
  if (!name.empty()) {
    Subfile *subfile = new Subfile;
    subfile->_name = name;
    subfile->_source_filename = fname;
    if (fname.is_text()) {
      subfile->_flags |= SF_text;
    }

    add_new_subfile(subfile, compression_level);
  }

  _timestamp = time(nullptr);
  _timestamp_dirty = true;

  return name;
}

/**
 * Adds a file from a stream as a subfile to the Multifile.  The indicated
 * istream will be read and its contents added to the Multifile at the next
 * call to flush(). The file will be added as a binary subfile.
 *
 * Note that the istream must remain untouched and unused by any other code
 * until flush() is called.  At that time, the Multifile will read the entire
 * contents of the istream from the current file position to the end of the
 * file.  Subsequently, the Multifile will *not* close or delete the istream.
 * It is the caller's responsibility to ensure that the istream pointer does
 * not destruct during the lifetime of the Multifile.
 *
 * Returns the subfile name on success (it might have been modified slightly),
 * or empty string on failure.
 */
string Multifile::
add_subfile(const string &subfile_name, istream *subfile_data,
            int compression_level) {
  nassertr(is_write_valid(), string());

  string name = standardize_subfile_name(subfile_name);
  if (!name.empty()) {
    Subfile *subfile = new Subfile;
    subfile->_name = name;
    subfile->_source = subfile_data;
    add_new_subfile(subfile, compression_level);
  }

  return name;
}

/**
 * Adds a file on disk to the subfile.  If a subfile already exists with the
 * same name, its contents are compared byte-for-byte to the disk file, and it
 * is replaced only if it is different; otherwise, the multifile is left
 * unchanged.
 *
 * Either Filename:::set_binary() or set_text() must have been called
 * previously to specify the nature of the source file.  If set_text() was
 * called, the text flag will be set on the subfile.
 */
string Multifile::
update_subfile(const string &subfile_name, const Filename &filename,
               int compression_level) {
  nassertr(is_write_valid(), string());

  Filename fname = filename;
  if (multifile_always_binary) {
    fname.set_binary();
  }

  nassertr(fname.is_binary_or_text(), string());

  if (!fname.exists()) {
    return string();
  }
  string name = standardize_subfile_name(subfile_name);
  if (!name.empty()) {
    int index = find_subfile(name);
    if (index >= 0) {
      // The subfile already exists; compare it to the source file.
      if (compare_subfile(index, fname)) {
        // The files are identical; do nothing.
        return name;
      }
    }

    // The subfile does not already exist or it is different from the source
    // file.  Add the new source file.
    Subfile *subfile = new Subfile;
    subfile->_name = name;
    subfile->_source_filename = fname;
    if (fname.is_text()) {
      subfile->_flags |= SF_text;
    }

    add_new_subfile(subfile, compression_level);
  }

  _timestamp = time(nullptr);
  _timestamp_dirty = true;

  return name;
}

#ifdef HAVE_OPENSSL
/**
 * Ownership of the X509 object is passed into the CertRecord; it will be
 * freed when the CertRecord destructs.
 */
Multifile::CertRecord::
CertRecord(X509 *cert) :
  _cert(cert)
{
}

/**
 *
 */
Multifile::CertRecord::
CertRecord(const Multifile::CertRecord &copy) :
  _cert(X509_dup(copy._cert))
{
}

/**
 *
 */
Multifile::CertRecord::
~CertRecord() {
  X509_free(_cert);
}

/**
 *
 */
void Multifile::CertRecord::
operator = (const Multifile::CertRecord &other) {
  X509_free(_cert);
  _cert = X509_dup(other._cert);
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Adds a new signature to the Multifile.  This signature associates the
 * indicated certificate with the current contents of the Multifile.  When the
 * Multifile is read later, the signature will still be present only if the
 * Multifile is unchanged; any subsequent changes to the Multifile will
 * automatically invalidate and remove the signature.
 *
 * The chain filename may be empty if the certificate does not require an
 * authenticating certificate chain (e.g.  because it is self-signed).
 *
 * The specified private key must match the certificate, and the Multifile
 * must be open in read-write mode.  The private key is only used for
 * generating the signature; it is not written to the Multifile and cannot be
 * retrieved from the Multifile later.  (However, the certificate *can* be
 * retrieved from the Multifile later, to identify the entity that created the
 * signature.)
 *
 * This implicitly causes a repack() operation if one is needed.  Returns true
 * on success, false on failure.
 *
 * This flavor of add_signature() reads the certificate and private key from a
 * PEM-formatted file, for instance as generated by the openssl command.  If
 * the private key file is password-encrypted, the third parameter will be
 * used as the password to decrypt it.
 */
bool Multifile::
add_signature(const Filename &certificate, const Filename &chain,
              const Filename &pkey, const string &password) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  if (chain.empty() && pkey.empty()) {
    // If the second two filenames are empty, assume we're going for the
    // composite mode, where everything's stuffed into the first file.
    return add_signature(certificate, password);
  }

  CertChain cert_chain;

  // Read the certificate file from VFS.  First, read the complete file into
  // memory.
  string certificate_data;
  if (!vfs->read_file(certificate, certificate_data, true)) {
    express_cat.info()
      << "Could not read " << certificate << ".\n";
    return false;
  }

  // Create an in-memory BIO to read the "file" from the buffer.
  BIO *certificate_mbio = BIO_new_mem_buf((void *)certificate_data.data(), certificate_data.size());
  X509 *x509 = PEM_read_bio_X509(certificate_mbio, nullptr, nullptr, (void *)"");
  BIO_free(certificate_mbio);
  if (x509 == nullptr) {
    express_cat.info()
      << "Could not read certificate in " << certificate << ".\n";
    return false;
  }

  // Store the first X509--the actual certificate--as the first record in our
  // CertChain object.
  cert_chain.push_back(CertRecord(x509));

  // Read the rest of the certificates in the chain file.
  if (!chain.empty()) {
    string chain_data;
    if (!vfs->read_file(chain, chain_data, true)) {
      express_cat.info()
        << "Could not read " << chain << ".\n";
      return false;
    }

    BIO *chain_mbio = BIO_new_mem_buf((void *)chain_data.data(), chain_data.size());
    X509 *c = PEM_read_bio_X509(chain_mbio, nullptr, nullptr, (void *)"");
    while (c != nullptr) {
      cert_chain.push_back(c);
      c = PEM_read_bio_X509(chain_mbio, nullptr, nullptr, (void *)"");
    }
    BIO_free(chain_mbio);

    if (cert_chain.size() == 1) {
      express_cat.info()
        << "Could not read certificate chain in " << chain << ".\n";
      return false;
    }
  }

  // Now do the same thing with the private key.  This one may be password-
  // encrypted on disk.
  string pkey_data;
  if (!vfs->read_file(pkey, pkey_data, true)) {
    express_cat.info()
      << "Could not read " << pkey << ".\n";
    return false;
  }

  BIO *pkey_mbio = BIO_new_mem_buf((void *)pkey_data.data(), pkey_data.size());
  EVP_PKEY *evp_pkey = PEM_read_bio_PrivateKey(pkey_mbio, nullptr, nullptr,
                                               (void *)password.c_str());
  BIO_free(pkey_mbio);
  if (evp_pkey == nullptr) {
    express_cat.info()
      << "Could not read private key in " << pkey << ".\n";
    return false;
  }

  bool result = add_signature(cert_chain, evp_pkey);

  EVP_PKEY_free(evp_pkey);

  return result;
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Adds a new signature to the Multifile.  This signature associates the
 * indicated certificate with the current contents of the Multifile.  When the
 * Multifile is read later, the signature will still be present only if the
 * Multifile is unchanged; any subsequent changes to the Multifile will
 * automatically invalidate and remove the signature.
 *
 * This flavor of add_signature() reads the certificate, private key, and
 * certificate chain from the same PEM-formatted file.  It takes the first
 * private key found as the intended key, and then uses the first certificate
 * found that matches that key as the signing certificate.  Any other
 * certificates in the file are taken to be part of the chain.
 */
bool Multifile::
add_signature(const Filename &composite, const string &password) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  // First, read the complete file into memory.
  string composite_data;
  if (!vfs->read_file(composite, composite_data, true)) {
    express_cat.info()
      << "Could not read " << composite << ".\n";
    return false;
  }

  // Get the private key.
  BIO *pkey_mbio = BIO_new_mem_buf((void *)composite_data.data(), composite_data.size());
  EVP_PKEY *evp_pkey = PEM_read_bio_PrivateKey(pkey_mbio, nullptr, nullptr,
                                               (void *)password.c_str());
  BIO_free(pkey_mbio);
  if (evp_pkey == nullptr) {
    express_cat.info()
      << "Could not read private key in " << composite << ".\n";
    return false;
  }

  // Now read all of the certificates.
  CertChain cert_chain;

  BIO *chain_mbio = BIO_new_mem_buf((void *)composite_data.data(), composite_data.size());
  X509 *c = PEM_read_bio_X509(chain_mbio, nullptr, nullptr, (void *)"");
  while (c != nullptr) {
    cert_chain.push_back(c);
    c = PEM_read_bio_X509(chain_mbio, nullptr, nullptr, (void *)"");
  }
  BIO_free(chain_mbio);

  if (cert_chain.empty()) {
    express_cat.info()
      << "Could not read certificates in " << composite << ".\n";
    return false;
  }

  // Now find the certificate that matches the signature, and move it to the
  // front of the chain.
  size_t i;
  bool found_match = false;
  for (i = 0; i < cert_chain.size(); ++i) {
    X509 *c = cert_chain[i]._cert;
    if (X509_check_private_key(c, evp_pkey)) {
      found_match = true;
      if (i != 0) {
        // Move this entry to the beginning.
        cert_chain.insert(cert_chain.begin(), cert_chain[i]);
        cert_chain.erase(cert_chain.begin() + i + 1);
      }
      break;
    }
  }

  if (!found_match) {
    express_cat.info()
      << "No certificates in " << composite << " match key.\n";
    return false;
  }

  bool result = add_signature(cert_chain, evp_pkey);

  EVP_PKEY_free(evp_pkey);

  return result;
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Adds a new signature to the Multifile.  This signature associates the
 * indicated certificate with the current contents of the Multifile.  When the
 * Multifile is read later, the signature will still be present only if the
 * Multifile is unchanged; any subsequent changes to the Multifile will
 * automatically invalidate and remove the signature.
 *
 * The signature certificate is the first certificate on the CertChain object.
 * Any remaining certificates are support certificates to authenticate the
 * first one.
 *
 * The specified private key must match the certificate, and the Multifile
 * must be open in read-write mode.  The private key is only used for
 * generating the signature; it is not written to the Multifile and cannot be
 * retrieved from the Multifile later.  (However, the certificate *can* be
 * retrieved from the Multifile later, to identify the entity that created the
 * signature.)
 *
 * This implicitly causes a repack() operation if one is needed.  Returns true
 * on success, false on failure.
 */
bool Multifile::
add_signature(const Multifile::CertChain &cert_chain, EVP_PKEY *pkey) {
  if (_needs_repack) {
    if (!repack()) {
      return false;
    }
  } else {
    if (!flush()) {
      return false;
    }
  }

  if (cert_chain.empty()) {
    express_cat.info()
      << "No certificate given.\n";
    return false;
  }

  if (pkey == nullptr) {
    express_cat.info()
      << "No private key given.\n";
    return false;
  }

  if (!X509_check_private_key(cert_chain[0]._cert, pkey)) {
    express_cat.info()
      << "Private key does not match certificate.\n";
    return false;
  }

  // Now encode that list of certs to a stream in DER form.
  stringstream der_stream;
  StreamWriter der_writer(der_stream);
  der_writer.add_uint32((uint32_t)cert_chain.size());

  CertChain::const_iterator ci;
  for (ci = cert_chain.begin(); ci != cert_chain.end(); ++ci) {
    X509 *cert = (*ci)._cert;

    int der_len = i2d_X509(cert, nullptr);
    unsigned char *der_buf = new unsigned char[der_len];
    unsigned char *p = der_buf;
    i2d_X509(cert, &p);
    der_writer.append_data(der_buf, der_len);
    delete[] der_buf;
  }

  // Create a temporary Subfile for writing out the signature.
  der_stream.seekg(0);
  Subfile *subfile = new Subfile;
  subfile->_pkey = pkey;
  subfile->_flags |= SF_signature;
  subfile->_source = &der_stream;

  // Write the new Subfile at the end.  The cert_special subfiles always go at
  // the end, because they're not the part of the file that's signed.
  nassertr(_new_subfiles.empty(), false);
  _new_subfiles.push_back(subfile);
  bool result = flush();

  delete subfile;

  return result;
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Returns the number of matching signatures found on the Multifile.  These
 * signatures may be iterated via get_signature() and related methods.
 *
 * A signature on this list is guaranteed to match the Multifile contents,
 * proving that the Multifile has been unmodified since the signature was
 * applied.  However, this does not guarantee that the certificate itself is
 * actually from who it says it is from; only that it matches the Multifile
 * contents.  See validate_signature_certificate() to authenticate a
 * particular certificate.
 */
int Multifile::
get_num_signatures() const {
  ((Multifile *)this)->check_signatures();
  return _signatures.size();
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Returns the nth signature found on the Multifile.  See the comments in
 * get_num_signatures().
 */
const Multifile::CertChain &Multifile::
get_signature(int n) const {
  ((Multifile *)this)->check_signatures();
  static CertChain error_chain;
  nassertr(n >= 0 && n < (int)_signatures.size(), error_chain);
  return _signatures[n];
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Returns the "subject name" for the nth signature found on the Multifile.
 * This is a string formatted according to RFC2253 that should more-or-less
 * identify a particular certificate; when paired with the public key (see
 * get_signature_public_key()), it can uniquely identify a certificate.  See
 * the comments in get_num_signatures().
 */
string Multifile::
get_signature_subject_name(int n) const {
  const CertChain &cert_chain = get_signature(n);
  nassertr(!cert_chain.empty(), string());

  X509_NAME *xname = X509_get_subject_name(cert_chain[0]._cert);
  if (xname != nullptr) {
    // We use "print" to dump the output to a memory BIO.  Is there an easier
    // way to extract the X509_NAME text?  Curse these incomplete docs.
    BIO *mbio = BIO_new(BIO_s_mem());
    X509_NAME_print_ex(mbio, xname, 0, XN_FLAG_RFC2253);

    char *pp;
    long pp_size = BIO_get_mem_data(mbio, &pp);
    string name(pp, pp_size);
    BIO_free(mbio);
    return name;
  }

  return string();
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Returns a "friendly name" for the nth signature found on the Multifile.
 * This attempts to extract out the most meaningful part of the subject name.
 * It returns the emailAddress, if it is defined; otherwise, it returns the
 * commonName.
 *
 * See the comments in get_num_signatures().
 */
string Multifile::
get_signature_friendly_name(int n) const {
  const CertChain &cert_chain = get_signature(n);
  nassertr(!cert_chain.empty(), string());

  static const int nid_choices[] = {
    NID_pkcs9_emailAddress,
    NID_subject_alt_name,
    NID_commonName,
    -1,
  };

  // Choose the first NID that exists on the cert.
  for (int ni = 0; nid_choices[ni] != -1; ++ni) {
    int nid = nid_choices[ni];

    // A complex OpenSSL interface to extract out the name in utf-8.
    X509_NAME *xname = X509_get_subject_name(cert_chain[0]._cert);
    if (xname != nullptr) {
      int pos = X509_NAME_get_index_by_NID(xname, nid, -1);
      if (pos != -1) {
        // We just get the first common name.  I guess it's possible to have
        // more than one; not sure what that means in this context.
        X509_NAME_ENTRY *xentry = X509_NAME_get_entry(xname, pos);
        if (xentry != nullptr) {
          ASN1_STRING *data = X509_NAME_ENTRY_get_data(xentry);
          if (data != nullptr) {
            // We use "print" to dump the output to a memory BIO.  Is there an
            // easier way to decode the ASN1_STRING?  Curse these incomplete
            // docs.
            BIO *mbio = BIO_new(BIO_s_mem());
            ASN1_STRING_print_ex(mbio, data, ASN1_STRFLGS_RFC2253 & ~ASN1_STRFLGS_ESC_MSB);

            char *pp;
            long pp_size = BIO_get_mem_data(mbio, &pp);
            string name(pp, pp_size);
            BIO_free(mbio);
            return name;
          }
        }
      }
    }
  }

  return string();
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Returns the public key used for the nth signature found on the Multifile.
 * This is encoded in DER form and returned as a string of hex digits.
 *
 * This can be used, in conjunction with the subject name (see
 * get_signature_subject_name()), to uniquely identify a particular
 * certificate and its subsequent reissues.  See the comments in
 * get_num_signatures().
 */
string Multifile::
get_signature_public_key(int n) const {
  const CertChain &cert_chain = get_signature(n);
  nassertr(!cert_chain.empty(), string());

  EVP_PKEY *pkey = X509_get_pubkey(cert_chain[0]._cert);
  if (pkey != nullptr) {
    int key_len = i2d_PublicKey(pkey, nullptr);
    unsigned char *key_buf = new unsigned char[key_len];
    unsigned char *p = key_buf;
    i2d_PublicKey(pkey, &p);
    string result;
    for (int i = 0; i < key_len; ++i) {
      result += tohex(key_buf[i] >> 4);
      result += tohex(key_buf[i]);
    }
    delete[] key_buf;
    return result;
  }

  return string();
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Writes the certificate for the nth signature, in user-readable verbose
 * form, to the indicated stream.  See the comments in get_num_signatures().
 */
void Multifile::
print_signature_certificate(int n, ostream &out) const {
  const CertChain &cert_chain = get_signature(n);
  nassertv(!cert_chain.empty());

  BIO *mbio = BIO_new(BIO_s_mem());
  X509_print(mbio, cert_chain[0]._cert);

  char *pp;
  long pp_size = BIO_get_mem_data(mbio, &pp);
  out.write(pp, pp_size);
  BIO_free(mbio);
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Writes the certificate for the nth signature, in PEM form, to the indicated
 * stream.  See the comments in get_num_signatures().
 */
void Multifile::
write_signature_certificate(int n, ostream &out) const {
  const CertChain &cert_chain = get_signature(n);
  nassertv(!cert_chain.empty());

  BIO *mbio = BIO_new(BIO_s_mem());

  CertChain::const_iterator ci;
  for (ci = cert_chain.begin(); ci != cert_chain.end(); ++ci) {
    X509 *c = (*ci)._cert;
    X509_print(mbio, c);
    PEM_write_bio_X509(mbio, c);
  }

  char *pp;
  long pp_size = BIO_get_mem_data(mbio, &pp);
  out.write(pp, pp_size);
  BIO_free(mbio);
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Checks that the certificate used for the nth signature is a valid,
 * authorized certificate with some known certificate authority.  Returns 0 if
 * it is valid, -1 if there is some error, or the corresponding OpenSSL error
 * code if it is invalid, out-of-date, or self-signed.
 */
int Multifile::
validate_signature_certificate(int n) const {
  int verify_result = -1;

  const CertChain &chain = get_signature(n);
  nassertr(!chain.empty(), false);

  OpenSSLWrapper *sslw = OpenSSLWrapper::get_global_ptr();

  // Copy our CertChain structure into an X509 pointer and accompanying
  // STACK_OF(X509) pointer.
  X509 *x509 = chain[0]._cert;
  STACK_OF(X509) *stack = nullptr;
  if (chain.size() > 1) {
    stack = sk_X509_new(nullptr);
    for (size_t n = 1; n < chain.size(); ++n) {
      sk_X509_push(stack, chain[n]._cert);
    }
  }

  // Create the X509_STORE_CTX for verifying the cert and chain.
  X509_STORE_CTX *ctx = X509_STORE_CTX_new();
  X509_STORE_CTX_init(ctx, sslw->get_x509_store(), x509, stack);
  X509_STORE_CTX_set_cert(ctx, x509);

  if (X509_verify_cert(ctx)) {
    verify_result = 0;
  } else {
    verify_result = X509_STORE_CTX_get_error(ctx);
  }

  if (express_cat.is_debug()) {
    express_cat.debug()
      << get_signature_subject_name(n) << ": validate " << verify_result
      << "\n";
  }

  sk_X509_free(stack);
  X509_STORE_CTX_cleanup(ctx);
  X509_STORE_CTX_free(ctx);

  return verify_result;
}
#endif // HAVE_OPENSSL

/**
 * Writes all contents of the Multifile to disk.  Until flush() is called,
 * add_subfile() and remove_subfile() do not actually do anything to disk.  At
 * this point, all of the recently-added subfiles are read and their contents
 * are added to the end of the Multifile, and the recently-removed subfiles
 * are marked gone from the Multifile.
 *
 * This may result in a suboptimal index.  To guarantee that the index is
 * written at the beginning of the file, call repack() instead of flush().
 *
 * It is not necessary to call flush() explicitly unless you are concerned
 * about reading the recently-added subfiles immediately.
 *
 * Returns true on success, false on failure.
 */
bool Multifile::
flush() {
  if (!is_write_valid()) {
    return false;
  }

  bool new_file = (_next_index == (streampos)0);
  if (new_file) {
    // If we don't have an index yet, we don't have a header.  Write the
    // header.
    if (!write_header()) {
      return false;
    }

  } else {
    if (_file_minor_ver != _current_minor_ver) {
      // If we *do* have an index already, but this is an old version
      // multifile, we have to completely rewrite it anyway.
      return repack();
    }
  }

  nassertr(_write != nullptr, false);

  // First, mark out all of the removed subfiles.
  PendingSubfiles::iterator pi;
  for (pi = _removed_subfiles.begin(); pi != _removed_subfiles.end(); ++pi) {
    Subfile *subfile = (*pi);
    subfile->rewrite_index_flags(*_write);
    delete subfile;
  }
  _removed_subfiles.clear();

  bool wrote_ok = true;

  if (!_new_subfiles.empty() || new_file) {
    // Add a few more files to the end.  We always add subfiles at the end of
    // the multifile, so go there first.
    sort(_new_subfiles.begin(), _new_subfiles.end(), IndirectLess<Subfile>());
    if (_last_index != (streampos)0) {
      _write->seekp(0, ios::end);
      if (_write->fail()) {
        express_cat.info()
          << "Unable to seek Multifile " << _multifile_name << ".\n";
        return false;
      }
      _next_index = _write->tellp();
      _next_index = pad_to_streampos(_next_index);

      // And update the forward link from the last_index to point to this new
      // index location.
      _write->seekp(_last_index);
      StreamWriter writer(_write, false);
      writer.add_uint32(streampos_to_word(_next_index));
    }

    _write->seekp(_next_index);
    nassertr(_next_index == _write->tellp(), false);

    // Ok, here we are at the end of the file.  Write out the recently-added
    // subfiles here.  First, count up the index size.
    for (pi = _new_subfiles.begin(); pi != _new_subfiles.end(); ++pi) {
      Subfile *subfile = (*pi);
      _last_index = _next_index;
      _next_index = subfile->write_index(*_write, _next_index, this);
      nassertr(_next_index == _write->tellp(), false);
      _next_index = pad_to_streampos(_next_index);
      nassertr(_next_index == _write->tellp(), false);
    }

    // Now we're at the end of the index.  Write a 0 here to mark the end.
    StreamWriter writer(_write, false);
    writer.add_uint32(0);
    _next_index += 4;
    nassertr(_next_index == _write->tellp(), false);
    _next_index = pad_to_streampos(_next_index);

    // All right, now write out each subfile's data.
    for (pi = _new_subfiles.begin(); pi != _new_subfiles.end(); ++pi) {
      Subfile *subfile = (*pi);

      if (_read != nullptr) {
        _read->acquire();
        _next_index = subfile->write_data(*_write, _read->get_istream(),
                                          _next_index, this);
        _read->release();

      } else {
        _next_index = subfile->write_data(*_write, nullptr, _next_index, this);
      }

      nassertr(_next_index == _write->tellp(), false);
      _next_index = pad_to_streampos(_next_index);
      if (subfile->is_data_invalid()) {
        wrote_ok = false;
      }

      if (!subfile->is_cert_special()) {
        _last_data_byte = max(_last_data_byte, subfile->get_last_byte_pos());
      }
      nassertr(_next_index == _write->tellp(), false);
    }

    // Now go back and fill in the proper addresses for the data start.  We
    // didn't do it in the first pass, because we don't really want to keep
    // all those file handles open, and so we didn't have to determine each
    // file's length ahead of time.
    for (pi = _new_subfiles.begin(); pi != _new_subfiles.end(); ++pi) {
      Subfile *subfile = (*pi);
      subfile->rewrite_index_data_start(*_write, this);
    }

    _new_subfiles.clear();
  }

  // Also update the overall timestamp.
  if (_timestamp_dirty) {
    nassertr(!_write->fail(), false);
    static const size_t timestamp_pos = _header_prefix.size() + _header_size + 2 + 2 + 4;
    _write->seekp(timestamp_pos);
    nassertr(!_write->fail(), false);

    StreamWriter writer(*_write);
    if (_record_timestamp) {
      writer.add_uint32(_timestamp);
    } else {
      writer.add_uint32(0);
    }
    _timestamp_dirty = false;
  }

  _write->flush();
  if (!wrote_ok || _write->fail()) {
    express_cat.info()
      << "Unable to update Multifile " << _multifile_name << ".\n";
    close();
    return false;
  }

  return true;
}

/**
 * Forces a complete rewrite of the Multifile and all of its contents, so that
 * its index will appear at the beginning of the file with all of the subfiles
 * listed in alphabetical order.  This is considered optimal for reading, and
 * is the standard configuration; but it is not essential to do this.
 *
 * It is only valid to call this if the Multifile was opened using
 * open_read_write() and an explicit filename, rather than an iostream.  Also,
 * we must have write permission to the directory containing the Multifile.
 *
 * Returns true on success, false on failure.
 */
bool Multifile::
repack() {
  if (_next_index == (streampos)0) {
    // If the Multifile hasn't yet been written, this is really just a flush
    // operation.
    _needs_repack = false;
    return flush();
  }

  nassertr(is_write_valid() && is_read_valid(), false);
  nassertr(!_multifile_name.empty(), false);

  // First, we open a temporary filename to copy the Multifile to.
  Filename dirname = _multifile_name.get_dirname();
  if (dirname.empty()) {
    dirname = ".";
  }
  Filename temp_filename = Filename::temporary(dirname, "mftemp");
  temp_filename.set_binary();
  pofstream temp;
  if (!temp_filename.open_write(temp)) {
    express_cat.info()
      << "Unable to open temporary file " << temp_filename << "\n";
    return false;
  }

  // Now we scrub our internal structures so it looks like we're a brand new
  // Multifile.
  PendingSubfiles::iterator pi;
  for (pi = _removed_subfiles.begin(); pi != _removed_subfiles.end(); ++pi) {
    Subfile *subfile = (*pi);
    delete subfile;
  }
  _removed_subfiles.clear();
  _new_subfiles.clear();
  std::copy(_subfiles.begin(), _subfiles.end(), std::back_inserter(_new_subfiles));
  _next_index = 0;
  _last_index = 0;
  _last_data_byte = 0;
  _scale_factor = _new_scale_factor;

  // And we write our contents to our new temporary file.
  _write = &temp;
  if (!flush()) {
    temp.close();
    temp_filename.unlink();
    return false;
  }

  // Now close everything, and move the temporary file back over our original
  // file.
  Filename orig_name = _multifile_name;
  temp.close();
  close();
  orig_name.unlink();
  if (!temp_filename.rename_to(orig_name)) {
    express_cat.info()
      << "Unable to rename temporary file " << temp_filename << " to "
      << orig_name << ".\n";
    return false;
  }

  if (!open_read_write(orig_name)) {
    express_cat.info()
      << "Unable to read newly repacked " << _multifile_name
      << ".\n";
    return false;
  }

  return true;
}

/**
 * Returns the number of subfiles within the Multifile.  The subfiles may be
 * accessed in alphabetical order by iterating through [0 ..
 * get_num_subfiles()).
 */
int Multifile::
get_num_subfiles() const {
  return _subfiles.size();
}

/**
 * Returns the index of the subfile with the indicated name, or -1 if the
 * named subfile is not within the Multifile.
 */
int Multifile::
find_subfile(const string &subfile_name) const {
  Subfile find_subfile;
  find_subfile._name = standardize_subfile_name(subfile_name);
  Subfiles::const_iterator fi;
  fi = _subfiles.find(&find_subfile);
  if (fi == _subfiles.end()) {
    // Not present.
    return -1;
  }
  return (fi - _subfiles.begin());
}

/**
 * Returns true if the indicated subfile name is the directory prefix to one
 * or more files within the Multifile.  That is, the Multifile contains at
 * least one file named "subfile_name/...".
 */
bool Multifile::
has_directory(const string &subfile_name) const {
  string prefix = subfile_name;
  if (!prefix.empty()) {
    prefix += '/';
  }
  Subfile find_subfile;
  find_subfile._name = prefix;
  Subfiles::const_iterator fi;
  fi = _subfiles.upper_bound(&find_subfile);
  if (fi == _subfiles.end()) {
    // Not present.
    return false;
  }

  // At least one subfile exists whose name sorts after prefix.  If it
  // contains prefix as the initial substring, then we have a match.
  Subfile *subfile = (*fi);
  return (subfile->_name.length() > prefix.length() &&
          subfile->_name.substr(0, prefix.length()) == prefix);
}

/**
 * Considers subfile_name to be the name of a subdirectory within the
 * Multifile, but not a file itself; fills the given vector up with the sorted
 * list of subdirectories or files within the named directory.
 *
 * Note that directories do not exist explicitly within a Multifile; this just
 * checks for the existence of files with the given initial prefix.
 *
 * Returns true if successful, false otherwise.
 */
bool Multifile::
scan_directory(vector_string &contents, const string &subfile_name) const {
  string prefix = subfile_name;
  if (!prefix.empty()) {
    prefix += '/';
  }
  Subfile find_subfile;
  find_subfile._name = prefix;
  Subfiles::const_iterator fi;
  fi = _subfiles.upper_bound(&find_subfile);

  string previous = "";
  while (fi != _subfiles.end()) {
    Subfile *subfile = (*fi);
    if (!(subfile->_name.length() > prefix.length() &&
          subfile->_name.substr(0, prefix.length()) == prefix)) {
      // We've reached the end of the list of subfiles beneath the indicated
      // directory prefix.
      return true;
    }

    size_t slash = subfile->_name.find('/', prefix.length());
    string basename = subfile->_name.substr(prefix.length(), slash - prefix.length());
    if (basename != previous) {
      contents.push_back(basename);
      previous = basename;
    }
    ++fi;
  }

  return true;
}

/**
 * Removes the nth subfile from the Multifile.  This will cause all subsequent
 * index numbers to decrease by one.  The file will not actually be removed
 * from the disk until the next call to flush().
 *
 * Note that this does not actually remove the data from the indicated
 * subfile; it simply removes it from the index.  The Multifile will not be
 * reduced in size after this operation, until the next call to repack().
 */
void Multifile::
remove_subfile(int index) {
  nassertv(is_write_valid());
  nassertv(index >= 0 && index < (int)_subfiles.size());
  Subfile *subfile = _subfiles[index];
  subfile->_flags |= SF_deleted;
  _removed_subfiles.push_back(subfile);
  _subfiles.erase(_subfiles.begin() + index);

  _timestamp = time(nullptr);
  _timestamp_dirty = true;

  _needs_repack = true;
}

/**
 * Returns the name of the nth subfile.
 */
const string &Multifile::
get_subfile_name(int index) const {
#ifndef NDEBUG
  static string empty_string;
  nassertr(index >= 0 && index < (int)_subfiles.size(), empty_string);
#endif
  return _subfiles[index]->_name;
}

/**
 * Returns the uncompressed data length of the nth subfile.  This might return
 * 0 if the subfile has recently been added and flush() has not yet been
 * called.
 */
size_t Multifile::
get_subfile_length(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  return _subfiles[index]->_uncompressed_length;
}

/**
 * Returns the modification time of the nth subfile.  If this is called on an
 * older .mf file, which did not store individual timestamps in the file (or
 * if get_record_timestamp() is false), this will return the modification time
 * of the overall multifile.
 */
time_t Multifile::
get_subfile_timestamp(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  if (!get_record_timestamp()) {
    return get_timestamp();
  } else {
    return _subfiles[index]->_timestamp;
  }
}

/**
 * Returns true if the indicated subfile has been compressed when stored
 * within the archive, false otherwise.
 */
bool Multifile::
is_subfile_compressed(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  return (_subfiles[index]->_flags & SF_compressed) != 0;
}

/**
 * Returns true if the indicated subfile has been encrypted when stored within
 * the archive, false otherwise.
 */
bool Multifile::
is_subfile_encrypted(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  return (_subfiles[index]->_flags & SF_encrypted) != 0;
}

/**
 * Returns true if the indicated subfile represents text data, or false if it
 * represents binary data.  If the file is text data, it may have been
 * processed by end-of-line conversion when it was added.  (But the actual
 * bits in the multifile will represent the standard Unix end-of-line
 * convention, e.g.  \n instead of \r\n.)
 */
bool Multifile::
is_subfile_text(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  return (_subfiles[index]->_flags & SF_text) != 0;
}

/**
 * Returns the first byte that is guaranteed to follow any index byte already
 * written to disk in the Multifile.
 *
 * This number is largely meaningless in many cases, but if needs_repack() is
 * false, and the file is flushed, this will indicate the number of bytes in
 * the header + index.  Everything at this byte position and later will be
 * actual data.
 */
streampos Multifile::
get_index_end() const {
  return normalize_streampos(_next_index + (streampos)4);
}

/**
 * Returns the starting byte position within the Multifile at which the
 * indicated subfile begins.  This may be used, with
 * get_subfile_internal_length(), for low-level access to the subfile, but
 * usually it is better to use open_read_subfile() instead (which
 * automatically decrypts and/or uncompresses the subfile data).
 */
streampos Multifile::
get_subfile_internal_start(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  return _subfiles[index]->_data_start;
}

/**
 * Returns the number of bytes the indicated subfile consumes within the
 * archive.  For compressed subfiles, this will generally be smaller than
 * get_subfile_length(); for encrypted (but noncompressed) subfiles, it may be
 * slightly different, for noncompressed and nonencrypted subfiles, it will be
 * equal.
 */
size_t Multifile::
get_subfile_internal_length(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  return _subfiles[index]->_data_length;
}

/**
 * Returns an istream that may be used to read the indicated subfile.  You may
 * seek() within this istream to your heart's content; even though it will be
 * a reference to the already-opened pfstream of the Multifile itself, byte 0
 * appears to be the beginning of the subfile and EOF appears to be the end of
 * the subfile.
 *
 * The returned istream will have been allocated via new; you should pass the
 * pointer to close_read_subfile() when you are finished with it to delete it
 * and release its resources.
 *
 * Any future calls to repack() or close() (or the Multifile destructor) will
 * invalidate all currently open subfile pointers.
 *
 * The return value will be NULL if the stream cannot be opened for some
 * reason.
 */
istream *Multifile::
open_read_subfile(int index) {
  nassertr(is_read_valid(), nullptr);
  nassertr(index >= 0 && index < (int)_subfiles.size(), nullptr);
  Subfile *subfile = _subfiles[index];

  if (subfile->_source != nullptr ||
      !subfile->_source_filename.empty()) {
    // The subfile has not yet been copied into the physical Multifile.  Force
    // a flush operation to incorporate it.
    flush();

    // That shouldn't change the subfile index or delete the subfile pointer.
    nassertr(subfile == _subfiles[index], nullptr);
  }

  return open_read_subfile(subfile);
}

/**
 * Closes a file opened by a previous call to open_read_subfile().  This
 * really just deletes the istream pointer, but it is recommended to use this
 * interface instead of deleting it explicitly, to help work around compiler
 * issues.
 */
void Multifile::
close_read_subfile(istream *stream) {
  if (stream != nullptr) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting the
    // stream pointer does not call the appropriate global delete function;
    // instead apparently calling the system delete function.  So we call the
    // delete function by hand instead.
#if !defined(WIN32_VC) && !defined(USE_MEMORY_NOWRAPPERS) && defined(REDEFINE_GLOBAL_OPERATOR_NEW)
    stream->~istream();
    (*global_operator_delete)(stream);
#else
    delete stream;
#endif
  }
}

/**
 * Extracts the nth subfile into a file with the given name.
 */
bool Multifile::
extract_subfile(int index, const Filename &filename) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);

  Filename fname = filename;
  if (multifile_always_binary) {
    fname.set_binary();
  }

  if (!fname.is_binary_or_text()) {
    // If we haven't specified binary or text, infer it from the type of the
    // subfile.
    if ((_subfiles[index]->_flags & SF_text) != 0) {
      fname.set_text();
    } else {
      fname.set_binary();
    }
  }
  fname.make_dir();
  pofstream out;
  if (!fname.open_write(out, true)) {
    express_cat.info()
      << "Unable to write to file " << filename << "\n";
    return false;
  }

  return extract_subfile_to(index, out);
}

/**
 * Extracts the nth subfile to the indicated ostream.
 */
bool Multifile::
extract_subfile_to(int index, ostream &out) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);

  istream *in = open_read_subfile(index);
  if (in == nullptr) {
    return false;
  }

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  in->read(buffer, buffer_size);
  size_t count = in->gcount();
  while (count != 0) {
    out.write(buffer, count);
    in->read(buffer, buffer_size);
    count = in->gcount();
  }

  bool failed = (in->fail() && !in->eof());
  close_read_subfile(in);
  nassertr(!failed, false);

  return (!out.fail());
}

/**
 * Performs a byte-for-byte comparison of the indicated file on disk with the
 * nth subfile.  Returns true if the files are equivalent, or false if they
 * are different (or the file is missing).
 *
 * If Filename::set_binary() or set_text() has already been called, it
 * specifies the nature of the source file.  If this is different from the
 * text flag of the subfile, the comparison will always return false.  If this
 * has not been specified, it will be set from the text flag of the subfile.
 */
bool Multifile::
compare_subfile(int index, const Filename &filename) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);

  if (!filename.exists()) {
    express_cat.info()
      << "File is missing: " << filename << "\n";
    return false;
  }

  Filename fname = filename;
  if (fname.is_binary()) {
    // If we've specified a binary file, it had better be a binary subfile.
    if ((_subfiles[index]->_flags & SF_text) != 0) {
      if (express_cat.is_debug()) {
        express_cat.debug()
          << "File is not binary: " << filename << "\n";
      }
      return false;
    }

  } else if (fname.is_text()) {
    // If we've specified a text file, it had better be a text subfile.
    if ((_subfiles[index]->_flags & SF_text) == 0) {
      if (express_cat.is_debug()) {
        express_cat.debug()
          << "File is not text: " << filename << "\n";
      }
      return false;
    }

  } else {
    // If we haven't specified binary or text, infer it from the type of the
    // subfile.
    if ((_subfiles[index]->_flags & SF_text) != 0) {
      fname.set_text();
    } else {
      fname.set_binary();
    }
  }

  istream *in1 = open_read_subfile(index);
  if (in1 == nullptr) {
    return false;
  }

  pifstream in2;

  if (!fname.open_read(in2)) {
    express_cat.info()
      << "Cannot read " << filename << "\n";
    return false;
  }

  if (fname.is_binary()) {
    // Check the file size.
    in2.seekg(0, ios::end);
    streampos file_size = in2.tellg();

    if (file_size != (streampos)get_subfile_length(index)) {
      // The files have different sizes.
      close_read_subfile(in1);
      return false;
    }
  }

  // Check the file data, byte-for-byte.
  in2.seekg(0);
  int byte1 = in1->get();
  int byte2 = in2.get();
  while (!in1->fail() && !in2.fail()) {
    if (byte1 != byte2) {
      close_read_subfile(in1);
      return false;
    }
    byte1 = in1->get();
    byte2 = in2.get();
  }

  bool failed = (in1->fail() && !in1->eof()) || (in2.fail() && !in2.eof());
  close_read_subfile(in1);

  nassertr(!failed, false);

  return true;
}

/**
 *
 */
void Multifile::
output(ostream &out) const {
  out << "Multifile " << _multifile_name << ", " << get_num_subfiles()
      << " subfiles.\n";
}

/**
 * Shows a list of all subfiles within the Multifile.
 */
void Multifile::
ls(ostream &out) const {
  int num_subfiles = get_num_subfiles();
  for (int i = 0; i < num_subfiles; i++) {
    string subfile_name = get_subfile_name(i);
    out << subfile_name << "\n";
  }
}

/**
 * Sets the string which is written to the Multifile before the Multifile
 * header.  This string must begin with a hash mark and end with a newline
 * character; and if it includes embedded newline characters, each one must be
 * followed by a hash mark.  If these conditions are not initially true, the
 * string will be modified as necessary to make it so.
 *
 * This is primarily useful as a simple hack to allow p3d applications to be
 * run directly from the command line on Unix-like systems.
 *
 * The return value is true if successful, or false on failure (for instance,
 * because the header prefix violates the above rules).
 */
void Multifile::
set_header_prefix(const string &header_prefix) {
  string new_header_prefix = header_prefix;

  if (!new_header_prefix.empty()) {
    // It must begin with a hash mark.
    if (new_header_prefix[0] != '#') {
      new_header_prefix = string("#") + new_header_prefix;
    }

    // It must end with a newline.
    if (new_header_prefix[new_header_prefix.size() - 1] != '\n') {
      new_header_prefix += string("\n");
    }

    // Embedded newlines must be followed by a hash mark.
    size_t newline = new_header_prefix.find('\n');
    while (newline < new_header_prefix.size() - 1) {
      if (new_header_prefix[newline + 1] != '#') {
        new_header_prefix = new_header_prefix.substr(0, newline + 1) + string("#") + new_header_prefix.substr(newline + 1);
      }
      newline = new_header_prefix.find('#', newline);
    }
  }

  if (_header_prefix != new_header_prefix) {
    _header_prefix = new_header_prefix;
    _needs_repack = true;
  }
}


/**
 * Fills a string with the entire contents of the indicated subfile.
 */
bool Multifile::
read_subfile(int index, string &result) {
  result = string();

  // We use a temporary pvector, because dynamic accumulation of a pvector
  // seems to be many times faster than that of a string, at least on the
  // Windows implementation of STL.
  vector_uchar pv;
  if (!read_subfile(index, pv)) {
    return false;
  }

  if (!pv.empty()) {
    result.append((const char *)&pv[0], pv.size());
  }

  return true;
}

/**
 * Fills a pvector with the entire contents of the indicated subfile.
 */
bool Multifile::
read_subfile(int index, vector_uchar &result) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  result.clear();

  // Now look up the particular Subfile we are reading.
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  Subfile *subfile = _subfiles[index];

  if (subfile->_source != nullptr ||
      !subfile->_source_filename.empty()) {
    // The subfile has not yet been copied into the physical Multifile.  Force
    // a flush operation to incorporate it.
    flush();

    // That shouldn't change the subfile index or delete the subfile pointer.
    nassertr(subfile == _subfiles[index], false);
  }

  result.reserve(subfile->_uncompressed_length);

  bool success = true;
  if (subfile->_flags & (SF_encrypted | SF_compressed)) {
    // If the subfile is encrypted or compressed, we can't read it directly.
    // Fall back to the generic implementation.
    istream *in = open_read_subfile(index);
    if (in == nullptr) {
      return false;
    }

    success = VirtualFile::simple_read_file(in, result);
    close_read_subfile(in);

  } else {
    // But if the subfile is just a plain file, we can just read the data
    // directly from the Multifile, without paying the cost of an ISubStream.
    static const size_t buffer_size = 4096;
    char buffer[buffer_size];

    streamsize pos = _offset + subfile->_data_start;
    size_t max_bytes = subfile->_data_length;
    streamsize count = 0;
    bool eof = true;

    streamsize num_bytes = (streamsize)min(buffer_size, max_bytes);
    _read->seek_read(pos, buffer, num_bytes, count, eof);
    while (count != 0) {
      thread_consider_yield();
      nassertr(count <= (streamsize)max_bytes, false);
      result.insert(result.end(), buffer, buffer + (size_t)count);
      max_bytes -= (size_t)count;
      pos += count;

      num_bytes = (streamsize)min(buffer_size, max_bytes);
      _read->seek_read(pos, buffer, num_bytes, count, eof);
    }

    success = !eof;
  }

  if (!success) {
    ostringstream message;
    message << "I/O error reading from " << get_multifile_name() << " at "
            << get_subfile_name(index);
    nassert_raise(message.str());
    return false;
  }

  return true;
}

/**
 * Assumes the _write pointer is at the indicated fpos, rounds the fpos up to
 * the next legitimate address (using normalize_streampos()), and writes
 * enough zeroes to the stream to fill the gap.  Returns the new fpos.
 */
streampos Multifile::
pad_to_streampos(streampos fpos) {
  nassertr(_write != nullptr, fpos);
  nassertr(_write->tellp() == fpos, fpos);
  streampos new_fpos = normalize_streampos(fpos);
  while (fpos < new_fpos) {
    _write->put(0);
    fpos += 1; // VC++ doesn't define streampos++ (!)
  }
  nassertr(_write->tellp() == fpos, fpos);
  return fpos;
}

/**
 * Adds a newly-allocated Subfile pointer to the Multifile.
 */
void Multifile::
add_new_subfile(Subfile *subfile, int compression_level) {
  if (compression_level != 0) {
#ifndef HAVE_ZLIB
    express_cat.warning()
      << "zlib not compiled in; cannot generated compressed multifiles.\n";
    compression_level = 0;
#else  // HAVE_ZLIB
    subfile->_flags |= SF_compressed;
    subfile->_compression_level = compression_level;
#endif  // HAVE_ZLIB
  }

#ifdef HAVE_OPENSSL
  if (_encryption_flag) {
    subfile->_flags |= SF_encrypted;
  }
#endif  // HAVE_OPENSSL

  if (_next_index != (streampos)0) {
    // If we're adding a Subfile to an already-existing Multifile, we will
    // eventually need to repack the file.
    _needs_repack = true;
  }

  std::pair<Subfiles::iterator, bool> insert_result = _subfiles.insert(subfile);
  if (!insert_result.second) {
    // Hmm, unable to insert.  There must already be a subfile by that name.
    // Remove the old one.
    Subfile *old_subfile = (*insert_result.first);
    old_subfile->_flags |= SF_deleted;

    // Maybe it was just added to the _new_subfiles list.  In this case,
    // remove it from that list.
    PendingSubfiles::iterator ni = find(_new_subfiles.begin(), _new_subfiles.end(), old_subfile);
    if (ni != _new_subfiles.end()) {
      _new_subfiles.erase(ni);

    } else {
      // Otherwise, add it to the _removed_subfiles list, so we can remove the
      // old one.
      _removed_subfiles.push_back(old_subfile);
    }

    (*insert_result.first) = subfile;
  }

  _new_subfiles.push_back(subfile);
}

/**
 * This variant of open_read_subfile() is used internally only, and accepts a
 * pointer to the internal Subfile object, which is assumed to be valid and
 * written to the multifile.
 */
istream *Multifile::
open_read_subfile(Subfile *subfile) {
  nassertr(subfile->_source == nullptr &&
           subfile->_source_filename.empty(), nullptr);

  // Return an ISubStream object that references into the open Multifile
  // istream.
  nassertr(subfile->_data_start != (streampos)0, nullptr);
  istream *stream =
    new ISubStream(_read, _offset + subfile->_data_start,
                   _offset + subfile->_data_start + (streampos)subfile->_data_length);

  if ((subfile->_flags & SF_encrypted) != 0) {
#ifndef HAVE_OPENSSL
    express_cat.error()
      << "OpenSSL not compiled in; cannot read encrypted multifiles.\n";
    delete stream;
    return nullptr;
#else  // HAVE_OPENSSL
    // The subfile is encrypted.  So actually, return an IDecryptStream that
    // wraps around the ISubStream.
    IDecryptStream *wrapper =
      new IDecryptStream(stream, true, _encryption_password);
    stream = wrapper;

    // Validate the password by confirming that the encryption header matches.
    if (!wrapper->read_magic(_encrypt_header, _encrypt_header_size)) {
      express_cat.error()
        << "Unable to decrypt subfile " << subfile->_name << ".\n";
      delete stream;
      return nullptr;
    }
#endif  // HAVE_OPENSSL
  }

  if ((subfile->_flags & SF_compressed) != 0) {
#ifndef HAVE_ZLIB
    express_cat.error()
      << "zlib not compiled in; cannot read compressed multifiles.\n";
    delete stream;
    return nullptr;
#else  // HAVE_ZLIB
    // Oops, the subfile is compressed.  So actually, return an
    // IDecompressStream that wraps around the ISubStream.
    IDecompressStream *wrapper = new IDecompressStream(stream, true);
    stream = wrapper;
#endif  // HAVE_ZLIB
  }

  if (stream->fail()) {
    // Hmm, some inexplicable problem.
    delete stream;
    return nullptr;
  }

  return stream;
}

/**
 * Returns the standard form of the subfile name.
 */
string Multifile::
standardize_subfile_name(const string &subfile_name) const {
  Filename name = subfile_name;
  name.standardize();
  if (name.empty() || name == "/") {
    // Invalid empty name.
    return string();
  }

  if (name[0] == '/') {
    return name.get_fullpath().substr(1);
  } else if (name.length() > 2 && name[0] == '.' && name[1] == '/') {
    return name.get_fullpath().substr(2);
  } else {
    return name.get_fullpath();
  }
}

/**
 * Removes the set of subfiles from the tables and frees their associated
 * memory.
 */
void Multifile::
clear_subfiles() {
  PendingSubfiles::iterator pi;
  for (pi = _removed_subfiles.begin(); pi != _removed_subfiles.end(); ++pi) {
    Subfile *subfile = (*pi);
    subfile->rewrite_index_flags(*_write);
    delete subfile;
  }
  _removed_subfiles.clear();

  // We don't have to delete the ones in _new_subfiles, because these also
  // appear in _subfiles.
  _new_subfiles.clear();

#ifdef HAVE_OPENSSL
  for (pi = _cert_special.begin(); pi != _cert_special.end(); ++pi) {
    Subfile *subfile = (*pi);
    delete subfile;
  }
  _cert_special.clear();

  _signatures.clear();
#endif  // HAVE_OPENSSL

  Subfiles::iterator fi;
  for (fi = _subfiles.begin(); fi != _subfiles.end(); ++fi) {
    Subfile *subfile = (*fi);
    delete subfile;
  }
  _subfiles.clear();
}

/**
 * Reads the Multifile header and index.  Returns true if successful, false if
 * the Multifile is not valid.
 */
bool Multifile::
read_index() {
  nassertr(_read != nullptr, false);

  // We acquire the IStreamWrapper lock for the duration of this method.
  _read->acquire();
  istream *read = _read->get_istream();

  char this_header[_header_size];
  read->seekg(_offset);

  // Here's a special case: if the multifile begins with a hash character,
  // then we continue reading and discarding lines of ASCII text, until we
  // come across a nonempty line that does not begin with a hash character.
  // This allows a P3D application (which is a multifile) to be run directly
  // on the command line on Unix-based systems.
  _header_prefix = string();
  int ch = read->get();

  if (ch == '#') {
    while (ch != EOF && ch == '#') {
      // Skip to the end of the line.
      while (ch != EOF && ch != '\n') {
        _header_prefix += ch;
        ch = read->get();
      }
      // Skip to the first non-whitespace character of the line.
      while (ch != EOF && (isspace(ch) || ch == '\r')) {
        _header_prefix += ch;
        ch = read->get();
      }
    }
  }

  // Now read the actual Multifile header.
  this_header[0] = ch;
  read->read(this_header + 1, _header_size - 1);
  if (read->fail() || read->gcount() != (unsigned)(_header_size - 1)) {
    express_cat.info()
      << "Unable to read Multifile header " << _multifile_name << ".\n";
    _read->release();
    close();
    return false;
  }

  if (memcmp(this_header, _header, _header_size) != 0) {
    express_cat.info()
      << _multifile_name << " is not a Multifile.\n";
    _read->release();
    close();
    return false;
  }

  // Now get the version numbers out.
  StreamReader reader(read, false);
  _file_major_ver = reader.get_int16();
  _file_minor_ver = reader.get_int16();
  _scale_factor = reader.get_uint32();
  _new_scale_factor = _scale_factor;

  if (read->eof() || read->fail()) {
    express_cat.info()
      << _multifile_name << " header is truncated.\n";
    _read->release();
    close();
    return false;
  }

  if (_file_major_ver != _current_major_ver ||
      (_file_major_ver == _current_major_ver &&
       _file_minor_ver > _current_minor_ver)) {
    express_cat.info()
      << _multifile_name << " has version " << _file_major_ver << "."
      << _file_minor_ver << ", expecting version "
      << _current_major_ver << "." << _current_minor_ver << ".\n";
    _read->release();
    close();
    return false;
  }

  _record_timestamp = true;
  if (_file_minor_ver >= 1) {
    time_t read_timestamp = reader.get_uint32();
    if (read_timestamp == 0) {
      // If we read a 0 timestamp from the file, that implies that we don't
      // want to record a timestamp in this particular file.
      _record_timestamp = false;
    } else {
      _timestamp = read_timestamp;
    }
    _timestamp_dirty = false;
  }

  // Now read the index out.
  streampos curr_pos = read->tellg() - _offset;
  _next_index = normalize_streampos(curr_pos);
  if (_next_index > curr_pos) {
    read->ignore(_next_index - curr_pos);
  }
  _last_index = 0;
  _last_data_byte = 0;
  streampos index_forward;
  streamoff bytes_skipped = 0;
  bool read_cert_special = false;

  Subfile *subfile = new Subfile;
  index_forward = subfile->read_index(*read, _next_index, this);
  while (index_forward != (streampos)0) {
    _last_index = _next_index;
    if (subfile->is_deleted()) {
      // Ignore deleted Subfiles in the index.
      _needs_repack = true;
      delete subfile;
    } else if (subfile->is_cert_special()) {
      // Certificate chains and signature files get stored in a special list.
      _cert_special.push_back(subfile);
      read_cert_special = true;
    } else {
      _subfiles.push_back(subfile);
    }
    if (!subfile->is_cert_special()) {
      if (bytes_skipped != 0) {
        // If the index entries don't follow exactly sequentially (except for
        // the cert special files), the file ought to be repacked.
        _needs_repack = true;
      }
      if (read_cert_special) {
        // If we read a normal subfile following a cert_special entry, the
        // file ought to be repacked (certificates have to go at the end).
        _needs_repack = true;
      }
      _last_data_byte = max(_last_data_byte, subfile->get_last_byte_pos());
    }
    streampos curr_pos = read->tellg() - _offset;
    bytes_skipped = index_forward - normalize_streampos(curr_pos);
    _next_index = index_forward;
    if (_next_index > curr_pos) {
      read->ignore(_next_index - curr_pos);
    }
    subfile = new Subfile;
    index_forward = subfile->read_index(*read, _next_index, this);
  }
  if (subfile->is_index_invalid()) {
    express_cat.info()
      << "Error reading index for " << _multifile_name << ".\n";
    _read->release();
    close();
    delete subfile;
    return false;
  }

  // Check if the list is already sorted.  If it is not, we need a repack.
  for (size_t si = 1; si < _subfiles.size() && !_needs_repack; ++si) {
    if (*_subfiles[si] < *_subfiles[si - 1]) {
      _needs_repack = true;
    }
  }

  if (_needs_repack) {
    // At least sort them now.
    size_t before_size = _subfiles.size();
    _subfiles.sort();
    size_t after_size = _subfiles.size();

    // If these don't match, the same filename appeared twice in the index,
    // which shouldn't be possible.
    nassertr(before_size == after_size, true);
  }

  delete subfile;
  _read->release();
  return true;
}

/**
 * Writes just the header part of the Multifile, not the index.
 */
bool Multifile::
write_header() {
  _file_major_ver = _current_major_ver;
  _file_minor_ver = _current_minor_ver;

  nassertr(_write != nullptr, false);
  nassertr(_write->tellp() == (streampos)0, false);
  _write->write(_header_prefix.data(), _header_prefix.size());
  _write->write(_header, _header_size);
  StreamWriter writer(_write, false);
  writer.add_int16(_current_major_ver);
  writer.add_int16(_current_minor_ver);
  writer.add_uint32(_scale_factor);

  if (_record_timestamp) {
    writer.add_uint32(_timestamp);
  } else {
    writer.add_uint32(0);
    _timestamp_dirty = false;
  }

  _next_index = _write->tellp();
  _next_index = pad_to_streampos(_next_index);
  _last_index = 0;

  if (_write->fail()) {
    express_cat.info()
      << "Unable to write header for " << _multifile_name << ".\n";
    close();
    return false;
  }

  return true;
}

/**
 * Walks through the list of _cert_special entries in the Multifile, moving
 * any valid signatures found to _signatures.  After this call, _cert_special
 * will be empty.
 *
 * This does not check the validity of the certificates themselves.  It only
 * checks that they correctly sign the Multifile contents.
 */
void Multifile::
check_signatures() {
#ifdef HAVE_OPENSSL
  PendingSubfiles::iterator pi;

  for (pi = _cert_special.begin(); pi != _cert_special.end(); ++pi) {
    Subfile *subfile = (*pi);
    nassertv((subfile->_flags & SF_signature) != 0);

    // Extract the signature data and certificate separately.
    istream *stream = open_read_subfile(subfile);
    nassertv(stream != nullptr);
    StreamReader reader(*stream);
    size_t sig_size = reader.get_uint32();
    vector_uchar sig_data = reader.extract_bytes(sig_size);

    size_t num_certs = reader.get_uint32();

    // Read the remaining buffer of certificate data.
    vector_uchar buffer;
    bool success = VirtualFile::simple_read_file(stream, buffer);
    nassertv(success);
    close_read_subfile(stream);

    // Now convert each of the certificates to an X509 object, and store it in
    // our CertChain.
    CertChain chain;
    EVP_PKEY *pkey = nullptr;
    if (!buffer.empty()) {
#if OPENSSL_VERSION_NUMBER >= 0x00908000L
      // Beginning in 0.9.8, d2i_X509() accepted a const unsigned char **.
      const unsigned char *bp, *bp_end;
#else
      // Prior to 0.9.8, d2i_X509() accepted an unsigned char **.
      unsigned char *bp, *bp_end;
#endif
      bp = (unsigned char *)&buffer[0];
      bp_end = bp + buffer.size();
      X509 *x509 = d2i_X509(nullptr, &bp, bp_end - bp);
      while (num_certs > 0 && x509 != nullptr) {
        chain.push_back(CertRecord(x509));
        --num_certs;
        x509 = d2i_X509(nullptr, &bp, bp_end - bp);
      }
      if (num_certs != 0 || x509 != nullptr) {
        express_cat.warning()
          << "Extra data in signature record.\n";
      }
    }

    if (!chain.empty()) {
      pkey = X509_get_pubkey(chain[0]._cert);
    }

    if (pkey != nullptr) {
      EVP_MD_CTX *md_ctx = EVP_MD_CTX_create();
      EVP_VerifyInit(md_ctx, EVP_sha1());

      nassertv(_read != nullptr);
      _read->acquire();
      istream *read = _read->get_istream();

      // Read and hash the multifile contents, but only up till
      // _last_data_byte.
      read->seekg(_offset);
      streampos bytes_remaining = _last_data_byte;
      static const size_t buffer_size = 4096;
      char buffer[buffer_size];
      read->read(buffer, min((streampos)buffer_size, bytes_remaining));
      size_t count = read->gcount();
      while (count != 0) {
        nassertv(count <= buffer_size);
        EVP_VerifyUpdate(md_ctx, buffer, count);
        bytes_remaining -= count;
        read->read(buffer, min((streampos)buffer_size, bytes_remaining));
        count = read->gcount();
      }
      nassertv(bytes_remaining == (streampos)0);
      _read->release();

      // Now check that the signature matches the hash.
      int verify_result =
        EVP_VerifyFinal(md_ctx, sig_data.data(), sig_data.size(), pkey);
      if (verify_result == 1) {
        // The signature matches; save the certificate and its chain.
        _signatures.push_back(chain);
      } else {
        // Bad match.
        _needs_repack = true;
      }
    }
  }
#endif  // HAVE_OPENSSL

  _cert_special.clear();
}

/**
 * Reads the index record for the Subfile from the indicated istream.  Assumes
 * the istream has already been positioned to the indicated stream position,
 * fpos, the start of the index record.  Returns the position within the file
 * of the next index record.
 */
streampos Multifile::Subfile::
read_index(istream &read, streampos fpos, Multifile *multifile) {
  nassertr(read.tellg() - multifile->_offset == fpos, fpos);

  // First, get the next stream position.  We do this separately, because if
  // it is zero, we don't get anything else.
  StreamReader reader(read);

  streampos next_index = multifile->word_to_streampos(reader.get_uint32());
  if (read.fail()) {
    _flags |= SF_index_invalid;
    return 0;
  }

  if (next_index == (streampos)0) {
    return 0;
  }

  // Now get the rest of the index.

  _index_start = fpos;
  _index_length = 0;

  _data_start = multifile->word_to_streampos(reader.get_uint32());
  _data_length = reader.get_uint32();
  _flags = reader.get_uint16();
  if ((_flags & (SF_compressed | SF_encrypted)) != 0) {
    _uncompressed_length = reader.get_uint32();
  } else {
    _uncompressed_length = _data_length;
  }
  if (multifile->_file_minor_ver < 1) {
    _timestamp = multifile->get_timestamp();
  } else {
    _timestamp = reader.get_uint32();
    if (_timestamp == 0) {
      _timestamp = multifile->get_timestamp();
    }
  }

  size_t name_length = reader.get_uint16();
  if (read.fail()) {
    _flags |= SF_index_invalid;
    return 0;
  }

  // And finally, get the rest of the name.
  char *name_buffer = (char *)PANDA_MALLOC_ARRAY(name_length);
  nassertr(name_buffer != nullptr, next_index);
  for (size_t ni = 0; ni < name_length; ni++) {
    name_buffer[ni] = read.get() ^ 0xff;
  }
  _name = string(name_buffer, name_length);
  PANDA_FREE_ARRAY(name_buffer);

  if (read.fail()) {
    _flags |= SF_index_invalid;
    return 0;
  }

  _index_length = read.tellg() - fpos - multifile->_offset;
  return next_index;
}

/**
 * Writes the index record for the Subfile to the indicated ostream.  Assumes
 * the istream has already been positioned to the indicated stream position,
 * fpos, the start of the index record, and that this is the effective end of
 * the file.  Returns the position within the file of the next index record.
 *
 * The _index_start member is updated by this operation.
 */
streampos Multifile::Subfile::
write_index(ostream &write, streampos fpos, Multifile *multifile) {
  nassertr(write.tellp() - multifile->_offset == fpos, fpos);

  _index_start = fpos;
  _index_length = 0;

  // This will be the contents of this particular index record.  We build it
  // up first since it will be variable length.
  Datagram dg;
  dg.add_uint32(multifile->streampos_to_word(_data_start));
  dg.add_uint32(_data_length);
  dg.add_uint16(_flags);
  if ((_flags & (SF_compressed | SF_encrypted)) != 0) {
    dg.add_uint32(_uncompressed_length);
  }
  dg.add_uint32(_timestamp);
  dg.add_uint16(_name.length());

  // For no real good reason, we'll invert all the bits in the name.  The only
  // reason we do this is to make it inconvenient for a casual browser of the
  // Multifile to discover the names of the files stored within it.
  // Naturally, this isn't real obfuscation or security.
  string::iterator ni;
  for (ni = _name.begin(); ni != _name.end(); ++ni) {
    dg.add_int8((*ni) ^ 0xff);
  }

  size_t this_index_size = 4 + dg.get_length();

  // Plus, we will write out the next index address first.
  streampos next_index = fpos + (streampos)this_index_size;

  Datagram idg;
  idg.add_uint32(multifile->streampos_to_word(next_index));

  write.write((const char *)idg.get_data(), idg.get_length());
  write.write((const char *)dg.get_data(), dg.get_length());

  _index_length = write.tellp() - fpos - multifile->_offset;
  return next_index;
}

/**
 * Writes the data record for the Subfile to the indicated ostream: the actual
 * contents of the Subfile.  Assumes the istream has already been positioned
 * to the indicated stream position, fpos, the start of the data record, and
 * that this is the effective end of the file.  Returns the position within
 * the file of the next data record.
 *
 * The _data_start, _data_length, and _uncompressed_length members are updated
 * by this operation.
 *
 * If the "read" pointer is non-NULL, it is the readable istream of a
 * Multifile in which the Subfile might already be packed.  This is used for
 * reading the contents of the Subfile during a repack() operation.
 */
streampos Multifile::Subfile::
write_data(ostream &write, istream *read, streampos fpos,
           Multifile *multifile) {
  nassertr(write.tellp() - multifile->_offset == fpos, fpos);

  istream *source = _source;
  pifstream source_file;
  if (source == nullptr && !_source_filename.empty()) {
    // If we have a filename, open it up and read that.
    if (!_source_filename.open_read(source_file)) {
      // Unable to open the source file.
      express_cat.info()
        << "Unable to read " << _source_filename << ".\n";
      _flags |= SF_data_invalid;
      _data_length = 0;
      _uncompressed_length = 0;
    } else {
      source = &source_file;
    }
  }

  if (source == nullptr) {
    // We don't have any source data.  Perhaps we're reading from an already-
    // packed Subfile (e.g.  during repack()).
    if (read == nullptr) {
      // No, we're just screwed.
      express_cat.info()
        << "No source for subfile " << _name << ".\n";
      _flags |= SF_data_invalid;
    } else {
      // Read the data from the original Multifile.
      read->seekg(_data_start + multifile->_offset);
      for (size_t p = 0; p < _data_length; p++) {
        int byte = read->get();
        if (read->eof() || read->fail()) {
          // Unexpected EOF or other failure on the source file.
          express_cat.info()
            << "Unexpected EOF for subfile " << _name << ".\n";
          _flags |= SF_data_invalid;
          break;
        }
        write.put(byte);
      }
    }
  } else {
    // We do have source data.  Copy it in, and also measure its length.
    ostream *putter = &write;
    bool delete_putter = false;

#ifndef HAVE_OPENSSL
    // Without OpenSSL, we can't support encryption.  The flag had better not
    // be set.
    nassertr((_flags & SF_encrypted) == 0, fpos);

#else  // HAVE_OPENSSL
    if ((_flags & SF_encrypted) != 0) {
      // Write it encrypted.
      OEncryptStream *encrypt = new OEncryptStream;
      encrypt->set_iteration_count(multifile->_encryption_iteration_count);
      encrypt->open(putter, delete_putter, multifile->_encryption_password);

      putter = encrypt;
      delete_putter = true;

      // Also write the encrypt_header to the beginning of the encrypted
      // stream, so we can validate the password on decryption.
      putter->write(_encrypt_header, _encrypt_header_size);
    }
#endif  // HAVE_OPENSSL

#ifndef HAVE_ZLIB
    // Without ZLIB, we can't support compression.  The flag had better not be
    // set.
    nassertr((_flags & SF_compressed) == 0, fpos);
#else  // HAVE_ZLIB
    if ((_flags & SF_compressed) != 0) {
      // Write it compressed.
      putter = new OCompressStream(putter, delete_putter, _compression_level);
      delete_putter = true;
    }
#endif  // HAVE_ZLIB

    streampos write_start = fpos;
    _uncompressed_length = 0;

#ifndef HAVE_OPENSSL
    // We also need OpenSSL for signatures.
    nassertr((_flags & SF_signature) == 0, fpos);

#else  // HAVE_OPENSSL
    if ((_flags & SF_signature) != 0) {
      // If it's a special signature record, precede the record data (the
      // certificate itself) with the signature data generated against the
      // multifile contents.

      // In order to generate a signature, we need to have a valid read
      // pointer.
      nassertr(read != nullptr, fpos);

      // And we also need to have a private key.
      nassertr(_pkey != nullptr, fpos);

      EVP_MD_CTX *md_ctx = EVP_MD_CTX_create();
      EVP_SignInit(md_ctx, EVP_sha1());

      // Read and hash the multifile contents, but only up till
      // _last_data_byte.
      nassertr(multifile->_last_data_byte < fpos, fpos);
      read->seekg(multifile->_offset);
      streampos bytes_remaining = multifile->_last_data_byte;
      static const size_t buffer_size = 4096;
      char buffer[buffer_size];
      read->read(buffer, min((streampos)buffer_size, bytes_remaining));
      size_t count = read->gcount();
      while (count != 0) {
        nassertr(count <= buffer_size, fpos);
        EVP_SignUpdate(md_ctx, buffer, count);
        bytes_remaining -= count;
        read->read(buffer, min((streampos)buffer_size, bytes_remaining));
        count = read->gcount();
      }
      nassertr(bytes_remaining == (streampos)0, fpos);

      // Now generate and write out the signature.
      unsigned int max_size = EVP_PKEY_size(_pkey);
      unsigned char *sig_data = new unsigned char[max_size];
      unsigned int sig_size;
      if (!EVP_SignFinal(md_ctx, sig_data, &sig_size, _pkey)) {
        OpenSSLWrapper *sslw = OpenSSLWrapper::get_global_ptr();
        sslw->notify_ssl_errors();
      }
      nassertr(sig_size <= max_size, fpos);

      StreamWriter writer(*putter);
      writer.add_uint32(sig_size);
      putter->write((char *)sig_data, sig_size);
      _uncompressed_length += 4 + sig_size;

      delete[] sig_data;

      EVP_MD_CTX_destroy(md_ctx);
    }
#endif  // HAVE_OPENSSL

    // Finally, we can write out the data itself.
    static const size_t buffer_size = 4096;
    char buffer[buffer_size];

    source->read(buffer, buffer_size);
    size_t count = source->gcount();
    while (count != 0) {
      _uncompressed_length += count;
      putter->write(buffer, count);
      source->read(buffer, buffer_size);
      count = source->gcount();
    }

    if (delete_putter) {
      delete putter;
    }

    streampos write_end = write.tellp() - multifile->_offset;
    _data_length = (size_t)(write_end - write_start);
  }

  // We can't set _data_start until down here, after we have read the Subfile.
  // (In case we are running during repack()).
  _data_start = fpos;

  // Get the modification timestamp for this subfile.  This is read from the
  // source file, if we have a filename; otherwise, it's the current time.
  if (!_source_filename.empty()) {
    _timestamp = _source_filename.get_timestamp();
  }
  if (_timestamp == 0) {
    _timestamp = time(nullptr);
  }

  _source = nullptr;
  _source_filename = Filename();
  source_file.close();

  return fpos + (streampos)_data_length;
}

/**
 * Seeks within the indicate pfstream back to the index record and rewrites
 * just the _data_start and _data_length part of the index record.
 */
void Multifile::Subfile::
rewrite_index_data_start(ostream &write, Multifile *multifile) {
  nassertv(_index_start != (streampos)0);

  static const size_t data_start_offset = 4;
  size_t data_start_pos = _index_start + (streampos)data_start_offset;
  write.seekp(data_start_pos + multifile->_offset);
  nassertv(!write.fail());

  StreamWriter writer(write);
  writer.add_uint32(multifile->streampos_to_word(_data_start));
  writer.add_uint32(_data_length);
  writer.add_uint16(_flags);
  if ((_flags & (SF_compressed | SF_encrypted)) != 0) {
    writer.add_uint32(_uncompressed_length);
  }
  if (multifile->_record_timestamp) {
    writer.add_uint32(_timestamp);
  } else {
    writer.add_uint32(0);
  }
}

/**
 * Seeks within the indicated ostream back to the index record and rewrites
 * just the _flags part of the index record.
 */
void Multifile::Subfile::
rewrite_index_flags(ostream &write) {
  // If the subfile has never even been recorded to disk, we don't need to do
  // anything at all in this function.
  if (_index_start != (streampos)0) {
    static const size_t flags_offset = 4 + 4 + 4;
    size_t flags_pos = _index_start + (streampos)flags_offset;
    write.seekp(flags_pos);
    nassertv(!write.fail());

    StreamWriter writer(write);
    writer.add_uint16(_flags);
  }
}
