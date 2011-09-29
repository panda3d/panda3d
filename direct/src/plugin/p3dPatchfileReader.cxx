// Filename: p3dPatchfileReader.cxx
// Created by:  drose (28Sep09)
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

#include "p3dPatchfileReader.h"
#include "wstring_encode.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPatchfileReader::
P3DPatchfileReader(const string &package_dir, const FileSpec &patchfile,
                   const FileSpec &source, const FileSpec &target) :
  _package_dir(package_dir),
  _patchfile(patchfile),
  _source(source),
  _target(target)
{
  _is_open = false;
  _bytes_written = 0;
  _success = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPatchfileReader::
~P3DPatchfileReader() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::open_read
//       Access: Public
//  Description: Opens the named patchfile for reading, reads the
//               header, and validates the inputs.  Returns true on
//               success, false otherwise.  If this returns false, you
//               should immediately call close(), or let this object
//               destruct.
////////////////////////////////////////////////////////////////////
bool P3DPatchfileReader::
open_read() {
  close();

  // Synthesize an output filename, in case the source and the target
  // refer to the same filename.
  _output_pathname = _target.get_pathname(_package_dir);
  _output_pathname += ".tmp";

  string patch_pathname = _patchfile.get_pathname(_package_dir);
  _patch_in.clear();
#ifdef _WIN32
  wstring patch_pathname_w;
  if (string_to_wstring(patch_pathname_w, patch_pathname)) {
    _patch_in.open(patch_pathname_w.c_str(), ios::in | ios::binary);
  }
#else // _WIN32
  _patch_in.open(patch_pathname.c_str(), ios::in | ios::binary);
#endif  // _WIN32

  string source_pathname = _source.get_pathname(_package_dir);
  _source_in.clear();
#ifdef _WIN32
  wstring source_pathname_w;
  if (string_to_wstring(source_pathname_w, source_pathname)) {
    _source_in.open(source_pathname_w.c_str(), ios::in | ios::binary);
  }
#else // _WIN32
  _source_in.open(source_pathname.c_str(), ios::in | ios::binary);
#endif  // _WIN32

  mkfile_complete(_output_pathname, nout);
  _target_out.clear();
#ifdef _WIN32
  wstring output_pathname_w;
  if (string_to_wstring(output_pathname_w, _output_pathname)) {
    _target_out.open(output_pathname_w.c_str(), ios::in | ios::binary);
  }
#else // _WIN32
  _target_out.open(_output_pathname.c_str(), ios::in | ios::binary);
#endif  // _WIN32

  _is_open = true;

  // If any of those failed to open, we fail.
  if (_patch_in.fail() || _source_in.fail() || _target_out.fail()) {
    nout << "Couldn't open patchfile source, input, and/or target.\n";
    return false;
  }

  // Read the patchfile header and validate it against the hashes we
  // were given.
  unsigned int magic_number = read_uint32();
  if (magic_number != 0xfeebfaac) {
    nout << "Not a valid patchfile: " << patch_pathname << "\n";
    return false;
  }

  unsigned int version = read_uint16();
  if (version != 2) {
    // This code only knows about patchfile version 2.  If the
    // patchfile code is updated, we have to update this code
    // accordingly.
    nout << "Unsupported patchfile version: " << version << "\n";
    return false;
  }

  size_t source_length = read_uint32();
  if (source_length != _source.get_size()) {
    nout << "Patchfile " << patch_pathname
         << " doesn't match source size.\n";
    return false;
  }
  FileSpec validate;
  validate.read_hash_stream(_patch_in);
  if (_source.compare_hash(validate) != 0) {
    nout << "Patchfile " << patch_pathname
         << " doesn't match source hash.\n";
    return false;
  }

  _target_length = read_uint32();
  if (_target_length != _target.get_size()) {
    nout << "Patchfile " << patch_pathname
         << " doesn't match target size.\n";
    return false;
  }
  validate.read_hash_stream(_patch_in);
  if (_target.compare_hash(validate) != 0) {
    nout << "Patchfile " << patch_pathname
         << " doesn't match target hash.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::step
//       Access: Public
//  Description: Performs one incremental step of the patching
//               operation.  Returns true if the operation should
//               continue and step() should be called again, false if
//               the patching is done (either successfully, or due to
//               failure).
////////////////////////////////////////////////////////////////////
bool P3DPatchfileReader::
step() {
  assert(_is_open);

  size_t add_length = read_uint16();
  if (add_length != 0) {
    // Add a number of bytes from the patchfile.
    if (!copy_bytes(_patch_in, add_length)) {
      nout << "Truncated patchfile.\n";
      return false;
    }
  }

  size_t copy_length = read_uint16();
  if (copy_length != 0) {
    // Copy a number of bytes from the original source.
    int offset = read_int32();
    _source_in.seekg(offset, ios::cur);
    if (!copy_bytes(_source_in, copy_length)) {
      nout << "Garbage in patchfile.\n";
      return false;
    }
  }

  assert(_bytes_written <= _target_length);

  // When both counts reach 0, the patchfile is done.
  if (add_length != 0 || copy_length != 0) {
    // So, we've still got more to do.
    return true;
  }

  if (_bytes_written != _target_length) {
    nout << "Patchfile wrote truncated file.\n";
    return false;
  }

  // Set the _success flag true, so close() will move the finished
  // file into place.
  _success = true;
  close();

  // Now validate the hash.
  if (!_target.full_verify(_package_dir)) {
    nout << "After patching, " << _target.get_filename()
         << " is still incorrect.\n";
    _success = false;
    return false;
  }

  // Successfully patched!  Return false to indicate completion.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::close
//       Access: Public
//  Description: Closes the previously-opened files, and moves the
//               output file into place.  This also deletes the
//               patchfile, assuming it will not be needed after it
//               has been used.
////////////////////////////////////////////////////////////////////
void P3DPatchfileReader::
close() {
  if (!_is_open) {
    return;
  }

  _is_open = false;

  _patch_in.close();
  _source_in.close();
  _target_out.close();
  
  // Delete the patchfile.
  string patch_pathname = _patchfile.get_pathname(_package_dir);
#ifdef _WIN32
  // Windows can't delete a file if it's read-only.
  chmod(patch_pathname.c_str(), 0644);
#endif
  unlink(patch_pathname.c_str());

  if (_success) {
    // Move the output file onto the target file.
    string target_pathname = _target.get_pathname(_package_dir);
#ifdef _WIN32
    chmod(target_pathname.c_str(), 0644);
#endif
    unlink(target_pathname.c_str());
    rename(_output_pathname.c_str(), target_pathname.c_str());

  } else {
    // Failure; remove the output file.
#ifdef _WIN32
    chmod(_output_pathname.c_str(), 0644);
#endif
    unlink(_output_pathname.c_str());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::copy_bytes
//       Access: Private
//  Description: Copies the indicated number of bytes from the
//               indicated stream onto the output stream.  Returns
//               true on success, false if the input stream didn't
//               have enough bytes.
////////////////////////////////////////////////////////////////////
bool P3DPatchfileReader::
copy_bytes(istream &in, size_t copy_byte_count) {
  static const size_t buffer_size = 8192;
  char buffer[buffer_size];

  streamsize read_size = min(copy_byte_count, buffer_size);
  in.read(buffer, read_size);
  streamsize count = in.gcount();
  while (count != 0) {
    _target_out.write(buffer, count);
    _bytes_written += (size_t)count;
    if (_bytes_written > _target_length) {
      nout << "Runaway patchfile.\n";
      return false;
    }
    if (count != read_size) {
      return false;
    }
    copy_byte_count -= (size_t)count;
    count = 0;
    if (copy_byte_count != 0) {
      read_size = min(copy_byte_count, buffer_size);
      in.read(buffer, read_size);
      count = in.gcount();
    }
  }

  return (copy_byte_count == 0);
}
