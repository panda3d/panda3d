// Filename: p3dMultifileReader.cxx
// Created by:  drose (15Jun09)
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

#include "p3dMultifileReader.h"
#include "p3dPackage.h"
#include "mkdir_complete.h"
#include "wstring_encode.h"

#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#endif

// This sequence of bytes begins each Multifile to identify it as a
// Multifile.
const char P3DMultifileReader::_header[] = "pmf\0\n\r";
const size_t P3DMultifileReader::_header_size = 6;

const int P3DMultifileReader::_current_major_ver = 1;
const int P3DMultifileReader::_current_minor_ver = 1;

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DMultifileReader::
P3DMultifileReader() {
  _is_open = false;
  _read_offset = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::open_read
//       Access: Public
//  Description: Opens the indicated file for reading.  Returns true
//               on success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DMultifileReader::
open_read(const string &pathname, const int &offset) {
  if (_is_open) {
    close();
  }
 
  _read_offset = offset;
  if (!read_header(pathname)) {
    return false;
  }

  _is_open = true;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::close
//       Access: Public
//  Description: Closes the previously-opened file.
////////////////////////////////////////////////////////////////////
void P3DMultifileReader::
close() {
  _in.close();
  _is_open = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::extract_all
//       Access: Public
//  Description: Reads the multifile, and extracts all the expected
//               extractable components within it to the indicated
//               directory.  Returns true on success, false on
//               failure.
//
//               Upates the "step" object with the progress through
//               this operation.
////////////////////////////////////////////////////////////////////
bool P3DMultifileReader::
extract_all(const string &to_dir, P3DPackage *package, 
            P3DPackage::InstallStepThreaded *step) {
  assert(_is_open);
  if (_in.fail()) {
    return false;
  }

  // Now walk through all of the files, and extract only the ones we
  // expect to encounter.
  Subfiles::iterator si;
  for (si = _subfiles.begin(); si != _subfiles.end(); ++si) {
    const Subfile &s = (*si);
    FileSpec file;
    if (package != NULL && !package->is_extractable(file, s._filename)) {
      continue;
    }

    string output_pathname = to_dir + "/" + s._filename;
    if (!mkfile_complete(output_pathname, nout)) {
      return false;
    }

    ofstream out;
#ifdef _WIN32
    wstring output_pathname_w;
    if (string_to_wstring(output_pathname_w, output_pathname)) {
      out.open(output_pathname_w.c_str(), ios::out | ios::binary);
    }
#else // _WIN32
    out.open(output_pathname.c_str(), ios::out | ios::binary);
#endif  // _WIN32
    if (!out) {
      nout << "Unable to write to " << output_pathname << "\n";
      return false;
    }

    if (!extract_subfile(out, s)) {
      return false;
    }
    out.close();

    // Check that the file was extracted correctly (and also set the
    // correct timestamp).
    if (!file.full_verify(to_dir)) {
      nout << "After extracting, " << s._filename << " is still incorrect.\n";
      return false;
    }

    // Be sure to set execute permissions on the file, in case it's a
    // program or something.
    chmod(output_pathname.c_str(), 0555);

    if (step != NULL && package != NULL) {
      step->thread_add_bytes_done(s._data_length);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::extract_one
//       Access: Public
//  Description: Reads the multifile, and extracts only the named
//               component to the indicated stream.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DMultifileReader::
extract_one(ostream &out, const string &filename) {
  assert(_is_open);
  if (_in.fail()) {
    return false;
  }

  // Look for the named component.
  Subfiles::iterator si;
  for (si = _subfiles.begin(); si != _subfiles.end(); ++si) {
    const Subfile &s = (*si);
    if (s._filename == filename) {
      return extract_subfile(out, s);
    }
  }

  nout << "Could not extract " << filename << ": not found.\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::get_num_signatures
//       Access: Published
//  Description: Returns the number of matching signatures found on
//               the Multifile.  These signatures may be iterated via
//               get_signature() and related methods.
//
//               A signature on this list is guaranteed to match the
//               Multifile contents, proving that the Multifile has
//               been unmodified since the signature was applied.
//               However, this does not guarantee that the certificate
//               itself is actually from who it says it is from; only
//               that it matches the Multifile contents.  See
//               validate_signature_certificate() to authenticate a
//               particular certificate.
////////////////////////////////////////////////////////////////////
int P3DMultifileReader::
get_num_signatures() const {
  if (_is_open) {
    ((P3DMultifileReader *)this)->check_signatures();
  }

  return _signatures.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::get_signature
//       Access: Published
//  Description: Returns the nth signature found on the Multifile.
//               See the comments in get_num_signatures().
////////////////////////////////////////////////////////////////////
const P3DMultifileReader::CertChain &P3DMultifileReader::
get_signature(int n) const {
  static CertChain error_chain;
  assert(n >= 0 && n < (int)_signatures.size());
  return _signatures[n];
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::read_header
//       Access: Private
//  Description: Opens the named multifile and reads the header
//               information and index, returning true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool P3DMultifileReader::
read_header(const string &pathname) {
  assert(!_is_open);
  _subfiles.clear();
  _cert_special.clear();
  _signatures.clear();

#ifdef _WIN32
  wstring pathname_w;
  if (string_to_wstring(pathname_w, pathname)) {
    _in.open(pathname_w.c_str(), ios::in | ios::binary);
  }
#else // _WIN32
  _in.open(pathname.c_str(), ios::in | ios::binary);
#endif  // _WIN32
  if (!_in) {
    nout << "Couldn't open " << pathname << "\n";
    return false;
  }

  char this_header[_header_size];
  _in.seekg(_read_offset);

  // Here's a special case: if the multifile begins with a hash
  // character, then we continue reading and discarding lines of ASCII
  // text, until we come across a nonempty line that does not begin
  // with a hash character.  This allows a P3D application (which is a
  // multifile) to be run directly on the command line on Unix-based
  // systems.
  int ch = _in.get();

  if (ch == '#') {
    while (ch != EOF && ch == '#') {
      // Skip to the end of the line.
      while (ch != EOF && ch != '\n') {
        ch = _in.get();
      }
      // Skip to the first non-whitespace character of the line.
      while (ch != EOF && (isspace(ch) || ch == '\r')) {
        ch = _in.get();
      }
    }
  }

  // Now read the actual Multifile header.
  this_header[0] = ch;
  _in.read(this_header + 1, _header_size - 1);
  if (_in.fail() || _in.gcount() != (unsigned)(_header_size - 1)) {
    nout << "Unable to read Multifile header: " << pathname << "\n";
    return false;
  }

  if (memcmp(this_header, _header, _header_size) != 0) {
    nout << "Failed header check: " << pathname << "\n";
    return false;
  }

  unsigned int major = read_uint16();
  unsigned int minor = read_uint16();
  if (major != _current_major_ver || minor != _current_minor_ver) {
    nout << "Incompatible multifile version: " << pathname << "\n";
    return false;
  }

  unsigned int scale = read_uint32();
  if (scale != 1) {
    nout << "Unsupported scale factor in " << pathname << "\n";
    return false;
  }

  // We don't care about the overall timestamp.
  read_uint32();

  if (!read_index()) {
    nout << "Error reading multifile index\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::read_index
//       Access: Private
//  Description: Assuming the file stream is positioned at the first
//               record, reads all of the records into the _subfiles
//               list.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DMultifileReader::
read_index() {
  _last_data_byte = 0;
  unsigned int next_entry = read_uint32();
  if (!_in) {
    return false;
  }
  while (next_entry != 0) {
    Subfile s;
    s._index_start = (size_t)_in.tellg() - _read_offset;
    s._index_length = 0;
    s._data_start = read_uint32();
    s._data_length = read_uint32();
    unsigned int flags = read_uint16();
    if ((flags & (SF_compressed | SF_encrypted)) != 0) {
      // Skip over the uncompressed length.
      read_uint32();
    }
      
    s._timestamp = read_uint32();
    size_t name_length = read_uint16();
    char *buffer = new char[name_length];
    _in.read(buffer, name_length);

    // The filenames are xored with 0xff just for fun.
    for (size_t ni = 0; ni < name_length; ++ni) {
      buffer[ni] ^= 0xff;
    }

    s._filename = string(buffer, name_length);
    delete[] buffer;

    s._index_length = (size_t)_in.tellg() - s._index_start - _read_offset;

    if (flags & SF_signature) {
      // A subfile with this bit set is a signature.
      _cert_special.push_back(s);
    } else {
      // Otherwise, it's a regular file.
      _last_data_byte = max(_last_data_byte, s.get_last_byte_pos());

      if ((flags & SF_ignore) == 0) {
        // We can only support subfiles with none of SF_ignore set.
        _subfiles.push_back(s);
      }
    }

    _in.seekg(next_entry + _read_offset);
    next_entry = read_uint32();
    if (!_in) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::extract_subfile
//       Access: Private
//  Description: Extracts the indicated subfile and writes it to the
//               indicated stream.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool P3DMultifileReader::
extract_subfile(ostream &out, const Subfile &s) {
  _in.seekg(s._data_start + _read_offset);

  static const streamsize buffer_size = 4096;
  char buffer[buffer_size];
  
  streamsize remaining_data = s._data_length;
  _in.read(buffer, min(buffer_size, remaining_data));
  streamsize count = _in.gcount();
  while (count != 0) {
    remaining_data -= count;
    out.write(buffer, count);
    _in.read(buffer, min(buffer_size, remaining_data));
    count = _in.gcount();
  }
  
  if (remaining_data != 0) {
    nout << "Unable to extract " << s._filename << "\n";
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::check_signatures
//       Access: Private
//  Description: Walks through the list of _cert_special entries in
//               the Multifile, moving any valid signatures found to
//               _signatures.  After this call, _cert_special will be
//               empty.
//
//               This does not check the validity of the certificates
//               themselves.  It only checks that they correctly sign
//               the Multifile contents.
////////////////////////////////////////////////////////////////////
void P3DMultifileReader::
check_signatures() {
  Subfiles::iterator pi;

  for (pi = _cert_special.begin(); pi != _cert_special.end(); ++pi) {
    Subfile *subfile = &(*pi);

    // Extract the signature data and certificate separately.
    _in.seekg(subfile->_data_start + _read_offset);
    size_t sig_size = read_uint32();
    char *sig = new char[sig_size];
    _in.read(sig, sig_size);
    if (_in.gcount() != sig_size) {
      nout << "read failure\n";
      delete[] sig;
      return;
    }

    size_t num_certs = read_uint32();

    // Read the remaining buffer of certificate data.
    size_t bytes_read = (size_t)_in.tellg() - subfile->_data_start - _read_offset;
    size_t buffer_size = subfile->_data_length - bytes_read;
    char *buffer = new char[buffer_size];
    _in.read(buffer, buffer_size);
    if (_in.gcount() != buffer_size) {
      nout << "read failure\n";
      delete[] sig;
      delete[] buffer;
      return;
    }

    // Now convert each of the certificates to an X509 object, and
    // store it in our CertChain.
    CertChain chain;
    EVP_PKEY *pkey = NULL;
    if (buffer_size > 0) {
#if OPENSSL_VERSION_NUMBER >= 0x00908000L
      // Beginning in 0.9.8, d2i_X509() accepted a const unsigned char **.
      const unsigned char *bp, *bp_end;
#else
      // Prior to 0.9.8, d2i_X509() accepted an unsigned char **.
      unsigned char *bp, *bp_end;
#endif
      bp = (unsigned char *)&buffer[0];
      bp_end = bp + buffer_size;
      X509 *x509 = d2i_X509(NULL, &bp, bp_end - bp);
      while (num_certs > 0 && x509 != NULL) {
        chain.push_back(CertRecord(x509));
        --num_certs;
        x509 = d2i_X509(NULL, &bp, bp_end - bp);
      }
      if (num_certs != 0 || x509 != NULL) {
        nout << "Extra data in signature record.\n";
      }
    }

    delete[] buffer;
    
    if (!chain.empty()) {
      pkey = X509_get_pubkey(chain[0]._cert);
    }
    
    if (pkey != NULL) {
      EVP_MD_CTX *md_ctx;
#if OPENSSL_VERSION_NUMBER >= 0x00907000L
      md_ctx = EVP_MD_CTX_create();
#else
      md_ctx = new EVP_MD_CTX;
#endif
      EVP_VerifyInit(md_ctx, EVP_sha1());

      // Read and hash the multifile contents, but only up till
      // _last_data_byte.
      _in.seekg(_read_offset);
      streampos bytes_remaining = (streampos)_last_data_byte;
      static const streamsize buffer_size = 4096;
      char buffer[buffer_size];
      _in.read(buffer, min((streampos)buffer_size, bytes_remaining));
      streamsize count = _in.gcount();
      while (count != 0) {
        assert(count <= buffer_size);
        EVP_VerifyUpdate(md_ctx, buffer, (size_t)count);
        bytes_remaining -= count;
        _in.read(buffer, min((streampos)buffer_size, bytes_remaining));
        count = _in.gcount();
      }
      assert(bytes_remaining == (streampos)0);
      
      // Now check that the signature matches the hash.
      int verify_result = 
        EVP_VerifyFinal(md_ctx, (unsigned char *)sig, 
                        sig_size, pkey);
      if (verify_result == 1) {
        // The signature matches; save the certificate and its chain.
        _signatures.push_back(chain);
      }
    }

    delete[] sig;
  }

  _cert_special.clear();
}
