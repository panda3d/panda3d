/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamFile.cxx
 * @author drose
 * @date 2000-07-02
 */

#include "bamFile.h"
#include "config_pgraph.h"

#include "bam.h"
#include "bamCacheRecord.h"
#include "config_putil.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "filename.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "dcast.h"

using std::string;

/**
 *
 */
BamFile::
BamFile() {
  _reader = nullptr;
  _writer = nullptr;
}

/**
 *
 */
BamFile::
~BamFile() {
  close();
}

/**
 * Attempts to open the indicated filename for reading.  Returns true if
 * successful, false on error.
 */
bool BamFile::
open_read(const Filename &bam_filename, bool report_errors) {
  close();

  if (!_din.open(bam_filename)) {
    return false;
  }

  return continue_open_read(bam_filename, report_errors);
}

/**
 * Attempts to open the indicated stream for reading.  The filename is just
 * for information purposes only.  Returns true if successful, false on error.
 */
bool BamFile::
open_read(std::istream &in, const string &bam_filename, bool report_errors) {
  close();

  if (!_din.open(in)) {
    return false;
  }

  return continue_open_read(bam_filename, report_errors);
}

/**
 * Reads and returns the next object from the Bam file, or NULL if the end of
 * the file has been reached, or if there is an error condition.  Use is_eof()
 * to differentiate these two cases.
 *
 * The pointers returned by this method will not be valid for use until
 * resolve() is subsequently called.
 */
TypedWritable *BamFile::
read_object() {
  if (_reader == nullptr) {
    return nullptr;
  }

  return _reader->read_object();
}

/**
 * Returns true if the reader has reached end-of-file, false otherwise.  This
 * call is only valid after a call to read_object().
 */
bool BamFile::
is_eof() const {
  return _reader != nullptr && _reader->is_eof();
}

/**
 * This must be called after one or more objects have been read via calls to
 * read_object() in order to resolve all internal pointer references in the
 * objects read and make all the pointers valid.  It returns true if all
 * objects are successfully resolved, or false if some have not been (in which
 * case you must call resolve() again later).
 */
bool BamFile::
resolve() {
  if (_reader == nullptr) {
    return false;
  }

  return _reader->resolve();
}

/**
 * Although the bam file format is general enough to store a list of objects
 * of arbitrary type, bam files on disk usually contain just one object, a
 * PandaNode that is the root of a scene graph.  (Bam files that store other
 * kinds of things are usually given the extension "boo", for "binary other
 * objects", to differentiate them from the normal scene graph type file.)
 *
 * This is a convenience method for when you believe you are reading a scene
 * graph bam file.  It reads the one PandaNode and returns it.  It also calls
 * resolve() to fully resolve the object, since we expect this will be the
 * only object in the file.
 *
 * If the bam file contains something other than a PandaNode, an error is
 * printed and NULL is returned.
 */
PT(PandaNode) BamFile::
read_node(bool report_errors) {
  PT(PandaNode) result;

  TypedWritable *object = read_object();

  if (object != nullptr &&
      object->is_exact_type(BamCacheRecord::get_class_type())) {
    // Here's a special case: if the first object in the file is a
    // BamCacheRecord, it's really a cache data file and not a true bam file;
    // but skip over the cache data record and let the user treat it like an
    // ordinary bam file.
    object = read_object();
  }

  if (object == TypedWritable::Null) {
    if (report_errors) {
      loader_cat.error() << "Bam file " << _bam_filename << " is empty.\n";
    }

  } else if (!object->is_of_type(PandaNode::get_class_type())) {
    if (report_errors) {
      loader_cat.error()
        << "Bam file " << _bam_filename
        << " contains a " << object->get_type() << ", not a PandaNode.\n";
    }

  } else {
    result = DCAST(PandaNode, object);

    if (report_errors) {
      read_object();
      if (!is_eof()) {
        loader_cat.warning()
          << "Ignoring extra objects in " << _bam_filename << "\n";
      }
    }
  }

  if (!resolve()) {
    if (report_errors) {
      loader_cat.error()
        << "Unable to resolve Bam file.\n";
    }
    result = nullptr;
  }

  return result;
}


/**
 * Attempts to open the indicated file for writing.  If another file by the
 * same name already exists, it will be silently removed.  Returns true if
 * successful, false otherwise.
 */
bool BamFile::
open_write(const Filename &bam_filename, bool report_errors) {
  close();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->delete_file(bam_filename);
  if (!_dout.open(bam_filename)) {
    if (report_errors) {
      loader_cat.error() << "Unable to open " << bam_filename << "\n";
    }
    return false;
  }

  return continue_open_write(bam_filename, report_errors);
}

/**
 * Attempts to open the indicated stream for writing.  The filename is just
 * for information purposes only.  Returns true if successful, false on error.
 */
bool BamFile::
open_write(std::ostream &out, const string &bam_filename, bool report_errors) {
  close();

  if (!_dout.open(out)) {
    loader_cat.error() << "Could not write bam: " << bam_filename << "\n";
    return false;
  }

  return continue_open_write(bam_filename, report_errors);
}

/**
 * Writes the indicated object to the Bam file.  Returns true if successful,
 * false on error.
 */
bool BamFile::
write_object(const TypedWritable *object) {
  if (_writer == nullptr) {
    return false;
  }

  if (!_writer->write_object(object)) {
    close();
    return false;
  }

  return true;
}

/**
 * Closes the input or output stream.
 */
void BamFile::
close() {
  if (_reader != nullptr) {
    // resolve();
    delete _reader;
    _reader = nullptr;
  }
  if (_writer != nullptr) {
    delete _writer;
    _writer = nullptr;
  }
  _din.close();
  _dout.close();
}


/**
 * Returns the major version number of the file currently being read, or the
 * system current major version number if no file is currently open for
 * reading.
 */
int BamFile::
get_file_major_ver() {
  if (_reader == nullptr) {
    return _bam_major_ver;
  }
  return _reader->get_file_major_ver();
}

/**
 * Returns the minor version number of the file currently being read, or the
 * system current minor version number if no file is currently open for
 * reading.
 */
int BamFile::
get_file_minor_ver() {
  if (_reader == nullptr) {
    return _bam_minor_ver;
  }
  return _reader->get_file_minor_ver();
}

/**
 * Returns the endian preference indicated by the Bam file currently being
 * read or written.
 */
BamFile::BamEndian BamFile::
get_file_endian() const {
  if (_writer != nullptr) {
    return _writer->get_file_endian();
  }
  if (_reader != nullptr) {
    return _reader->get_file_endian();
  }

  return bam_endian;
}

/**
 * Returns true if the file stores all "standard" floats as 64-bit doubles, or
 * false if they are 32-bit floats.
 */
bool BamFile::
get_file_stdfloat_double() const {
  if (_writer != nullptr) {
    return _writer->get_file_stdfloat_double();
  }
  if (_reader != nullptr) {
    return _reader->get_file_stdfloat_double();
  }

  return bam_stdfloat_double;
}

/**
 * Returns the system current major version number.  This is the version
 * number that will be assigned to any generated Bam files.
 */
int BamFile::
get_current_major_ver() {
  return _bam_major_ver;
}

/**
 * Returns the system current minor version number.  This is the version
 * number that will be assigned to any generated Bam files.
 */
int BamFile::
get_current_minor_ver() {
  return _bam_minor_ver;
}

/**
 * Returns the BamReader in charge of performing the read operations.  This
 * will return NULL unless open_read() was called.
 */
BamReader *BamFile::
get_reader() {
  return _reader;
}

/**
 * Returns the BamWriter in charge of performing the write operations.  This
 * will return NULL unless open_write() was called.
 */
BamWriter *BamFile::
get_writer() {
  return _writer;
}

/**
 * Reads the header of the recently-opened bam stream and prepares to read the
 * contents of the file.  Returns true if successful, false otherwise.
 */
bool BamFile::
continue_open_read(const string &bam_filename, bool report_errors) {
  _bam_filename = bam_filename;

  if (!_bam_filename.empty()) {
    loader_cat.info()
      << "Reading " << _bam_filename << "\n";
  }

  string head;
  if (!_din.read_header(head, _bam_header.size())) {
    if (report_errors) {
      loader_cat.error() << _bam_filename << " is not a valid BAM file.\n";
    }
    return false;
  }

  if (head != _bam_header) {
    if (report_errors) {
      loader_cat.error() << _bam_filename << " is not a valid BAM file.\n";
    }
    return false;
  }

  _reader = new BamReader(&_din);
  if (!_reader->init()) {
    close();
    return false;
  }

  return true;
}

/**
 * Writers the header of the recently-opened bam stream and prepares to write
 * the contents of the file.  Returns true if successful, false otherwise.
 */
bool BamFile::
continue_open_write(const string &bam_filename, bool report_errors) {
  _bam_filename = bam_filename;

  if (!_bam_filename.empty()) {
    loader_cat.info() << "Writing " << _bam_filename << "\n";
  }

  if (!_dout.write_header(_bam_header)) {
    if (report_errors) {
      loader_cat.error() << "Unable to write to " << _bam_filename << "\n";
    }
    return false;
  }

  _writer = new BamWriter(&_dout);

  if (!_writer->init()) {
    close();
    return false;
  }

  return true;
}
