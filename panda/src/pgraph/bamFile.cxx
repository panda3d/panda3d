// Filename: bamFile.cxx
// Created by:  drose (02Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "bamFile.h"
#include "config_pgraph.h"

#include "bam.h"
#include "config_util.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "filename.h"
#include "config_express.h"
#include "virtualFileSystem.h"

////////////////////////////////////////////////////////////////////
//     Function: BamFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BamFile::
BamFile() {
  _reader = NULL;
  _writer = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BamFile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BamFile::
~BamFile() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: BamFile::open_read
//       Access: Public
//  Description: Attempts to open the indicated filename for reading.
//               Returns true if successful, false on error.
////////////////////////////////////////////////////////////////////
bool BamFile::
open_read(const Filename &filename, bool report_errors) {
  close();

  Filename bam_filename(filename);

  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    if (!vfs->exists(bam_filename)) {
      if (report_errors) {
        loader_cat.error() << "Could not find " << bam_filename << "\n";
      }
      return false;
    }
  } else {
    if (!bam_filename.exists()) {
      if (report_errors) {
        loader_cat.error() << "Could not find " << bam_filename << "\n";
      }
      return false;
    }
  }

  loader_cat.info() << "Reading " << bam_filename << "\n";

  if (!_din.open(bam_filename)) {
    loader_cat.error() << "Could not open " << bam_filename << "\n";
    return false;
  }

  string head;
  if (!_din.read_header(head, _bam_header.size())) {
    loader_cat.error() << bam_filename << " is not a valid BAM file.\n";
    return false;
  }

  if (head != _bam_header) {
    loader_cat.error() << bam_filename << " is not a valid BAM file.\n";
    return false;
  }

  _reader = new BamReader(&_din);
  if (!_reader->init()) {
    close();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BamFile::read_object
//       Access: Public
//  Description: Reads and returns the next object from the Bam file,
//               or NULL if the end of the file has been reached, or
//               if there is an error condition.  Use is_eof() to
//               differentiate these two cases.
//
//               The pointers returned by this method will not be
//               valid for use until resolve() is subsequently called.
////////////////////////////////////////////////////////////////////
TypedWritable *BamFile::
read_object() {
  if (_reader == (BamReader *)NULL) {
    return NULL;
  }

  return _reader->read_object();
}

////////////////////////////////////////////////////////////////////
//     Function: BamFile::is_eof
//       Access: Public
//  Description: Returns true if the reader has reached end-of-file,
//               false otherwise.  This call is only valid after a
//               call to read_object().
////////////////////////////////////////////////////////////////////
bool BamFile::
is_eof() const {
  return _reader != (BamReader *)NULL && _reader->is_eof();
}

////////////////////////////////////////////////////////////////////
//     Function: BamFile::resolve
//       Access: Public
//  Description: This must be called after one or more objects have
//               been read via calls to read_object() in order to
//               resolve all internal pointer references in the
//               objects read and make all the pointers valid.  It
//               returns true if all objects are successfully
//               resolved, or false if some have not been (in which
//               case you must call resolve() again later).
////////////////////////////////////////////////////////////////////
bool BamFile::
resolve() {
  if (_reader == (BamReader *)NULL) {
    return false;
  }

  return _reader->resolve();
}


////////////////////////////////////////////////////////////////////
//     Function: BamFile::open_write
//       Access: Public
//  Description: Attempts to open the indicated file for writing.  If
//               another file by the same name already exists, it will
//               be silently truncated.  Returns true if successful,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool BamFile::
open_write(const Filename &filename, bool) {
  close();

  loader_cat.info() << "Writing " << filename << "\n";

  filename.unlink();
  if (!_dout.open(filename)) {
    loader_cat.error() << "Unable to open " << filename << "\n";
    return false;
  }

  if (!_dout.write_header(_bam_header)) {
    loader_cat.error() << "Unable to write to " << filename << "\n";
    return false;
  }

  _writer = new BamWriter(&_dout);

  if (!_writer->init()) {
    close();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BamFile::write_object
//       Access: Public
//  Description: Writes the indicated object to the Bam file.  Returns
//               true if successful, false on error.
////////////////////////////////////////////////////////////////////
bool BamFile::
write_object(const TypedWritable *object) {
  if (_writer == (BamWriter *)NULL) {
    return false;
  }

  if (!_writer->write_object(object)) {
    close();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BamFile::close
//       Access: Public
//  Description: Closes the input or output stream.
////////////////////////////////////////////////////////////////////
void BamFile::
close() {
  if (_reader != (BamReader *)NULL) {
    resolve();
    delete _reader;
    _reader = NULL;
  }
  if (_writer != (BamWriter *)NULL) {
    delete _writer;
    _writer = NULL;
  }
  _din.close();
  _dout.close();
}


////////////////////////////////////////////////////////////////////
//     Function: BamFile::get_file_major_ver
//       Access: Public
//  Description: Returns the major version number of the file
//               currently being read, or the system current major
//               version number if no file is currently open for
//               reading.
////////////////////////////////////////////////////////////////////
int BamFile::
get_file_major_ver() {
  if (_reader == (BamReader *)NULL) {
    return _bam_major_ver;
  }
  return _reader->get_file_major_ver();
}

////////////////////////////////////////////////////////////////////
//     Function: BamFile::get_file_minor_ver
//       Access: Public
//  Description: Returns the minor version number of the file
//               currently being read, or the system current minor
//               version number if no file is currently open for
//               reading.
////////////////////////////////////////////////////////////////////
int BamFile::
get_file_minor_ver() {
  if (_reader == (BamReader *)NULL) {
    return _bam_minor_ver;
  }
  return _reader->get_file_minor_ver();
}

////////////////////////////////////////////////////////////////////
//     Function: BamFile::get_current_major_ver
//       Access: Public
//  Description: Returns the system current major version number.
//               This is the version number that will be assigned to
//               any generated Bam files.
////////////////////////////////////////////////////////////////////
int BamFile::
get_current_major_ver() {
  return _bam_major_ver;
}

////////////////////////////////////////////////////////////////////
//     Function: BamFile::get_current_minor_ver
//       Access: Public
//  Description: Returns the system current minor version number.
//               This is the version number that will be assigned to
//               any generated Bam files.
////////////////////////////////////////////////////////////////////
int BamFile::
get_current_minor_ver() {
  return _bam_minor_ver;
}
