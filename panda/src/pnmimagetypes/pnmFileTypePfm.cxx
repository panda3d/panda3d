/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypePfm.cxx
 * @author drose
 * @date 1998-04-04
 */

#include "pnmFileTypePfm.h"
#include "pfmFile.h"
#include "config_pnmimage.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

using std::istream;
using std::ostream;
using std::string;

TypeHandle PNMFileTypePfm::_type_handle;

/**
 *
 */
PNMFileTypePfm::
PNMFileTypePfm() {
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypePfm::
get_name() const {
  return "Portable Float Map";
}

/**
 * Returns the number of different possible filename extensions associated
 * with this particular file type.
 */
int PNMFileTypePfm::
get_num_extensions() const {
  return 1;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypePfm::
get_extension(int n) const {
  return "pfm";
}

/**
 * Returns a suitable filename extension (without a leading dot) to suggest
 * for files of this type, or empty string if no suggestions are available.
 */
string PNMFileTypePfm::
get_suggested_extension() const {
  return "pfm";
}

/**
 * Returns true if this particular file type uses a magic number to identify
 * it, false otherwise.
 */
bool PNMFileTypePfm::
has_magic_number() const {
  return true;
}

/**
 * Returns true if the indicated "magic number" byte stream (the initial few
 * bytes read from the file) matches this particular file type, false
 * otherwise.
 */
bool PNMFileTypePfm::
matches_magic_number(const string &magic_number) const {
  return (magic_number.size() >= 2) &&
    (magic_number.substr(0, 2) == "PF" ||
     magic_number.substr(0, 2) == "Pf" ||
     magic_number.substr(0, 2) == "pf");
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypePfm::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileTypePfm::
make_writer(ostream *file, bool owns_file) {
  return new Writer(this, file, owns_file);
}


/**
 *
 */
PNMFileTypePfm::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  read_magic_number(_file, magic_number, 2);

  if (magic_number == "pf") {
    // In this case, we're probably reading a special-extension 4-channel pfm
    // file, and we need a four-byte magic number to confirm this and fully
    // identify the file format.
    read_magic_number(_file, magic_number, 4);
  }

  if (magic_number == "PF") {
    _num_channels = 3;

  } else if (magic_number == "Pf") {
    _num_channels = 1;

  } else if (magic_number == "pf2c") {
    // Special DRZ extension.
    _num_channels = 2;

  } else if (magic_number == "pf4c") {
    // Special DRZ extension.
    _num_channels = 4;

  } else {
    pnmimage_cat.debug()
      << "Not a PFM file\n";
    _is_valid = false;
    return;
  }

  _maxval = PGM_MAXMAXVAL;

  (*_file) >> _x_size >> _y_size >> _scale;
  if (!(*_file)) {
    pnmimage_cat.debug()
      << "Error parsing PFM header\n";
    _is_valid = false;
    return;
  }

  // Skip the last newlinewhitespace character before the raw data begins.
  (*_file).get();
}

/**
 * Returns true if this PNMFileType represents a floating-point image type,
 * false if it is a normal, integer type.  If this returns true, read_pfm() is
 * implemented instead of read_data().
 */
bool PNMFileTypePfm::Reader::
is_floating_point() {
  return true;
}

/**
 * Reads floating-point data directly into the indicated PfmFile.  Returns
 * true on success, false on failure.
 */
bool PNMFileTypePfm::Reader::
read_pfm(PfmFile &pfm) {
  if (!is_valid()) {
    return false;
  }

  bool little_endian = false;
  if (_scale < 0) {
    _scale = -_scale;
    little_endian = true;
  }
  if (pfm_force_littleendian) {
    little_endian = true;
  }
  if (pfm_reverse_dimensions) {
    int t = _x_size;
    _x_size = _y_size;
    _y_size = t;
  }

  pfm.clear(_x_size, _y_size, _num_channels);
  pfm.set_scale(_scale);

  // So far, so good.  Now read the data.
  int size = _x_size * _y_size * _num_channels;

  pvector<PN_float32> table;
  pfm.swap_table(table);

  (*_file).read((char *)&table[0], sizeof(PN_float32) * size);
  if ((*_file).fail() && !(*_file).eof()) {
    pfm.clear();
    return false;
  }

  // Now we may have to endian-reverse the data.
#ifdef WORDS_BIGENDIAN
  bool endian_reversed = little_endian;
#else
  bool endian_reversed = !little_endian;
#endif

  if (endian_reversed) {
    for (int ti = 0; ti < size; ++ti) {
      ReversedNumericData nd(&table[ti], sizeof(PN_float32));
      nd.store_value(&table[ti], sizeof(PN_float32));
    }
  }

  pfm.swap_table(table);
  return true;
}


/**
 *
 */
PNMFileTypePfm::Writer::
Writer(PNMFileType *type, ostream *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}

/**
 * Returns true if this PNMFileType can accept a floating-point image type,
 * false if it can only accept a normal, integer type.  If this returns true,
 * write_pfm() is implemented.
 */
bool PNMFileTypePfm::Writer::
supports_floating_point() {
  return true;
}

/**
 * Returns true if this PNMFileType can accept an integer image type, false if
 * it can only accept a floating-point type.  If this returns true,
 * write_data() or write_row() is implemented.
 */
bool PNMFileTypePfm::Writer::
supports_integer() {
  return false;
}

/**
 * Writes floating-point data from the indicated PfmFile.  Returns true on
 * success, false on failure.
 */
bool PNMFileTypePfm::Writer::
write_pfm(const PfmFile &pfm) {
  nassertr(pfm.is_valid(), false);

  switch (pfm.get_num_channels()) {
  case 1:
    (*_file) << "Pf\n";
    break;

  case 2:
    (*_file) << "pf2c\n";
    break;

  case 3:
    (*_file) << "PF\n";
    break;

  case 4:
    (*_file) << "pf4c\n";
    break;

  default:
    nassert_raise("unexpected channel count");
    return false;
  }
  (*_file) << pfm.get_x_size() << " " << pfm.get_y_size() << "\n";

  PN_float32 scale = cabs(pfm.get_scale());
  if (scale == 0.0f) {
    scale = 1.0f;
  }
#ifndef WORDS_BIGENDIAN
  // Little-endian computers must write a negative scale to indicate the
  // little-endian nature of the output.
  scale = -scale;
#endif
  (*_file) << scale << "\n";

  int size = pfm.get_x_size() * pfm.get_y_size() * pfm.get_num_channels();
  const pvector<PN_float32> &table = pfm.get_table();
  (*_file).write((const char *)&table[0], sizeof(PN_float32) * size);

  if ((*_file).fail()) {
    return false;
  }
  nassertr(sizeof(PN_float32) == 4, false);
  return true;
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypePfm::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypePfm);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 *
 * In the case of the PNMFileType objects, since these objects are all shared,
 * we just pull the object from the registry.
 */
TypedWritable *PNMFileTypePfm::
make_PNMFileTypePfm(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}
