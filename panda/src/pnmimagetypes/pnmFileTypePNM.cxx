// Filename: pnm-rw-pnm.cxx
// Created by:  drose (04Apr98)
// 
////////////////////////////////////////////////////////////////////

#include "pnmFileTypePNM.h"
#include "config_pnmimagetypes.h"

extern "C" {
#include "../pnm/pnm.h"
#include "../pnm/ppm.h"
#include "../pnm/pgm.h"
#include "../pnm/pbm.h"
#include "../pnm/libppm.h"
#include "../pnm/libpgm.h"
#include "../pnm/libpbm.h"
}

static const char * const extensions[] = {
  "pbm", "ppm", "pnm"
};
static const int num_extensions = sizeof(extensions) / sizeof(const char *);

TypeHandle PNMFileTypePNM::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypePNM::
PNMFileTypePNM() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::get_name
//       Access: Public, Virtual
//  Description: Returns a few words describing the file type.
////////////////////////////////////////////////////////////////////
string PNMFileTypePNM::
get_name() const {
  return "NetPBM-style PBM/PPM/PNM";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::get_num_extensions
//       Access: Public, Virtual
//  Description: Returns the number of different possible filename
//               extensions associated with this particular file type.
////////////////////////////////////////////////////////////////////
int PNMFileTypePNM::
get_num_extensions() const {
  return num_extensions;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::get_extension
//       Access: Public, Virtual
//  Description: Returns the nth possible filename extension
//               associated with this particular file type, without a
//               leading dot.
////////////////////////////////////////////////////////////////////
string PNMFileTypePNM::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions, string());
  return extensions[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::get_suggested_extension
//       Access: Public, Virtual
//  Description: Returns a suitable filename extension (without a
//               leading dot) to suggest for files of this type, or
//               empty string if no suggestions are available.
////////////////////////////////////////////////////////////////////
string PNMFileTypePNM::
get_suggested_extension() const {
  return "pnm";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::has_magic_number
//       Access: Public, Virtual
//  Description: Returns true if this particular file type uses a
//               magic number to identify it, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypePNM::
has_magic_number() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::matches_magic_number
//       Access: Public, Virtual
//  Description: Returns true if the indicated "magic number" byte
//               stream (the initial few bytes read from the file)
//               matches this particular file type, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypePNM::
matches_magic_number(const string &magic_number) const {
  return (magic_number.size() >= 2) &&
    magic_number[0] == 'P' && 
    (magic_number[1] >= '1' && magic_number[1] <= '6');
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::make_reader
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMReader suitable for
//               reading from this file type, if possible.  If reading
//               from this file type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMReader *PNMFileTypePNM::
make_reader(FILE *file, bool owns_file, const string &magic_number) {
  return new Reader(this, file, owns_file, magic_number);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::make_writer
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMWriter suitable for
//               reading from this file type, if possible.  If writing
//               files of this type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMFileTypePNM::
make_writer(FILE *file, bool owns_file) {
  return new Writer(this, file, owns_file);
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::Reader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypePNM::Reader::
Reader(PNMFileType *type, FILE *file, bool owns_file, string magic_number) : 
  PNMReader(type, file, owns_file)
{
  if (!read_magic_number(_file, magic_number, 2)) {
    // No magic number.  No image.
    if (pnmimage_pnm_cat.is_debug()) {
      pnmimage_pnm_cat.debug()
	<< "PNM file appears to be empty.\n";
    }
    _is_valid = false;
    return;
  }

  //  pnm_pbmmaxval = PNM_MAXMAXVAL;  /* use larger value for better results */

  _ftype = 
    ((unsigned char)magic_number[0] << 8) | 
    (unsigned char)magic_number[1];

  switch ( PNM_FORMAT_TYPE(_ftype) ) {
  case PPM_TYPE:
    ppm_readppminitrest( file, &_x_size, &_y_size, &_maxval );
    _num_channels = 3;
    break;
    
  case PGM_TYPE:
    pgm_readpgminitrest( file, &_x_size, &_y_size, &_maxval );
    _num_channels = 1;
    break;
    
  case PBM_TYPE:
    pbm_readpbminitrest( file, &_x_size, &_y_size );
    _num_channels = 1;
    _maxval = pnm_pbmmaxval;
    break;
    
  default:
    _is_valid = false;
  }

  if (pnmimage_pnm_cat.is_debug()) {
    if (is_valid()) {
      pnmimage_pnm_cat.debug()
	<< "Reading ";
      switch (PNM_FORMAT_TYPE(_ftype)) {
      case PPM_TYPE:
	pnmimage_pnm_cat.debug(false) << "PPM";
	break;
      case PGM_TYPE:
	pnmimage_pnm_cat.debug(false) << "PGM";
	break;
      case PBM_TYPE:
	pnmimage_pnm_cat.debug(false) << "PBM";
	break;
      }
      pnmimage_pnm_cat.debug(false)
	<< " " << *this << "\n";
    } else {
      pnmimage_pnm_cat.debug()
	<< "File is not a valid PNM image.\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::Reader::supports_read_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMReader supports a
//               streaming interface to reading the data: that is, it
//               is capable of returning the data one row at a time,
//               via repeated calls to read_row().  Returns false if
//               the only way to read from this file is all at once,
//               via read_data().
////////////////////////////////////////////////////////////////////
bool PNMFileTypePNM::Reader::
supports_read_row() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::Reader::read_row
//       Access: Public, Virtual
//  Description: If supports_read_row(), above, returns true, this
//               function may be called repeatedly to read the image,
//               one horizontal row at a time, beginning from the top.
//               Returns true if the row is successfully read, false
//               if there is an error or end of file.
////////////////////////////////////////////////////////////////////
bool PNMFileTypePNM::Reader::
read_row(xel *row_data, xelval *) {
  if (!is_valid()) {
    return false;
  }
  pnm_readpnmrow(_file, row_data, _x_size, _maxval, _ftype);
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::Writer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypePNM::Writer::
Writer(PNMFileType *type, FILE *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::Writer::supports_write_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMWriter supports a
//               streaming interface to writing the data: that is, it
//               is capable of writing the image one row at a time,
//               via repeated calls to write_row().  Returns false if
//               the only way to write from this file is all at once,
//               via write_data().
////////////////////////////////////////////////////////////////////
bool PNMFileTypePNM::Writer::
supports_write_row() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::Writer::write_header
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
bool PNMFileTypePNM::Writer::
write_header() {
  switch (get_color_type()) {
  case PNMImageHeader::CT_grayscale:
  case PNMImageHeader::CT_two_channel:
    _pnm_format = PGM_TYPE;
    break;
    
  case PNMImageHeader::CT_color:
  case PNMImageHeader::CT_four_channel:
    _pnm_format = PPM_TYPE;
    break;
  }

  pnm_writepnminit(_file, _x_size, _y_size, _maxval, _pnm_format, 0);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePNM::Writer::write_row
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
bool PNMFileTypePNM::Writer::
write_row(xel *row_data, xelval *) {
  pnm_writepnmrow(_file, row_data, _x_size, _maxval, _pnm_format, 0);

  return true;
}
