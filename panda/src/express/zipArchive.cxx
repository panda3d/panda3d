/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zipArchive.cxx
 * @author rdb
 * @date 2019-01-20
 */

#include "zipArchive.h"

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

using std::streamoff;
using std::streampos;
using std::streamsize;
using std::stringstream;
using std::string;

// 1980-01-01 00:00:00
static const time_t dos_epoch = 315532800;

#ifdef HAVE_OPENSSL
/**
 * Encodes the given string using base64 encoding.
 */
static std::string base64_encode(const void *buf, int len) {
  BIO *b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

  BIO *sink = BIO_new(BIO_s_mem());
  BIO_push(b64, sink);
  BIO_write(b64, buf, len);
  BIO_flush(b64);

  const char *encoded;
  const long encoded_len = BIO_get_mem_data(sink, &encoded);
  std::string result(encoded, encoded_len);
  BIO_free_all(b64);
  return result;
}
#endif

/**
 *
 */
ZipArchive::
ZipArchive() :
  _read_filew(_read_file),
  _read_write_filew(_read_write_file)
{
  _read = nullptr;
  _write = nullptr;
  _owns_stream = false;
  _index_changed = false;
  _needs_repack = false;
  _record_timestamp = true;
}

/**
 *
 */
ZipArchive::
~ZipArchive() {
  close();
}

/**
 * Opens the named ZipArchive on disk for reading.  The ZipArchive index is read
 * in, and the list of subfiles becomes available; individual subfiles may
 * then be extracted or read, but the list of subfiles may not be modified.
 *
 * Also see the version of open_read() which accepts an istream.  Returns true
 * on success, false on failure.
 */
bool ZipArchive::
open_read(const Filename &filename) {
  close();
  Filename fname = filename;
  fname.set_binary();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vfile = vfs->get_file(fname);
  if (vfile == nullptr) {
    return false;
  }
  std::istream *stream = vfile->open_read_file(false);
  if (stream == nullptr) {
    return false;
  }

  _read = new IStreamWrapper(stream, true);
  _owns_stream = true;
  _filename = filename;
  return read_index();
}

/**
 * Opens an anonymous ZipArchive for reading using an istream.  There must be
 * seek functionality via seekg() and tellg() on the istream.
 *
 * If owns_pointer is true, then the ZipArchive assumes ownership of the stream
 * pointer and will delete it when the ZIP file is closed, including if this
 * function returns false.
 *
 * The given stream must be seekable.
 */
bool ZipArchive::
open_read(IStreamWrapper *stream, bool owns_pointer) {
  close();
  _read = stream;
  _owns_stream = owns_pointer;
  return read_index();
}

/**
 * Opens the named ZipArchive on disk for writing.  If there already exists a
 * file by that name, it is truncated.  The ZipArchive is then prepared for
 * accepting a brand new set of subfiles, which will be written to the
 * indicated filename.  Individual subfiles may not be extracted or read.
 *
 * Also see the version of open_write() which accepts an ostream.  Returns
 * true on success, false on failure.
 */
bool ZipArchive::
open_write(const Filename &filename) {
  close();
  Filename fname = filename;
  fname.set_binary();
  if (!fname.open_write(_write_file, true)) {
    return false;
  }
  _write = &_write_file;
  _filename = filename;
  _index_start = 0;
  _file_end = 0;
  _index_changed = true;
  return true;
}

/**
 * Opens an anonymous ZipArchive for writing using an ostream.
 *
 * If owns_pointer is true, then the ZipArchive assumes ownership of the stream
 * pointer and will delete it when the ZIP file is closed, including if this
 * function returns false.
 */
bool ZipArchive::
open_write(std::ostream *stream, bool owns_pointer) {
  close();
  _write = stream;
  _owns_stream = owns_pointer;
  _write->seekp(0, std::ios::beg);
  _index_start = 0;
  _file_end = 0;
  _index_changed = true;
  return true;
}

/**
 * Opens the named ZipArchive on disk for reading and writing.  If there
 * already exists a file by that name, its index is read.  Subfiles may be
 * added or removed, and the resulting changes will be written to the named
 * file.
 *
 * Also see the version of open_read_write() which accepts an iostream.
 * Returns true on success, false on failure.
 */
bool ZipArchive::
open_read_write(const Filename &filename) {
  close();
  Filename fname = filename;
  fname.set_binary();
  bool exists = fname.exists();
  if (!fname.open_read_write(_read_write_file)) {
    return false;
  }
  _read = &_read_write_filew;
  _write = &_read_write_file;
  _filename = filename;

  if (exists) {
    return read_index();
  } else {
    _index_start = 0;
    _file_end = 0;
    _index_changed = true;
    return true;
  }
}

/**
 * Opens an anonymous ZipArchive for reading and writing using an iostream.
 * There must be seek functionality via seekg()/seekp() and tellg()/tellp() on
 * the iostream.
 *
 * If owns_pointer is true, then the ZipArchive assumes ownership of the stream
 * pointer and will delete it when the ZIP file is closed, including if this
 * function returns false.
 */
bool ZipArchive::
open_read_write(std::iostream *stream, bool owns_pointer) {
  close();

  // We don't support locking when opening a file in read-write mode, because
  // we don't bother with locking on write.  But we need to have an
  // IStreamWrapper to assign to the _read member, so we create one on-the-fly
  // here.
  _read = new StreamWrapper(stream, owns_pointer);
  _write = stream;
  _owns_stream = true;  // Because we own the StreamWrapper, above.
  _write->seekp(0, std::ios::beg);

  // Check whether the read stream is empty.
  stream->seekg(0, std::ios::end);
  if (stream->tellg() == (streampos)0) {
    // The read stream is empty, which is always valid.
    _index_changed = true;
    return true;
  }

  // The read stream is not empty, so we'd better have a valid ZipArchive.
  return read_index();
}

/**
 * Verifies the integrity of the contents of the ZIP archive.
 */
bool ZipArchive::
verify() {
  nassertr_always(is_read_valid(), false);

  _read->acquire();
  std::istream *read = _read->get_istream();

  bool passes = true;

  for (Subfile *subfile : _subfiles) {
    if (!subfile->read_header(*read) ||
        !subfile->verify_data(*read)) {
      passes = false;
    }
  }

  _read->release();
  return passes;
}

/**
 * Closes the ZipArchive if it is open.  All changes are flushed to disk, and
 * the file becomes invalid for further operations until the next call to
 * open().
 */
void ZipArchive::
close() {
  flush();

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
  _owns_stream = false;
  _index_start = 0;
  _file_end = 0;
  _index_changed = false;
  _needs_repack = false;

  _read_file.close();
  _write_file.close();
  _read_write_file.close();
  _filename = Filename();

  clear_subfiles();
}

/**
 * Adds a file on disk as a subfile to the ZipArchive.  The file named by
 * filename will be read and added to the ZipArchive immediately, but the index
 * will not be updated until you call flush().  If there already exists a
 * subfile with the indicated name, it is replaced without examining its
 * contents (but see also update_subfile).
 *
 * Returns the subfile name on success (it might have been modified slightly),
 * or empty string on failure.
 */
std::string ZipArchive::
add_subfile(const std::string &subfile_name, const Filename &filename,
            int compression_level) {
  nassertr(is_write_valid(), std::string());

#ifndef HAVE_ZLIB
  express_cat.warning()
    << "zlib not compiled in; unable to modify ZIP file.\n";
  return std::string();
#endif

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vfile = vfs->get_file(filename);
  if (vfile == nullptr) {
    return std::string();
  }

  std::istream *in = vfs->open_read_file(filename, false);
  if (in == nullptr) {
    return std::string();
  }

  std::string name = add_subfile(subfile_name, in, compression_level);
  vfs->close_read_file(in);
  return name;
}

/**
 * Adds a file from a stream as a subfile to the ZipArchive.  The indicated
 * istream will be read and its contents added to the end of the current ZIP
 * file immediately.
 *
 * Note that the istream must remain untouched and unused by any other code
 * until flush() is called.  At that time, the index of the ZIP archive will be
 * rewritten to the end of the file.
 *
 * Returns the subfile name on success (it might have been modified slightly),
 * or empty string on failure.
 */
std::string ZipArchive::
add_subfile(const std::string &subfile_name, std::istream *subfile_data,
            int compression_level) {
  nassertr(is_write_valid(), string());

#ifndef HAVE_ZLIB
  express_cat.warning()
    << "zlib not compiled in; unable to modify ZIP file.\n";
  return std::string();
#endif

  std::string name = standardize_subfile_name(subfile_name);
  if (!name.empty()) {
    Subfile *subfile = new Subfile(subfile_name, compression_level);

    // Write it straight away, overwriting the index at the end of the file.
    // This index will be rewritten at the next call to flush() or close().
    std::streampos fpos = _index_start;
    _write->seekp(fpos);

    if (!subfile->write_header(*_write, fpos)) {
      delete subfile;
      return "";
    }

    if (!subfile->write_data(*_write, subfile_data, fpos, compression_level)) {
      // Failed to write the data.
      delete subfile;
      return "";
    }

    if (fpos > _index_start) {
      _index_start = fpos;
    }
    add_new_subfile(subfile, compression_level);
  }

  return name;
}

/**
 * Adds a file on disk to the subfile.  If a subfile already exists with the
 * same name, its contents are compared byte-for-byte to the disk file, and it
 * is replaced only if it is different; otherwise, the ZIP file is left
 * unchanged.
 *
 * Either Filename:::set_binary() or set_text() must have been called
 * previously to specify the nature of the source file.  If set_text() was
 * called, the text flag will be set on the subfile.
 */
string ZipArchive::
update_subfile(const std::string &subfile_name, const Filename &filename,
               int compression_level) {
  nassertr(is_write_valid(), string());

#ifndef HAVE_ZLIB
  express_cat.warning()
    << "zlib not compiled in; unable to modify ZIP file.\n";
  return std::string();
#endif

  if (!filename.exists()) {
    return string();
  }
  std::string name = standardize_subfile_name(subfile_name);
  if (!name.empty()) {
    int index = find_subfile(name);
    if (index >= 0) {
      // The subfile already exists; compare it to the source file.
      if (compare_subfile(index, filename)) {
        // The files are identical; do nothing.
        return name;
      }
    }

    // The subfile does not already exist or it is different from the source
    // file.  Add the new source file.
    Subfile *subfile = new Subfile(name, compression_level);
    add_new_subfile(subfile, compression_level);
  }

  return name;
}

#ifdef HAVE_OPENSSL
/**
 * Adds a new JAR-style signature to the .zip file.  The file must have been
 * opened in read/write mode.
 *
 * This implicitly causes a repack() operation if one is needed.  Returns true
 * on success, false on failure.
 *
 * This flavor of add_jar_signature() reads the certificate and private key
 * from a PEM-formatted file, for instance as generated by the openssl command.
 * If the private key file is password-encrypted, the third parameter will be
 * used as the password to decrypt it.
 *
 * It's possible to add multiple signatures, by providing multiple unique
 * aliases.  Note that aliases are considered case-insensitively and only the
 * first 8 characters are considered.
 *
 * There is no separate parameter to pass a certificate chain.  Instead, any
 * necessary certificates are expected to be in the certificate file.
 */
bool ZipArchive::
add_jar_signature(const Filename &certificate, const Filename &pkey,
                  const string &password, const string &alias) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  // Read the certificate file from VFS.  First, read the complete file into
  // memory.
  string certificate_data;
  if (!vfs->read_file(certificate, certificate_data, true)) {
    express_cat.error()
      << "Could not read " << certificate << ".\n";
    return false;
  }

  // Now do the same thing with the private key.  This one may be password-
  // encrypted on disk.
  string pkey_data;
  if (!vfs->read_file(pkey, pkey_data, true)) {
    express_cat.error()
      << "Could not read " << pkey << ".\n";
    return false;
  }

  // Create an in-memory BIO to read the "file" from the buffer.
  BIO *certificate_mbio = BIO_new_mem_buf((void *)certificate_data.data(), certificate_data.size());
  X509 *cert = PEM_read_bio_X509(certificate_mbio, nullptr, nullptr, (void *)"");
  BIO_free(certificate_mbio);
  if (cert == nullptr) {
    express_cat.error()
      << "Could not read certificate in " << certificate << ".\n";
    return false;
  }

  // Same with private key.
  BIO *pkey_mbio = BIO_new_mem_buf((void *)pkey_data.data(), pkey_data.size());
  EVP_PKEY *evp_pkey = PEM_read_bio_PrivateKey(pkey_mbio, nullptr, nullptr,
                                               (void *)password.c_str());
  BIO_free(pkey_mbio);
  if (evp_pkey == nullptr) {
    express_cat.error()
      << "Could not read private key in " << pkey << ".\n";

    X509_free(cert);
    return false;
  }

  bool result = add_jar_signature(cert, evp_pkey, alias);

  X509_free(cert);
  EVP_PKEY_free(evp_pkey);

  return result;
}
#endif  // HAVE_OPENSSL

#ifdef HAVE_OPENSSL
/**
 * Adds a new JAR-style signature to the .zip file.  The file must have been
 * opened in read/write mode.
 *
 * This implicitly causes a repack() operation if one is needed.  Returns true
 * on success, false on failure.
 *
 * It's possible to add multiple signatures, by providing multiple unique
 * aliases.  Note that aliases are considered case-insensitively and only the
 * first 8 characters are considered.
 *
 * The private key is expected to match the first certificate in the chain.
 */
bool ZipArchive::
add_jar_signature(X509 *cert, EVP_PKEY *pkey, const std::string &alias) {
  nassertr(is_write_valid() && is_read_valid(), false);
  nassertr(cert != nullptr, false);
  nassertr(pkey != nullptr, false);

  if (!X509_check_private_key(cert, pkey)) {
    express_cat.error()
      << "Private key does not match certificate.\n";
    return false;
  }

  const char *ext;
  int algo = EVP_PKEY_base_id(pkey);
  switch (algo) {
  case EVP_PKEY_RSA:
    ext = ".RSA";
    break;
  case EVP_PKEY_DSA:
    ext = ".DSA";
    break;
  case EVP_PKEY_EC:
    ext = ".EC";
    break;
  default:
    express_cat.error()
      << "Private key has unsupported algorithm.\n";
    return false;
  }

  // Sanitize alias to be used in a filename.
  std::string basename;
  for (char c : alias.substr(0, 8)) {
    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || c == '-' || c == '_') {
      basename += c;
    }
    else if (c >= 'a' && c <= 'z') {
      basename += (c - 0x20);
    }
    else if (((uint8_t)c & 0xc0) != 0x80) {
      basename += '_';
    }
  }

  // Generate a MANIFEST.MF file.
  const std::string header = "Manifest-Version: 1.0\r\n\r\n";
  const std::string header_digest = "VmrRqAIgAm0FCZViZFzpaP8OfDbN4iY0MyYFuzTMPv8=";

  std::stringstream manifest;
  SHA256_CTX manifest_ctx;
  SHA256_Init(&manifest_ctx);

  manifest << header;
  SHA256_Update(&manifest_ctx, header.data(), header.size());

  std::ostringstream sigfile_body;

  for (Subfile *subfile : _subfiles) {
    nassertr(subfile != nullptr, false);

    if (subfile->_name.compare(0, 9, "META-INF/") == 0) {
      continue;
    }

    std::string section = "Name: " + subfile->_name + "\r\n";
    sigfile_body << section;

    // Hash the subfile.
    unsigned char digest[SHA256_DIGEST_LENGTH];
    {
      std::istream *stream = open_read_subfile(subfile);

      SHA256_CTX subfile_ctx;
      SHA256_Init(&subfile_ctx);

      char buffer[4096];
      stream->read(buffer, sizeof(buffer));
      size_t count = stream->gcount();
      while (count > 0) {
        SHA256_Update(&subfile_ctx, buffer, count);
        stream->read(buffer, sizeof(buffer));
        count = stream->gcount();
      }
      delete stream;

      SHA256_Final(digest, &subfile_ctx);
    }

    // Encode to base64.
    section += "SHA-256-Digest: " + base64_encode(digest, SHA256_DIGEST_LENGTH) + "\r\n\r\n";

    // Encode what we just wrote to the manifest file as well.
    {
      unsigned char digest[SHA256_DIGEST_LENGTH];

      SHA256_CTX section_ctx;
      SHA256_Init(&section_ctx);
      SHA256_Update(&section_ctx, section.data(), section.size());
      SHA256_Final(digest, &section_ctx);

      sigfile_body << "SHA-256-Digest: " << base64_encode(digest, SHA256_DIGEST_LENGTH) << "\r\n\r\n";
    }

    manifest << section;
    SHA256_Update(&manifest_ctx, section.data(), section.size());
  }

  // The hash for the whole manifest file goes at the beginning of the .SF file.
  std::stringstream sigfile;
  {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_Final(digest, &manifest_ctx);
    sigfile << "Signature-Version: 1.0\r\n";
    sigfile << "SHA-256-Digest-Manifest-Main-Attributes: " << header_digest << "\r\n";
    sigfile << "SHA-256-Digest-Manifest: " << base64_encode(digest, SHA256_DIGEST_LENGTH) << "\r\n\r\n";
    sigfile << sigfile_body.str();
  }

  // Sign and convert to to DER format
  std::string sigfile_data = sigfile.str();
  BIO *sigfile_mbio = BIO_new_mem_buf((void *)sigfile_data.data(), sigfile_data.size());
  PKCS7 *p7 = PKCS7_sign(cert, pkey, nullptr, sigfile_mbio, PKCS7_DETACHED | PKCS7_NOATTR);
  int der_len = i2d_PKCS7(p7, nullptr);
  std::string signature_str(der_len, '\0');
  unsigned char *p = (unsigned char *)signature_str.data();
  i2d_PKCS7(p7, &p);
  std::istringstream signature(std::move(signature_str));
  PKCS7_free(p7);

  add_subfile("META-INF/MANIFEST.MF", &manifest, 9);
  add_subfile("META-INF/" + basename + ".SF", &sigfile, 9);
  add_subfile("META-INF/" + basename + ext, &signature, 9);

  return true;
}
#endif  // HAVE_OPENSSL

/**
 * Ensures that any changes made to the ZIP archive have been synchronized to
 * disk.  In particular, this causes the central directory to be rewritten at
 * the end of the file.
 *
 * This may result in a suboptimal packing in the ZIP file, especially if
 * existing files were changed or files were removed.  To guarantee that the
 * file is as compact as it can be, call repack() instead of flush().
 *
 * It is not necessary to call flush() explicitly unless you are concerned
 * about reading the recently-added subfiles immediately.
 *
 * Returns true on success, false on failure.
 */
bool ZipArchive::
flush() {
  if (!is_write_valid()) {
    return false;
  }

  nassertr(_write != nullptr, false);

  // First, mark out all of the removed subfiles.
  for (Subfile *subfile : _removed_subfiles) {
    delete subfile;
  }
  _removed_subfiles.clear();

  if (_index_changed) {
    std::streampos fpos = _index_start;
    _write->seekp(fpos);
    if (!write_index(*_write, fpos)) {
      express_cat.info()
        << "Unable to write updated central directory to ZIP archive " << _filename << ".\n";
      close();
      return false;
    }
    _index_changed = false;
  }

  _write->flush();
  return true;
}

/**
 * Forces a complete rewrite of the ZipArchive and all of its contents, so that
 * the files are tightly packed in the file without any gaps.  This is useful to
 * do after removing files, to ensure that the file size is minimized.
 *
 * It is only valid to call this if the ZipArchive was opened using
 * open_read_write() and an explicit filename, rather than an iostream.  Also,
 * we must have write permission to the directory containing the ZipArchive.
 *
 * Returns true on success, false on failure.
 */
bool ZipArchive::
repack() {
  nassertr(is_write_valid() && is_read_valid(), false);
  nassertr(!_filename.empty(), false);

  // First, we open a temporary filename to copy the ZipArchive to.
  Filename dirname = _filename.get_dirname();
  if (dirname.empty()) {
    dirname = ".";
  }
  Filename temp_filename = Filename::temporary(dirname, "ziptemp");
  temp_filename.set_binary();
  pofstream temp;
  if (!temp_filename.open_write(temp)) {
    express_cat.info()
      << "Unable to open temporary file " << temp_filename << "\n";
    return false;
  }

  // Now we scrub our internal structures so it looks like we're a brand new
  // ZipArchive.
  for (Subfile *subfile : _removed_subfiles) {
    delete subfile;
  }
  _removed_subfiles.clear();

  // And we write our contents to our new temporary file.
  //_write = &temp;

  bool success = true;

  _read->acquire();
  std::istream &read = *_read->get_istream();

  // Copy over all of the subfiles.
  std::streampos fpos = 0;

  for (Subfile *subfile : _subfiles) {
    if (!subfile->read_header(read)) {
      success = false;
      continue;
    }

    // We don't need to write a data descriptor, since at this point we know
    // the checksum and sizes.
    subfile->_flags &= ~SF_data_descriptor;

    if (!subfile->write_header(temp, fpos)) {
      success = false;
      continue;
    }

    static const size_t buffer_size = 4096;
    char buffer[buffer_size];
    uint64_t num_bytes = subfile->_data_length;
    fpos += num_bytes;

    while (num_bytes >= buffer_size) {
      read.read(buffer, buffer_size);
      temp.write(buffer, buffer_size);
      num_bytes -= buffer_size;
    }
    if (num_bytes > 0) {
      read.read(buffer, num_bytes);
      temp.write(buffer, num_bytes);
    }
  }
  _read->release();

  // Write the central directory at the end of the file.
  success = success && write_index(temp, fpos);

  if (!success) {
    express_cat.error()
      << "Failed to write repacked archive to " << temp_filename << ".\n";
    temp.close();
    temp_filename.unlink();
    return false;
  }

  // Now close everything, and move the temporary file back over our original
  // file.
  Filename orig_name = _filename;
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
      << "Unable to read newly repacked " << _filename
      << ".\n";
    return false;
  }

  return true;
}

/**
 * Returns the number of subfiles within the ZipArchive.  The subfiles may be
 * accessed in alphabetical order by iterating through [0 ..
 * get_num_subfiles()).
 */
int ZipArchive::
get_num_subfiles() const {
  return _subfiles.size();
}

/**
 * Returns the index of the subfile with the indicated name, or -1 if the
 * named subfile is not within the ZipArchive.
 */
int ZipArchive::
find_subfile(const std::string &subfile_name) const {
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
 * or more files within the ZipArchive.  That is, the ZipArchive contains at
 * least one file named "subfile_name/...".
 */
bool ZipArchive::
has_directory(const std::string &subfile_name) const {
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
 * ZipArchive, but not a file itself; fills the given vector up with the sorted
 * list of subdirectories or files within the named directory.
 *
 * Note that directories do not exist explicitly within a ZipArchive; this just
 * checks for the existence of files with the given initial prefix.
 *
 * Returns true if successful, false otherwise.
 */
bool ZipArchive::
scan_directory(vector_string &contents, const std::string &subfile_name) const {
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
 * Removes the nth subfile from the ZipArchive.  This will cause all subsequent
 * index numbers to decrease by one.  The file will not actually be removed
 * from the disk until the next call to flush().
 *
 * Note that this does not actually remove the data from the indicated
 * subfile; it simply removes it from the index.  The ZipArchive will not be
 * reduced in size after this operation, until the next call to repack().
 */
void ZipArchive::
remove_subfile(int index) {
  nassertv(is_write_valid());
  nassertv(index >= 0 && index < (int)_subfiles.size());
  Subfile *subfile = _subfiles[index];
  //subfile->_flags |= SF_deleted;
  _removed_subfiles.push_back(subfile);
  _subfiles.erase(_subfiles.begin() + index);

  // We'll need to rewrite the index to remove it.  The packing is also
  // suboptimal now, so a repack would be good.
  _index_changed = true;
  _needs_repack = true;
}

/**
 * Returns the name of the nth subfile.
 */
const string &ZipArchive::
get_subfile_name(int index) const {
#ifndef NDEBUG
  static string empty_string;
  nassertr(index >= 0 && index < (int)_subfiles.size(), empty_string);
#endif
  return _subfiles[index]->_name;
}

/**
 * Returns the uncompressed data length of the nth subfile.
 */
size_t ZipArchive::
get_subfile_length(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  return _subfiles[index]->_uncompressed_length;
}

/**
 * Returns the modification time of the nth subfile.  If this is called on an
 * older .zip file, which did not store individual timestamps in the file (or
 * if get_record_timestamp() is false), this will return the modification time
 * of the overall ZIP file.
 */
time_t ZipArchive::
get_subfile_timestamp(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  return _subfiles[index]->_timestamp;
}

/**
 * Returns true if the indicated subfile has been compressed when stored
 * within the archive, false otherwise.
 */
bool ZipArchive::
is_subfile_compressed(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  return _subfiles[index]->is_compressed();
}

/**
 * Returns true if the indicated subfile has been encrypted when stored within
 * the archive, false otherwise.
 */
bool ZipArchive::
is_subfile_encrypted(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  return _subfiles[index]->is_encrypted();
}

/**
 * Returns the starting byte position within the ZipArchive at which the
 * indicated subfile begins.  This may be used, with
 * get_subfile_internal_length(), for low-level access to the subfile, but
 * usually it is better to use open_read_subfile() instead (which
 * automatically decrypts and/or uncompresses the subfile data).
 */
streampos ZipArchive::
get_subfile_internal_start(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  _read->acquire();
  _subfiles[index]->read_header(*_read->get_istream());
  std::streampos data_start = _read->get_istream()->tellg();
  _read->release();
  return data_start;
}

/**
 * Returns the number of bytes the indicated subfile consumes within the
 * archive.  For compressed subfiles, this will generally be smaller than
 * get_subfile_length(); for encrypted (but noncompressed) subfiles, it may be
 * slightly different, for noncompressed and nonencrypted subfiles, it will be
 * equal.
 */
size_t ZipArchive::
get_subfile_internal_length(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  return _subfiles[index]->_data_length;
}

/**
 * Returns an istream that may be used to read the indicated subfile.  You may
 * seek() within this istream to your heart's content; even though it will be
 * a reference to the already-opened pfstream of the ZipArchive itself, byte 0
 * appears to be the beginning of the subfile and EOF appears to be the end of
 * the subfile.
 *
 * The returned istream will have been allocated via new; you should pass the
 * pointer to close_read_subfile() when you are finished with it to delete it
 * and release its resources.
 *
 * Any future calls to repack() or close() (or the ZipArchive destructor) will
 * invalidate all currently open subfile pointers.
 *
 * The return value will be NULL if the stream cannot be opened for some
 * reason.
 */
std::istream *ZipArchive::
open_read_subfile(int index) {
  nassertr(is_read_valid(), nullptr);
  nassertr(index >= 0 && index < (int)_subfiles.size(), nullptr);
  Subfile *subfile = _subfiles[index];

  return open_read_subfile(subfile);
}

/**
 * Closes a file opened by a previous call to open_read_subfile().  This
 * really just deletes the istream pointer, but it is recommended to use this
 * interface instead of deleting it explicitly, to help work around compiler
 * issues.
 */
void ZipArchive::
close_read_subfile(std::istream *stream) {
  if (stream != nullptr) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting the
    // stream pointer does not call the appropriate global delete function;
    // instead apparently calling the system delete function.  So we call the
    // delete function by hand instead.
#if defined(__GNUC__) && !defined(USE_MEMORY_NOWRAPPERS) && defined(REDEFINE_GLOBAL_OPERATOR_NEW)
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
bool ZipArchive::
extract_subfile(int index, const Filename &filename) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);

  filename.make_dir();

  Filename fname = filename;
  if (!filename.is_text()) {
    fname.set_binary();
  }

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
bool ZipArchive::
extract_subfile_to(int index, std::ostream &out) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);

  std::istream *in = open_read_subfile(index);
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
bool ZipArchive::
compare_subfile(int index, const Filename &filename) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);

  if (!filename.exists()) {
    express_cat.info()
      << "File is missing: " << filename << "\n";
    return false;
  }

  std::istream *in1 = open_read_subfile(index);
  if (in1 == nullptr) {
    return false;
  }

  pifstream in2;

  if (!filename.open_read(in2)) {
    express_cat.info()
      << "Cannot read " << filename << "\n";
    return false;
  }

  if (filename.is_binary()) {
    // Check the file size.
    in2.seekg(0, std::ios::end);
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
void ZipArchive::
output(std::ostream &out) const {
  out << "ZipArchive " << _filename << ", " << get_num_subfiles()
      << " subfiles.\n";
}

/**
 * Shows a list of all subfiles within the ZipArchive.
 */
void ZipArchive::
ls(std::ostream &out) const {
  int num_subfiles = get_num_subfiles();
  for (int i = 0; i < num_subfiles; i++) {
    string subfile_name = get_subfile_name(i);
    out << subfile_name << "\n";
  }
}

/**
 * Sets the string which is appended to the very end of the ZIP archive.
 * This string may not be longer than 65535 characters.
 */
void ZipArchive::
set_comment(const std::string &comment) {
  nassertv(comment.size() <= 65535);

  if (_comment != comment) {
    _comment = comment;
    _index_changed = true;
  }
}

/**
 * Fills a string with the entire contents of the indicated subfile.
 */
bool ZipArchive::
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
bool ZipArchive::
read_subfile(int index, vector_uchar &result) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  result.clear();

  // Now look up the particular Subfile we are reading.
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  Subfile *subfile = _subfiles[index];

  result.reserve(subfile->_uncompressed_length);

  bool success = true;
  if (subfile->is_compressed() || subfile->is_encrypted()) {
    // If the subfile is encrypted or compressed, we can't read it directly.
    // Fall back to the generic implementation.
    std::istream *in = open_read_subfile(index);
    if (in == nullptr) {
      return false;
    }

    success = VirtualFile::simple_read_file(in, result);
    close_read_subfile(in);

  } else {
    // But if the subfile is just a plain file, we can just read the data
    // directly from the ZipArchive, without paying the cost of an ISubStream.
    static const size_t buffer_size = 4096;
    char buffer[buffer_size];

    _read->acquire();
    if (!subfile->read_header(*_read->get_istream())) {
      _read->release();
      express_cat.error()
        << "Failed to read local header of "
        << _filename << "/" << subfile->_name << "\n";
      return false;
    }
    std::istream &read = *_read->get_istream();
    /*std::streampos data_start =*/ read.tellg();

    size_t bytes_to_go = subfile->_uncompressed_length;
    read.read(buffer, std::min(bytes_to_go, buffer_size));
    size_t read_bytes = read.gcount();

    while (read_bytes > 0) {
      result.insert(result.end(), buffer, buffer + read_bytes);

      bytes_to_go -= read_bytes;
      if (bytes_to_go == 0) {
        break;
      }

      read.read(buffer, std::min(bytes_to_go, buffer_size));
      read_bytes = read.gcount();
    }

    _read->release();
    success = (bytes_to_go == 0);
  }

  if (!success) {
    std::ostringstream message;
    message << "I/O error reading from " << get_filename() << " at "
            << get_subfile_name(index);
    nassert_raise(message.str());
    return false;
  }

  return true;
}

/**
 * Adds a newly-allocated Subfile pointer to the ZipArchive.
 */
void ZipArchive::
add_new_subfile(Subfile *subfile, int compression_level) {
  // We'll need to rewrite the index after this.
  _index_changed = true;

  std::pair<Subfiles::iterator, bool> insert_result = _subfiles.insert(subfile);
  if (!insert_result.second) {
    // Hmm, unable to insert.  There must already be a subfile by that name.
    // Add it to the _removed_subfiles list, so we can remove the old one.
    std::swap(subfile, *insert_result.first);
    _removed_subfiles.push_back(subfile);

    // Since we're removing a subfile and adding the new one at the end, we've
    // got empty space.  A repack would be good.
    _needs_repack = true;
  }
}

/**
 * This variant of open_read_subfile() is used internally only, and accepts a
 * pointer to the internal Subfile object, which is assumed to be valid and
 * written to the multifile.
 */
std::istream *ZipArchive::
open_read_subfile(Subfile *subfile) {
  // Read the header first.
  _read->acquire();
  if (!subfile->read_header(*_read->get_istream())) {
    _read->release();
    express_cat.error()
      << "Failed to read local header of "
      << _filename << "/" << subfile->_name << "\n";
    return nullptr;
  }
  std::streampos data_start = _read->get_istream()->tellg();
  _read->release();

  // Return an ISubStream object that references into the open ZipArchive
  // istream.
  nassertr(data_start != (streampos)0, nullptr);
  std::istream *stream =
    new ISubStream(_read, data_start,
                   data_start + (streampos)subfile->_data_length);

  if (subfile->is_compressed()) {
#ifndef HAVE_ZLIB
    express_cat.error()
      << "zlib not compiled in; cannot read compressed multifiles.\n";
    return nullptr;
#else  // HAVE_ZLIB
    // Oops, the subfile is compressed.  So actually, return an
    // IDecompressStream that wraps around the ISubStream.
    IDecompressStream *wrapper = new IDecompressStream(stream, true, -1, false);
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
string ZipArchive::
standardize_subfile_name(const std::string &subfile_name) const {
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
void ZipArchive::
clear_subfiles() {
  for (Subfile *subfile : _removed_subfiles) {
    delete subfile;
  }
  _removed_subfiles.clear();

  for (Subfile *subfile : _subfiles) {
    delete subfile;
  }
  _subfiles.clear();
}

/**
 * Reads the ZipArchive header and index.  Returns true if successful, false if
 * the ZipArchive is not valid.
 *
 * Assumes that the get pointer is at the end of the file.
 */
bool ZipArchive::
read_index() {
  nassertr(_read != nullptr, false);

  // We acquire the IStreamWrapper lock for the duration of this method.
  _read->acquire();
  std::istream *read = _read->get_istream();

  // ZIP files need to be read from the end.
  read->seekg(-2, std::ios::end);
  if (read->fail()) {
    express_cat.info()
      << "Unable to seek ZIP archive " << _filename << ".\n";
    _read->release();
    close();
    return false;
  }

  std::streampos fpos = read->tellg();
  _file_end = fpos + (std::streamoff)2;

  uint64_t cdir_entries = 0;
  uint64_t cdir_offset = 0;
  uint32_t comment_length = 0;
  std::streampos eocd_offset = 0;
  bool found = false;

  // Seek backwards until we have found the the end-of-directory record.
  StreamReader reader(read, false);
  while (comment_length <= 0xffff && fpos >= 20) {
    //nassertr(fpos == read->tellg(), false);

    if (reader.get_uint16() == comment_length) {
      // This field references the distance to the end of the .zip file, so it
      // could be the comment length field at the end of the record.  Skip to the
      // beginning of the record to see if the signature matches.
      read->seekg(-22, std::ios::cur);

      if (reader.get_uint32() == 0x06054b50) {
        // Yes, got it.
        eocd_offset = read->tellg() - (std::streamoff)4;
        reader.skip_bytes(6);
        cdir_entries = reader.get_uint16();
        /*cdir_size = */reader.get_uint32();
        cdir_offset = reader.get_uint32();
        if (comment_length > 0) {
          _comment = reader.get_fixed_string(comment_length);
        } else {
          _comment.clear();
        }
        found = true;
        break;
      }

      // No, skip back to where we were and continue scanning for comments.
      comment_length += 2;
      read->seekg(22 - 4 - 4, std::ios::cur);
      fpos -= 2;
      continue;
    }

    comment_length += 2;
    read->seekg(-4, std::ios::cur);
    fpos -= 2;
  }

  if (!found) {
    express_cat.info()
      << "Unable to find end-of-directory record in ZIP archive " << _filename << ".\n";
    _read->release();
    close();
    return false;
  }

  // Now look for a ZIP64 end-of-central-directory locator.
  if (eocd_offset >= 20) {
    uint64_t eocd64_offset = 0;
    read->seekg(eocd_offset - (std::streamoff)20);
    if (reader.get_uint32() == 0x07064b50) {
      reader.skip_bytes(4); // disk no
      eocd64_offset = reader.get_uint64();
      reader.skip_bytes(4); // disk count

      read->seekg(eocd64_offset);
      if (reader.get_uint32() == 0x06064b50) {
        reader.skip_bytes(20);
        cdir_entries = reader.get_uint64();
        /*cdir_size = */reader.get_uint64();
        cdir_offset = reader.get_uint64();
      } else {
        express_cat.info()
          << "Unable to read ZIP64 end-of-directory record in ZIP archive "
          << _filename << ".\n";
      }
    }
  }

  _index_start = cdir_offset;

  // Find the central directory.
  read->seekg((std::streampos)cdir_offset);
  if (read->fail()) {
    express_cat.info()
      << "Unable to locate central directory in ZIP archive " << _filename << ".\n";
    _read->release();
    close();
    return false;
  }

  _record_timestamp = false;

  for (size_t i = 0; i < cdir_entries; ++i) {
    Subfile *subfile = new Subfile;
    if (!subfile->read_index(*read)) {
      express_cat.info()
        << "Failed to read central directory for " << _filename << ".\n";
      _read->release();
      close();
      return false;
    }

    if (subfile->_timestamp != dos_epoch) {
      // If all subfiles have the timestamp set to the DOS epoch, we apparently
      // don't care about preserving timestamps.
      _record_timestamp = true;
    }
    _subfiles.push_back(subfile);
  }

  // Sort the subfiles.
  size_t before_size = _subfiles.size();
  _subfiles.sort();
  size_t after_size = _subfiles.size();

  // If these don't match, the same filename appeared twice in the index,
  // which shouldn't be possible.
  nassertr(before_size == after_size, false);

  _read->release();
  return true;
}

/**
 * Writes the index of the ZIP archive at the current put position.
 */
bool ZipArchive::
write_index(std::ostream &write, std::streampos &fpos) {
  nassertr(write.tellp() == fpos, false);

  _index_start = fpos;

  for (Subfile *subfile : _subfiles) {
    if (!subfile->write_index(write, fpos)) {
      express_cat.info()
        << "Failed to write central directory entry for "
        << _filename << "/" << subfile->_name << ".\n";
      _read->release();
      close();
      return false;
    }
  }

  size_t cdir_entries = _subfiles.size();
  std::streamoff cdir_size = fpos - _index_start;

  StreamWriter writer(write);

  if (_index_start >= 0xffffffff ||
      cdir_size >= 0xffffffff ||
      cdir_entries >= 0xffff) {
    // Write a ZIP64 end-of-central-directory record.
    writer.add_uint32(0x06064b50);
    writer.add_uint64(44); // size of the rest of the record (w/o first 12 bytes)
    writer.add_uint16(45); // version number that produced this file
    writer.add_uint16(45); // version number needed to read this file
    writer.add_uint32(0);
    writer.add_uint32(0);
    writer.add_uint64(cdir_entries);
    writer.add_uint64(cdir_entries);
    writer.add_uint64(cdir_size);
    writer.add_uint64(_index_start);
    nassertr(write.tellp() == fpos + std::streamoff(12 + 44), false);

    // And write the ZIP64 end-of-central-directory-record locator.
    writer.add_uint32(0x07064b50);
    writer.add_uint32(0);
    writer.add_uint64(fpos);
    writer.add_uint32(1); // number of disks
  }

  // Write the end of central directory record.
  writer.add_uint32(0x06054b50);
  writer.add_uint16(0);
  writer.add_uint16(0);
  writer.add_uint16(std::min((size_t)0xffffu, cdir_entries));
  writer.add_uint16(std::min((size_t)0xffffu, cdir_entries));
  writer.add_uint32(std::min((std::streamoff)0xffffffffu, cdir_size));
  writer.add_uint32(std::min((std::streampos)0xffffffffu, _index_start));
  writer.add_uint16(_comment.size());
  writer.append_data(_comment);

  if (write.fail()) {
    express_cat.warning()
      << "Unable to write central directory for " << _filename << ".\n";
    close();
    return false;
  }

  fpos = write.tellp();
  if (fpos < _file_end) {
    // We didn't hit the end of the file writing the index.  This is a problem
    // because readers start looking for the EOCD record at the end of the file.
    // We'll have to shift the whole index forwards.  Unfortunately it's hard to
    // anticipate having to do this ahead of time.
    fpos = _index_start + (_file_end - fpos);
    _needs_repack = true;
    write.seekp(fpos);
    return write_index(write, fpos);
  }

  _file_end = fpos;
  return true;
}

/**
 * Creates a new subfile record.
 */
ZipArchive::Subfile::
Subfile(const std::string &name, int compression_level) :
  _name(name),
  _timestamp(dos_epoch),
  _compression_method((compression_level > 0) ? CM_deflate : CM_store)
{
  // If the name contains any non-ASCII characters, we set the UTF-8 flag.
  for (char c : name) {
    if (c & ~0x7f) {
      _flags |= SF_utf8_encoding;
      break;
    }
  }

  if (compression_level > 6) {
    _flags |= SF_deflate_best;
  } else if (compression_level > 1 && compression_level < 6) {
    _flags |= SF_deflate_fast;
  } else if (compression_level == 1) {
    _flags |= SF_deflate_fastest;
  }
}

/**
 * Reads the index record for the Subfile from the indicated istream.  Assumes
 * the istream has already been positioned to the indicated stream position,
 * fpos, the start of the index record.  Returns true on success.
 */
bool ZipArchive::Subfile::
read_index(std::istream &read) {
  StreamReader reader(read);

  if (reader.get_uint32() != 0x02014b50) {
    return false;
  }

  /*uint16_t version = */reader.get_uint8();
  _system = reader.get_uint8();
  /*uint16_t min_version = */reader.get_uint16();
  _flags = reader.get_uint16();
  _compression_method = (CompressionMethod)reader.get_uint16();
  {
    // Convert from DOS/FAT timestamp to UNIX timestamp.
    uint16_t mtime = reader.get_uint16();
    uint16_t mdate = reader.get_uint16();

    struct tm time = {};
    time.tm_sec  =  (mtime & 0b0000000000011111u) << 1;
    time.tm_min  =  (mtime & 0b0000011111100000u) >> 5;
    time.tm_hour =  (mtime & 0b1111100000000000u) >> 11;
    time.tm_mday =  (mdate & 0b0000000000011111u);
    time.tm_mon  = ((mdate & 0b0000000111100000u) >> 5) - 1;
    time.tm_year = ((mdate & 0b1111111000000000u) >> 9) + 80;
    time.tm_isdst = -1;
    _timestamp = mktime(&time);
  }
  _checksum = reader.get_uint32();
  _data_length = reader.get_uint32();
  _uncompressed_length = reader.get_uint32();
  size_t name_length = reader.get_uint16();
  size_t extra_length = reader.get_uint16();
  size_t comment_length = reader.get_uint16();
  /*size_t disk_number =*/ reader.get_uint16();
  _internal_attribs = reader.get_uint16();
  _external_attribs = reader.get_uint32();
  _header_start = (std::streampos)reader.get_uint32();

  std::string name = reader.get_fixed_string(name_length);

  // Read the extra fields, which may include a UNIX timestamp, which can be
  // specified with greater precision than a DOS timestamp.
  while (extra_length >= 4) {
    uint16_t const tag = reader.get_uint16();
    uint16_t const size = reader.get_uint16();
    if (tag == 0x0001) {
      // ZIP64 extended info.
      int size_left = size;
      if (_uncompressed_length == 0xffffffffu && size_left >= 8) {
        _uncompressed_length = reader.get_uint64();
        size_left -= 8;
      }
      if (_data_length == 0xffffffffu && size_left >= 8) {
        _data_length = reader.get_uint64();
        size_left -= 8;
      }
      if ((uint64_t)_header_start == 0xffffffffu && size_left >= 8) {
        _header_start = reader.get_uint64();
        size_left -= 8;
      }
      reader.skip_bytes(size_left);
    } else if (tag == 0x5455 && size == 5) {
      reader.skip_bytes(1);
      _timestamp = reader.get_uint32();
    } else {
      reader.skip_bytes(size);
    }
    extra_length -= 4 + size;
  }
  // Skip leftover bytes in the extra field not large enough to contain a proper
  // extra tag.  This may be the case for Android .apk files processed with
  // zipalign, which uses this for alignment.
  reader.skip_bytes(extra_length);

  std::string comment = reader.get_fixed_string(comment_length);

  if (_flags & SF_utf8_encoding) {
    _name = std::move(name);
    _comment = std::move(comment);
  } else {
    _name = TextEncoder::reencode_text(name, TextEncoder::E_cp437, TextEncoder::E_utf8);
    _comment = TextEncoder::reencode_text(comment, TextEncoder::E_cp437, TextEncoder::E_utf8);
  }

  return true;
}

/**
 * Reads the header record for the Subfile from the indicated istream.
 */
bool ZipArchive::Subfile::
read_header(std::istream &read) {
  read.seekg(_header_start);
  if (read.fail()) {
    return false;
  }

  // First, get the next stream position.  We do this separately, because if
  // it is zero, we don't get anything else.
  StreamReader reader(read);

  uint32_t signature = reader.get_uint32();
  if (signature != 0x04034b50) {
    //0x02014b50
    express_cat.warning()
      << "ZIP subfile " << _name << " header does not contain expected signature\n";
    return false;
  }

  // We skip most of the stuff in the local file header, since most of this is
  // duplicated in the central directory.
  reader.get_uint16();
  int flags = reader.get_uint16();

  if (flags != _flags) {
    express_cat.warning()
      << "ZIP subfile " << _name << " flags mismatch between file header and index record\n";
  }
  _flags = flags;

  if (reader.get_uint16() != (uint16_t)_compression_method) {
    express_cat.warning()
      << "ZIP subfile " << _name << " compression method mismatch between file header and index record\n";
    return false;
  }

  reader.get_uint32();

  if (flags & SF_data_descriptor) {
    // Ignore these fields, the real values will follow the file.
    reader.skip_bytes(4 * 3);
  } else {
    if (reader.get_uint32() != _checksum) {
      express_cat.warning()
        << "ZIP subfile " << _name << " CRC32 mismatch between file header and index record\n";
      return false;
    }

    // Compressed and uncompressed size
    uint32_t data_length = reader.get_uint32();
    uint32_t uncompressed_length = reader.get_uint32();

    if ((data_length != 0xffffffffu && data_length != _data_length) ||
        (uncompressed_length != 0xffffffffu && uncompressed_length != _uncompressed_length)) {
      express_cat.warning()
        << "ZIP subfile " << _name << " length mismatch between file header and index record\n";
      return false;
    }
  }

  size_t name_length = reader.get_uint16();
  size_t extra_length = reader.get_uint16();

  std::string name = reader.get_fixed_string(name_length);
  if ((flags & SF_utf8_encoding) == 0) {
    name = TextEncoder::reencode_text(name, TextEncoder::E_cp437, TextEncoder::E_utf8);
  }

  if (extra_length < 4) {
    reader.skip_bytes(extra_length);
  } else if (extra_length > 0) {
    for (size_t i = 0; i < extra_length;) {
      size_t length = reader.get_uint16();
      i += 4;
      reader.skip_bytes(length);
      i += length;
    }
  }

  if (name != _name) {
    express_cat.warning()
      << "Name of ZIP subfile \"" << _name << "\" in index record does not match "
         "name in file header \"" << name << "\"\n";
    return false;
  }

  //_data_start = read.tellg();
  return true;
}

/**
 * Called after read_header to verify the integrity of the data.
 * If ZLib support is not enabled, this does not verify the checksum or the
 * compression.
 */
bool ZipArchive::Subfile::
verify_data(std::istream &read) {
  //nassertr(read.tellg() == _data_start, false);

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

#ifdef HAVE_ZLIB
  unsigned long crc = crc32(0L, Z_NULL, 0);
  IDecompressStream wrapper;

  std::istream *data_stream;
  if (_compression_method == CM_store) {
    data_stream = &read;
  }
  else if (_compression_method == CM_deflate) {
    wrapper.open(&read, false, _data_length, false);
    data_stream = &wrapper;
  }
  else {
    express_cat.warning()
      << "Unable to verify ZIP subfile \"" << _name << "\": compression method "
      << (int)_compression_method << " not supported.\n";
    return false;
  }

  size_t bytes_to_go = _uncompressed_length;
  data_stream->read(buffer, std::min(bytes_to_go, buffer_size));
  size_t read_bytes = data_stream->gcount();

  while (read_bytes > 0) {
    crc = crc32(crc, (unsigned char *)buffer, read_bytes);

    bytes_to_go -= read_bytes;
    if (bytes_to_go == 0) {
      break;
    }

    data_stream->read(buffer, std::min(bytes_to_go, buffer_size));
    read_bytes = data_stream->gcount();
  }

  if (data_stream == &wrapper) {
    wrapper.close();
  }

  if (bytes_to_go > 0) {
    express_cat.warning()
      << "Reached end of compressed data verifying ZIP subfile " << _name << ".\n";
    return false;
  }

  if (crc != _checksum) {
    express_cat.warning()
      << "ZIP file member " << _name << " is corrupted.\n";
    return false;
  }
#else
  read.ignore(_data_length);

  if (read.eof()) {
    express_cat.warning()
      << "Reached EOF verifying ZIP subfile " << _name << ".\n";
    return false;
  }
#endif

  // If we are expecting a data descriptor, verify that it matches what is in
  // the index entry.
  if (_flags & SF_data_descriptor) {
    StreamReader reader(read);
    uint32_t checksum = reader.get_uint32();
    if (checksum == 0x08074b50) {
      // There is an optional data descriptor signature.
      if (_checksum == 0x08074b50) {
        // The CRC32 happens to match the data descriptor signature by accident.
        // Since the data descriptor signature is optional, we can't know for
        // sure which is which, so let's just not bother validating this.
        return true;
      }
      checksum = reader.get_uint32();
    }
    uint32_t data_length = reader.get_uint32();
    uint32_t uncompressed_length = reader.get_uint32();
    if (checksum != _checksum ||
        data_length != _data_length ||
        uncompressed_length != _uncompressed_length) {
      express_cat.warning()
        << "ZIP file member " << _name << " has mismatched data descriptor.\n";
      return false;
    }
  }

  return true;
}

/**
 * Writes the index record for the Subfile to the indicated ostream.  Assumes
 * the istream has already been positioned to the indicated stream position,
 * fpos, the start of the index record, and that this is the effective end of
 * the file.  Returns true on success.
 *
 * The _index_start member is updated by this operation.
 */
bool ZipArchive::Subfile::
write_index(std::ostream &write, streampos &fpos) {
  nassertr(write.tellp() == fpos, false);

  StreamWriter writer(write);
  writer.add_uint32(0x02014b50);

  bool zip64_length =
    (_data_length >= 0xffffffffu || _uncompressed_length >= 0xffffffffu);
  bool zip64_offset = (_header_start >= 0xffffffffu);
  size_t extra_length = zip64_length * 16 + zip64_offset * 8;

  if (zip64_length || zip64_offset) {
    writer.add_uint8(45);
    writer.add_uint8(_system);
    writer.add_uint16((_flags & SF_strong_encryption) ? 50 : 45);
  } else {
    // We just write 2.0 if it we support DEFLATE compression, 1.0 otherwise.
#ifdef HAVE_ZLIB
    writer.add_uint8(20);
    writer.add_uint8(_system);
    writer.add_uint16((_flags & SF_strong_encryption) ? 50 : (is_compressed() ? 20 : 10));
#else
    writer.add_uint8(10);
    writer.add_uint8(_system);
    writer.add_uint16((_flags & SF_strong_encryption) ? 50 : 10);
#endif
  }

  writer.add_uint16(_flags);
  writer.add_uint16((uint16_t)_compression_method);

  if (_timestamp > dos_epoch) {
    // Convert from UNIX timestamp to DOS/FAT timestamp.
#ifdef _MSC_VER
    struct tm time_data;
    struct tm *time = &time_data;
    localtime_s(time, &_timestamp);
#else
    struct tm *time = localtime(&_timestamp);
#endif
    writer.add_uint16((time->tm_sec >> 1)
                    | (time->tm_min << 5)
                    | (time->tm_hour << 11));
    writer.add_uint16(time->tm_mday
                    | ((time->tm_min + 1) << 5)
                    | ((time->tm_year - 1980) << 9));
  } else {
    // January 1, 1980
    writer.add_uint16(0);
    writer.add_uint16(33);
  }

  std::string encoded_name;
  std::string encoded_comment;
  if (_flags & SF_utf8_encoding) {
    encoded_name = _name;
    encoded_comment = _comment;
 } else {
    encoded_name = TextEncoder::reencode_text(_name, TextEncoder::E_utf8, TextEncoder::E_cp437);
    encoded_comment = TextEncoder::reencode_text(_comment, TextEncoder::E_utf8, TextEncoder::E_cp437);
  }

  writer.add_uint32(_checksum);
  if (zip64_length) {
    writer.add_uint32(0xffffffffu);
    writer.add_uint32(0xffffffffu);
  } else {
    writer.add_uint32(_data_length);
    writer.add_uint32(_uncompressed_length);
  }
  writer.add_uint16(encoded_name.size());
  writer.add_uint16(extra_length);
  writer.add_uint16(encoded_comment.size());
  writer.add_uint16(0); // disk number start
  writer.add_uint16(_internal_attribs);
  writer.add_uint32(_external_attribs);
  writer.add_uint32(zip64_offset ? 0xffffffffu : (uint32_t)_header_start);

  writer.append_data(encoded_name);

  // Write any extra fields.
  if (zip64_length || zip64_offset) {
    writer.add_uint16(0x0001);
    writer.add_uint16(zip64_length * 16 + zip64_offset * 8);
    if (zip64_length) {
      writer.add_uint64(_data_length);
      writer.add_uint64(_uncompressed_length);
    }
    if (zip64_offset) {
      writer.add_uint64(_header_start);
    }
  }

  writer.append_data(encoded_comment);

  fpos += 46 + extra_length + encoded_name.size() + encoded_comment.size();
  nassertr(write.tellp() == fpos, false);
  return !write.fail();
}

/**
 * Writes the local file header to the indicated ostream.  This immediately
 * precedes the data, so should be followed up by a call to write_data.
 *
 * Assumes that the file is currently positioned at the given fpos pointer, and
 * advances it by the amount of bytes written to the output (which may be longer
 * than the actual size of the subfile).
 */
bool ZipArchive::Subfile::
write_header(std::ostream &write, std::streampos &fpos) {
  nassertr(write.tellp() == fpos, false);

  std::string encoded_name;
  if (_flags & SF_utf8_encoding) {
    encoded_name = _name;
  } else {
    encoded_name = TextEncoder::reencode_text(_name, TextEncoder::E_utf8, TextEncoder::E_cp437);
  }

  std::streamoff header_size = 30 + encoded_name.size();

  StreamWriter writer(write);
  int modulo = (fpos + header_size) % 4;
  if (!is_compressed() && modulo != 0) {
    // Align uncompressed files to 4-byte boundary.  We don't really need to do
    // this, but it's needed when producing .apk files, and it doesn't really
    // cause harm to do it in other cases as well.
    writer.pad_bytes(4 - modulo);
    fpos += (4 - modulo);
  }

  _header_start = fpos;

  writer.add_uint32(0x04034b50);
  writer.add_uint16((_flags & SF_strong_encryption) ? 50 : (is_compressed() ? 20 : 10));

  writer.add_uint16(_flags);
  writer.add_uint16((uint16_t)_compression_method);

  if (_timestamp > 315532800) {
    // Convert from UNIX timestamp to DOS/FAT timestamp.
#ifdef _MSC_VER
    struct tm time_data;
    struct tm *time = &time_data;
    localtime_s(time, &_timestamp);
#else
    struct tm *time = localtime(&_timestamp);
#endif
    writer.add_uint16((time->tm_sec >> 1)
                    | (time->tm_min << 5)
                    | (time->tm_hour << 11));
    writer.add_uint16(time->tm_mday
                    | ((time->tm_min + 1) << 5)
                    | ((time->tm_year - 1980) << 9));
  } else {
    // January 1, 1980
    writer.add_uint16(0);
    writer.add_uint16(33);
  }

  // This flag is set if we don't yet have the checksum or lengths.  We will
  // write a data descriptor after the actual data containing these values.
  if (_flags & SF_data_descriptor) {
    writer.add_uint32(0);
    writer.add_uint32(0);
    writer.add_uint32(0);
  } else {
    writer.add_uint32(_checksum);
    writer.add_uint32(_data_length);
    writer.add_uint32(_uncompressed_length);
  }

  writer.add_uint16(encoded_name.size());
  writer.add_uint16(0); // We don't write extras for now.
  writer.append_data(encoded_name);

  fpos += header_size;
  nassertr(write.tellp() == fpos, false);
  return !write.fail();
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
 * ZipArchive in which the Subfile might already be packed.  This is used for
 * reading the contents of the Subfile during a repack() operation.
 */
bool ZipArchive::Subfile::
write_data(std::ostream &write, std::istream *read, std::streampos &fpos, int compression_level) {
  nassertr(write.tellp() == fpos, false);

  if (!is_compressed()) {
    nassertr((fpos % 4) == 0, false);
  }

  //_data_start = fpos;

  std::ostream *putter = &write;
  bool delete_putter = false;

#ifndef HAVE_ZLIB
  // Without ZLIB, we can't support compression.  The flag had better not be
  // set.
  nassertr(!is_compressed(), false);
#else  // HAVE_ZLIB
  if (is_compressed()) {
    // Write it compressed.
    putter = new OCompressStream(putter, delete_putter, compression_level, false);
    delete_putter = true;
  }
#endif  // HAVE_ZLIB

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];
  size_t total_count = 0;

#ifdef HAVE_ZLIB
  unsigned long crc = crc32(0L, Z_NULL, 0);
#endif

  read->read(buffer, buffer_size);
  size_t count = read->gcount();
  while (count != 0) {
#ifdef HAVE_ZLIB
    crc = crc32(crc, (unsigned char *)buffer, count);
#endif
    total_count += count;
    putter->write(buffer, count);
    read->read(buffer, buffer_size);
    count = read->gcount();
  }

  if (delete_putter) {
    delete putter;
  }

  if (is_compressed()) {
    std::streampos write_end = write.tellp();
    _data_length = (size_t)(write_end - fpos);
    fpos = write_end;
  } else {
    _data_length = total_count;
    fpos += total_count;
  }
  _uncompressed_length = total_count;
#ifdef HAVE_ZLIB
  _checksum = crc;
#endif

  //TODO: what if we need a zip64 data descriptor?
  if (_flags & SF_data_descriptor) {
    StreamWriter writer(write);
    writer.add_uint32(0x08074b50);
    writer.add_uint32(_checksum);
    writer.add_uint32(_data_length);
    writer.add_uint32(_uncompressed_length);
    fpos += 16;
  }

  nassertr(write.tellp() == fpos, false);
  return !write.fail();
}
