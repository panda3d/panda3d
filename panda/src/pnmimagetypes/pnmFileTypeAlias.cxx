// Filename: pnmFileTypeAlias.cxx
// Created by:  drose (17Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pnmFileTypeAlias.h"
#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

// Since Alias image files don't have a magic number, we'll make a
// little sanity check on the size of the image.  If either the width
// or height is larger than this, it must be bogus.
#define INSANE_SIZE 20000

static const char * const extensions_alias[] = {
  "pix", "als"
};
static const int num_extensions_alias = sizeof(extensions_alias) / sizeof(const char *);

TypeHandle PNMFileTypeAlias::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypeAlias::
PNMFileTypeAlias() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::get_name
//       Access: Public, Virtual
//  Description: Returns a few words describing the file type.
////////////////////////////////////////////////////////////////////
string PNMFileTypeAlias::
get_name() const {
  return "Alias";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::get_num_extensions
//       Access: Public, Virtual
//  Description: Returns the number of different possible filename
//               extensions associated with this particular file type.
////////////////////////////////////////////////////////////////////
int PNMFileTypeAlias::
get_num_extensions() const {
  return num_extensions_alias;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::get_extension
//       Access: Public, Virtual
//  Description: Returns the nth possible filename extension
//               associated with this particular file type, without a
//               leading dot.
////////////////////////////////////////////////////////////////////
string PNMFileTypeAlias::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_alias, string());
  return extensions_alias[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::get_suggested_extension
//       Access: Public, Virtual
//  Description: Returns a suitable filename extension (without a
//               leading dot) to suggest for files of this type, or
//               empty string if no suggestions are available.
////////////////////////////////////////////////////////////////////
string PNMFileTypeAlias::
get_suggested_extension() const {
  return "pix";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::make_reader
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMReader suitable for
//               reading from this file type, if possible.  If reading
//               from this file type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMReader *PNMFileTypeAlias::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::make_writer
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMWriter suitable for
//               reading from this file type, if possible.  If writing
//               files of this type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMFileTypeAlias::
make_writer(ostream *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}



inline unsigned short
read_ushort(istream *file) {
  unsigned short x;
  return pm_readbigshort(file, (short *)&x)==0 ? x : 0;
}

inline unsigned char
read_uchar_ALIAS(istream *file) {
  int x;
  x = file->get();
  return (x!=EOF) ? (unsigned char)x : 0;
}

inline void
write_ushort(ostream *file, unsigned short x) {
  pm_writebigshort(file, (short)x);
}

inline void
write_uchar_ALIAS(ostream *file, unsigned char x) {
  file->put(x);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::Reader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypeAlias::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  if (!read_magic_number(_file, magic_number, 4)) {
    // Although Alias files have no magic number, they do have a
    // number of ushorts at the beginning.  If these aren't present,
    // we have a problem.
    if (pnmimage_alias_cat.is_debug()) {
      pnmimage_alias_cat.debug()
        << "Alias image file appears to be empty.\n";
    }
    _is_valid = false;
    return;
  }

  _x_size =
    ((unsigned char)magic_number[0] << 8) |
    ((unsigned char)magic_number[1]);
  _y_size =
    ((unsigned char)magic_number[2] << 8) |
    ((unsigned char)magic_number[3]);

  if (_x_size == 0 || _y_size == 0 ||
      _x_size > INSANE_SIZE || _y_size > INSANE_SIZE) {
    _is_valid = false;
    pnmimage_alias_cat.debug()
      << "File is not a valid Alias image.\n";
    return;
  }

  read_ushort(_file);
  read_ushort(_file);

  int bpp = read_ushort(_file);

  switch (bpp) {
  case 8:
    _num_channels = 1;
    break;

  case 24:
    _num_channels = 3;
    break;

  default:
    _is_valid = false;
    return;
  }

  _maxval = 255;

  if (pnmimage_alias_cat.is_debug()) {
    pnmimage_alias_cat.debug()
      << "Reading Alias " << *this << "\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::Reader::supports_read_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMReader supports a
//               streaming interface to reading the data: that is, it
//               is capable of returning the data one row at a time,
//               via repeated calls to read_row().  Returns false if
//               the only way to read from this file is all at once,
//               via read_data().
////////////////////////////////////////////////////////////////////
bool PNMFileTypeAlias::Reader::
supports_read_row() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::Reader::read_row
//       Access: Public, Virtual
//  Description: If supports_read_row(), above, returns true, this
//               function may be called repeatedly to read the image,
//               one horizontal row at a time, beginning from the top.
//               Returns true if the row is successfully read, false
//               if there is an error or end of file.
////////////////////////////////////////////////////////////////////
bool PNMFileTypeAlias::Reader::
read_row(xel *row_data, xelval *) {
  if (!is_valid()) {
    return false;
  }

  int x;
  int num;
  unsigned char red, grn, blu;

  x = 0;
  while (x < _x_size) {
    num = read_uchar_ALIAS(_file);
    if (num==0 || x+num > _x_size) {
      return false;
    }
    blu = read_uchar_ALIAS(_file);

    if (get_color_type() == PNMImageHeader::CT_color) {
      grn = read_uchar_ALIAS(_file);
      red = read_uchar_ALIAS(_file);
      while (num>0) {
        PPM_ASSIGN(row_data[x], red, grn, blu);
        x++;
        num--;
      }
    } else {
      while (num>0) {
        PPM_PUTB(row_data[x], blu);
        x++;
        num--;
      }
    }
  }

  return true;
}

static unsigned char last_red = 0, last_blu = 0, last_grn = 0;
static int num_count = 0;

static void
flush_color(ostream *file) {
  if (num_count>0) {
    write_uchar_ALIAS(file, num_count);
    write_uchar_ALIAS(file, last_blu);
    write_uchar_ALIAS(file, last_grn);
    write_uchar_ALIAS(file, last_red);
    num_count = 0;
  }
}

static void
write_color(ostream *file,
            unsigned char red, unsigned char blu, unsigned char grn) {
  if (red==last_red && blu==last_blu && grn==last_grn && num_count<0377) {
    num_count++;
  } else {
    flush_color(file);
    last_red = red;
    last_grn = grn;
    last_blu = blu;
    num_count = 1;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::Writer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypeAlias::Writer::
Writer(PNMFileType *type, ostream *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::Writer::supports_write_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMWriter supports a
//               streaming interface to writing the data: that is, it
//               is capable of writing the image one row at a time,
//               via repeated calls to write_row().  Returns false if
//               the only way to write from this file is all at once,
//               via write_data().
////////////////////////////////////////////////////////////////////
bool PNMFileTypeAlias::Writer::
supports_write_row() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::Writer::write_header
//       Access: Public, Virtual
//  Description: If supports_write_row(), above, returns true, this
//               function may be called to write out the image header
//               in preparation to writing out the image data one row
//               at a time.  Returns true if the header is
//               successfully written, false if there is an error.
//
//               It is the user's responsibility to fill in the header
//               data via calls to set_x_size(), set_num_channels(),
//               etc., or copy_header_from(), before calling
//               write_header().
////////////////////////////////////////////////////////////////////
bool PNMFileTypeAlias::Writer::
write_header() {
  write_ushort(_file, _x_size);
  write_ushort(_file, _y_size);

  write_ushort(_file, 0);
  write_ushort(_file, 0);

  // We'll always write full-color Alias images, even if the source
  // was grayscale.  Many programs don't seem to understand grayscale
  // Alias images.
  write_ushort(_file, 24);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::Writer::write_row
//       Access: Public, Virtual
//  Description: If supports_write_row(), above, returns true, this
//               function may be called repeatedly to write the image,
//               one horizontal row at a time, beginning from the top.
//               Returns true if the row is successfully written,
//               false if there is an error.
//
//               You must first call write_header() before writing the
//               individual rows.  It is also important to delete the
//               PNMWriter class after successfully writing the last
//               row.  Failing to do this may result in some data not
//               getting flushed!
////////////////////////////////////////////////////////////////////
bool PNMFileTypeAlias::Writer::
write_row(xel *row_data, xelval *) {
  int x;
  unsigned char red, grn, blu;

  bool grayscale = is_grayscale();

  for (x = 0; x < _x_size; x++) {
    if (grayscale) {
      red = grn = blu = (unsigned char)(255*PPM_GETB(row_data[x]) / _maxval);
    } else {
      red = (unsigned char)(255*PPM_GETR(row_data[x]) / _maxval);
      grn = (unsigned char)(255*PPM_GETG(row_data[x]) / _maxval);
      blu = (unsigned char)(255*PPM_GETB(row_data[x]) / _maxval);
    }

    write_color(_file, red, blu, grn);
  }
  flush_color(_file);

  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void PNMFileTypeAlias::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeAlias);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeAlias::make_PNMFileTypeAlias
//       Access: Protected, Static
//  Description: This method is called by the BamReader when an object
//               of this type is encountered in a Bam file; it should
//               allocate and return a new object with all the data
//               read.
//
//               In the case of the PNMFileType objects, since these
//               objects are all shared, we just pull the object from
//               the registry.
////////////////////////////////////////////////////////////////////
TypedWritable *PNMFileTypeAlias::
make_PNMFileTypeAlias(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}
