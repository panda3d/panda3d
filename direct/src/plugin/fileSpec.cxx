// Filename: fileSpec.cxx
// Created by:  drose (29Jun09)
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

#include "fileSpec.h"
#include "wstring_encode.h"
#include "openssl/md5.h"

#include <fstream>
#include <fcntl.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#include <sys/utime.h>
#include <direct.h>
#define utimbuf _utimbuf

#else
#include <utime.h>

#endif

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FileSpec::
FileSpec() {
  _size = 0;
  _timestamp = 0;
  memset(_hash, 0, hash_size);
  _got_hash = false;
  _actual_file = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FileSpec::
FileSpec(const FileSpec &copy) :
  _filename(copy._filename),
  _size(copy._size),
  _timestamp(copy._timestamp),
  _got_hash(copy._got_hash)
{
  memcpy(_hash, copy._hash, hash_size);
  _actual_file = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FileSpec::
operator = (const FileSpec &copy) {
  _filename = copy._filename;
  _size = copy._size;
  _timestamp = copy._timestamp;
  memcpy(_hash, copy._hash, hash_size);
  _got_hash = copy._got_hash;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FileSpec::
~FileSpec() {
  if (_actual_file != NULL) {
    delete _actual_file;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::load_xml
//       Access: Public
//  Description: Reads the data from the indicated XML file.
////////////////////////////////////////////////////////////////////
void FileSpec::
load_xml(TiXmlElement *xelement) {
  const char *filename = xelement->Attribute("filename");
  if (filename != NULL) {
    _filename = filename;
  }

  const char *size = xelement->Attribute("size");
  if (size != NULL) {
    char *endptr;
    _size = strtoul(size, &endptr, 10);
  }

  const char *timestamp = xelement->Attribute("timestamp");
  if (timestamp != NULL) {
    char *endptr;
    _timestamp = strtoul(timestamp, &endptr, 10);
  }

  _got_hash = false;
  const char *hash = xelement->Attribute("hash");
  if (hash != NULL && strlen(hash) == (hash_size * 2)) {
    // Decode the hex hash string.
    _got_hash = decode_hex(_hash, hash, hash_size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::store_xml
//       Access: Public
//  Description: Stores the data to the indicated XML file.
////////////////////////////////////////////////////////////////////
void FileSpec::
store_xml(TiXmlElement *xelement) {
  if (!_filename.empty()) {
    xelement->SetAttribute("filename", _filename);
  }
  if (_size != 0) {
    xelement->SetAttribute("size", _size);
  }
  if (_timestamp != 0) {
    xelement->SetAttribute("timestamp", (int)_timestamp);
  }
  if (_got_hash) {
    char hash[hash_size * 2 + 1];
    encode_hex(hash, _hash, hash_size);
    hash[hash_size * 2] = '\0';
    xelement->SetAttribute("hash", hash);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::quick_verify
//       Access: Public
//  Description: Performs a quick test to ensure the file has not been
//               modified.  This test is vulnerable to people
//               maliciously attempting to fool the program (by
//               setting datestamps etc.).
//
//               Returns true if it is intact, false if it needs to be
//               redownloaded.
////////////////////////////////////////////////////////////////////
bool FileSpec::
quick_verify(const string &package_dir) {
  return quick_verify_pathname(get_pathname(package_dir));
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::quick_verify_pathname
//       Access: Public
//  Description: Works like quick_verify(), above, with an explicit
//               pathname.  Useful for verifying the copy of a file in
//               a temporary location.
////////////////////////////////////////////////////////////////////
bool FileSpec::
quick_verify_pathname(const string &pathname) {
  if (_actual_file != NULL) {
    delete _actual_file;
    _actual_file = NULL;
  }

  int result = 1;
#ifdef _WIN32
  struct _stat st;
  wstring pathname_w;
  if (string_to_wstring(pathname_w, pathname)) {
    result = _wstat(pathname_w.c_str(), &st);
  }
#else // _WIN32
  struct stat st;
  result = stat(pathname.c_str(), &st);
#endif  // _WIN32

  if (result != 0) {
    //cerr << "file not found: " << _filename << "\n";
    return false;
  }

  if (st.st_size != _size) {
    // If the size is wrong, the file fails.
    //cerr << "size wrong: " << _filename << "\n";
    return false;
  }

  if (st.st_mtime == _timestamp) {
    // If the size is right and the timestamp is right, the file passes.
    //cerr << "file ok: " << _filename << "\n";
    return true;
  }

  //cerr << "modification time wrong: " << _filename << "\n";

  // If the size is right but the timestamp is wrong, the file
  // soft-fails.  We follow this up with a hash check.
  if (!priv_check_hash(pathname, &st)) {
    // Hard fail, the hash is wrong.
    //cerr << "hash check wrong: " << _filename << "\n";
    return false;
  }

  //cerr << "hash check ok: " << _filename << "\n";

  // The hash is OK after all.  Change the file's timestamp back to
  // what we expect it to be, so we can quick-verify it successfully
  // next time.
  utimbuf utb;
  utb.actime = st.st_atime;
  utb.modtime = _timestamp;

#ifdef _WIN32
  _wutime(pathname_w.c_str(), &utb);
#else // _WIN32
  utime(pathname.c_str(), &utb);
#endif  // _WIN32

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::full_verify
//       Access: Public
//  Description: Performs a more thorough test to ensure the file has
//               not been modified.  This test is less vulnerable to
//               malicious attacks, since it reads and verifies the
//               entire file.
//
//               Returns true if it is intact, false if it needs to be
//               redownloaded.
////////////////////////////////////////////////////////////////////
bool FileSpec::
full_verify(const string &package_dir) {
  if (_actual_file != NULL) {
    delete _actual_file;
    _actual_file = NULL;
  }

  string pathname = get_pathname(package_dir);
  int result = 1;
#ifdef _WIN32
  struct _stat st;
  wstring pathname_w;
  if (string_to_wstring(pathname_w, pathname)) {
    result = _wstat(pathname_w.c_str(), &st);
  }
#else // _WIN32
  struct stat st;
  result = stat(pathname.c_str(), &st);
#endif  // _WIN32

  if (result != 0) {
    //cerr << "file not found: " << _filename << "\n";
    return false;
  }

  if (st.st_size != _size) {
    // If the size is wrong, the file fails.
    //cerr << "size wrong: " << _filename << "\n";
    return false;
  }

  if (!priv_check_hash(pathname, &st)) {
    // Hard fail, the hash is wrong.
    //cerr << "hash check wrong: " << _filename << "\n";
    return false;
  }

  //cerr << "hash check ok: " << _filename << "\n";

  // The hash is OK.  If the timestamp is wrong, change it back to
  // what we expect it to be, so we can quick-verify it successfully
  // next time.

  if (st.st_mtime != _timestamp) {
    utimbuf utb;
    utb.actime = st.st_atime;
    utb.modtime = _timestamp;
#ifdef _WIN32
    _wutime(pathname_w.c_str(), &utb);
#else // _WIN32
    utime(pathname.c_str(), &utb);
#endif  // _WIN32
  }
    
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::force_get_actual_file
//       Access: Public
//  Description: Returns a FileSpec that represents the actual data
//               read on disk.  This will read the disk to determine
//               the data if necessary.
////////////////////////////////////////////////////////////////////
const FileSpec *FileSpec::
force_get_actual_file(const string &pathname) {
  if (_actual_file == NULL) {
#ifdef _WIN32
    struct _stat st;
    wstring pathname_w;
    if (string_to_wstring(pathname_w, pathname)) {
      _wstat(pathname_w.c_str(), &st);
    }
#else // _WIN32
    struct stat st;
    stat(pathname.c_str(), &st);
#endif  // _WIN32

    priv_check_hash(pathname, &st);
  }

  return _actual_file;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::check_hash
//       Access: Public
//  Description: Returns true if the file has the expected md5 hash,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool FileSpec::
check_hash(const string &pathname) const {
  FileSpec other;
  if (!other.read_hash(pathname)) {
    return false;
  }

  return (memcmp(_hash, other._hash, hash_size) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::read_hash
//       Access: Public
//  Description: Computes the hash from the indicated pathname and
//               stores it within the FileSpec.
////////////////////////////////////////////////////////////////////
bool FileSpec::
read_hash(const string &pathname) {
  memset(_hash, 0, hash_size);
  _got_hash = false;

  ifstream stream;
#ifdef _WIN32
  wstring pathname_w;
  if (string_to_wstring(pathname_w, pathname)) {
    stream.open(pathname_w.c_str(), ios::in | ios::binary);
  }
#else // _WIN32
  stream.open(pathname.c_str(), ios::in | ios::binary);
#endif  // _WIN32
  
  if (!stream) {
    //cerr << "unable to read " << pathname << "\n";
    return false;
  }

  MD5_CTX ctx;
  MD5_Init(&ctx);

  static const int buffer_size = 4096;
  char buffer[buffer_size];

  stream.read(buffer, buffer_size);
  size_t count = stream.gcount();
  while (count != 0) {
    MD5_Update(&ctx, buffer, count);
    stream.read(buffer, buffer_size);
    count = stream.gcount();
  }

  MD5_Final(_hash, &ctx);
  _got_hash = true;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::read_hash_stream
//       Access: Public
//  Description: Reads the hash from the next 16 bytes on the
//               indicated istream, in the same unusual order observed
//               by Panda's HashVal::read_stream() method.
////////////////////////////////////////////////////////////////////
bool FileSpec::
read_hash_stream(istream &in) {
  for (int i = 0; i < hash_size; i += 4) {
    unsigned int a = in.get();
    unsigned int b = in.get();
    unsigned int c = in.get();
    unsigned int d = in.get();
    _hash[i + 0] = d;
    _hash[i + 1] = c;
    _hash[i + 2] = b;
    _hash[i + 3] = a;
  }

  return !in.fail();
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::compare_hash
//       Access: Public
//  Description: Returns < 0 if this hash sorts before the other
//               hash, > 0 if it sorts after, 0 if they are the same.
////////////////////////////////////////////////////////////////////
int FileSpec::
compare_hash(const FileSpec &other) const {
  return memcmp(_hash, other._hash, hash_size);
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::write
//       Access: Public
//  Description: Describes the data in the FileSpec.
////////////////////////////////////////////////////////////////////
void FileSpec::
write(ostream &out) const {
  out << "filename: " << _filename << ", " << _size << " bytes, "
      << asctime(localtime(&_timestamp));
  // asctime includes a newline.
  out << "hash: ";
  stream_hex(out, _hash, hash_size);
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::output_hash
//       Access: Public
//  Description: Writes just the hash code.
////////////////////////////////////////////////////////////////////
void FileSpec::
output_hash(ostream &out) const {
  stream_hex(out, _hash, hash_size);
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::priv_check_hash
//       Access: Private
//  Description: Returns true if the file has the expected md5 hash,
//               false otherwise.  Updates _actual_file with the data
//               read from disk, including the hash, for future
//               reference.
//
//               The parameter stp is a pointer to a stat structure.
//               It's declared as a void * to get around issues with
//               the nonstandard declaration of this structure in
//               Windows.
////////////////////////////////////////////////////////////////////
bool FileSpec::
priv_check_hash(const string &pathname, void *stp) {
  const struct stat &st = *(const struct stat *)stp;
  assert(_actual_file == NULL);
  _actual_file = new FileSpec;
  _actual_file->_filename = pathname;
  _actual_file->_size = st.st_size;
  _actual_file->_timestamp = st.st_mtime;

  if (!_actual_file->read_hash(pathname)) {
    return false;
  }

  return (memcmp(_hash, _actual_file->_hash, hash_size) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::decode_hex
//       Access: Private, Static
//  Description: Decodes the hex string in source into the character
//               array in dest.  dest must have has least size bytes;
//               source must have size * 2 bytes.
//
//               Returns true on success, false if there was a non-hex
//               digit in the string.
////////////////////////////////////////////////////////////////////
bool FileSpec::
decode_hex(unsigned char *dest, const char *source, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    int high = decode_hexdigit(source[i * 2]);
    int low = decode_hexdigit(source[i * 2 + 1]);
    if (high < 0 || low < 0) {
      return false;
    }
    dest[i] = (high << 4) | low;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::encode_hex
//       Access: Private, Static
//  Description: Encodes a character array into a hex string for
//               output.  dest must have at least size * 2 bytes;
//               source must have size bytes.  The result is not
//               null-terminated.
////////////////////////////////////////////////////////////////////
void FileSpec::
encode_hex(char *dest, const unsigned char *source, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    int high = (source[i] >> 4) & 0xf;
    int low = source[i] & 0xf;
    dest[2 * i] = encode_hexdigit(high);
    dest[2 * i + 1] = encode_hexdigit(low);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FileSpec::stream_hex
//       Access: Private, Static
//  Description: Writes the indicated buffer as a string of hex
//               characters to the given ostream.
////////////////////////////////////////////////////////////////////
void FileSpec::
stream_hex(ostream &out, const unsigned char *source, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    int high = (source[i] >> 4) & 0xf;
    int low = source[i] & 0xf;
    out.put(encode_hexdigit(high));
    out.put(encode_hexdigit(low));
  }
}
