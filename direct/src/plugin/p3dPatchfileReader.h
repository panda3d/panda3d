// Filename: p3dPatchfileReader.h
// Created by:  drose (27Sep09)
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

#ifndef P3DPATCHFILEREADER_H
#define P3DPATCHFILEREADER_H

#include "p3d_plugin_common.h"
#include "p3dInstanceManager.h"  // for openssl
#include "fileSpec.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DPatchfileReader
// Description : A read-only implementation of Panda's patchfile
//               format, for applying patches.
//
//               This object assumes that the sourcefile has been
//               already validated against its md5 hash, and does not
//               validate it again.  It *does* verify that the md5
//               hash in source and target match those read in the
//               patchfile header; and it verifies the md5 hash on the
//               target after completion.
////////////////////////////////////////////////////////////////////
class P3DPatchfileReader {
public:
  P3DPatchfileReader(const string &package_dir,
                     const FileSpec &patchfile,
                     const FileSpec &source,
                     const FileSpec &target);
  ~P3DPatchfileReader();

  bool open_read();
  inline bool is_open() const;

  bool step();
  inline size_t get_bytes_written() const; 
  inline bool get_success() const;

  void close();

private:
  bool copy_bytes(istream &in, size_t copy_byte_count);
  inline unsigned int read_uint16();
  inline unsigned int read_uint32();
  inline int read_int32();

private:
  string _package_dir;
  FileSpec _patchfile;
  FileSpec _source;
  FileSpec _target;

  string _output_pathname;
  ifstream _patch_in;
  ifstream _source_in;
  ofstream _target_out;

  bool _is_open;
  size_t _target_length;
  size_t _bytes_written;
  bool _success;
};

#include "p3dPatchfileReader.I"

#endif
