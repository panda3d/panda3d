// Filename: pnmFileTypeRadiance.cxx
// Created by:  drose (17Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "pnmFileTypeRadiance.h"
#include "config_pnmimagetypes.h"

extern "C" {
  #include  "color.h"
  #include  "resolu.h"

  void setcolrgam(double);
  int checkheader(FILE *, char *, FILE *);
  int fgetresolu(int *, int *, FILE *);
  int freadcolrs(COLR *, int, FILE *);
  int fwritecolrs(COLR *, unsigned, FILE *);
  void fputformat(char *, FILE *);
  void shiftcolrs(COLR *, int, int);
  void colrs_gambs(COLR *, int);
  void newheader(char *, FILE *);
  void printargs(int, char **, FILE *);
  void gambs_colrs(COLR *, int);
}

static const char * const extensions[] = {
  "pic", "rad"
};
static const int num_extensions = sizeof(extensions) / sizeof(const char *);

TypeHandle PNMFileTypeRadiance::_type_handle;


static const int COLR_MAX = 255;

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeRadiance::
PNMFileTypeRadiance() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::get_name
//       Access: Public, Virtual
//  Description: Returns a few words describing the file type.
////////////////////////////////////////////////////////////////////
string PNMFileTypeRadiance::
get_name() const {
  return "Radiance";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::get_num_extensions
//       Access: Public, Virtual
//  Description: Returns the number of different possible filename
//               extensions associated with this particular file type.
////////////////////////////////////////////////////////////////////
int PNMFileTypeRadiance::
get_num_extensions() const {
  return num_extensions;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::get_extension
//       Access: Public, Virtual
//  Description: Returns the nth possible filename extension
//               associated with this particular file type, without a
//               leading dot.
////////////////////////////////////////////////////////////////////
string PNMFileTypeRadiance::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions, string());
  return extensions[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::get_suggested_extension
//       Access: Public, Virtual
//  Description: Returns a suitable filename extension (without a
//               leading dot) to suggest for files of this type, or
//               empty string if no suggestions are available.
////////////////////////////////////////////////////////////////////
string PNMFileTypeRadiance::
get_suggested_extension() const {
  return "rad";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::has_magic_number
//       Access: Public, Virtual
//  Description: Returns true if this particular file type uses a
//               magic number to identify it, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypeRadiance::
has_magic_number() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::matches_magic_number
//       Access: Public, Virtual
//  Description: Returns true if the indicated "magic number" byte
//               stream (the initial few bytes read from the file)
//               matches this particular file type, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypeRadiance::
matches_magic_number(const string &magic_number) const {
  nassertr(magic_number.size() >= 2, false);
  return (magic_number.substr(0, 2) == "#?");
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::make_reader
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMReader suitable for
//               reading from this file type, if possible.  If reading
//               from this file type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMReader *PNMFileTypeRadiance::
make_reader(FILE *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::make_writer
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMWriter suitable for
//               reading from this file type, if possible.  If writing
//               files of this type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMFileTypeRadiance::
make_writer(FILE *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}



////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::Reader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeRadiance::Reader::
Reader(PNMFileType *type, FILE *file, bool owns_file, string magic_number) : 
  PNMReader(type, file, owns_file)
{
  setcolrgam(radiance_gamma_correction);

  // Hope we can ungetc() more than one character.
  for (string::reverse_iterator mi = magic_number.rbegin();
       mi != magic_number.rend();
       mi++) {
    ungetc(*mi, _file);
  }

  /* get our header */
  if (checkheader(file, COLRFMT, NULL) < 0 ||
      fgetresolu(&_x_size, &_y_size, _file) < 0) {
    _is_valid = false;
    pnmimage_radiance_cat.debug()
      << "File is not a valid Radiance image.\n";
    return;
  }

  _num_channels = 3;
  _maxval = COLR_MAX;

  if (pnmimage_radiance_cat.is_debug()) {
    pnmimage_radiance_cat.debug()
      << "Reading Radiance image: " << _x_size << " by " << _y_size 
      << " pixels.\n"
      << "gamma correction is " << radiance_gamma_correction
      << ", brightness adjustment is " << radiance_brightness_adjustment
      << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::Reader::supports_read_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMReader supports a
//               streaming interface to reading the data: that is, it
//               is capable of returning the data one row at a time,
//               via repeated calls to read_row().  Returns false if
//               the only way to read from this file is all at once,
//               via read_data().
////////////////////////////////////////////////////////////////////
bool PNMFileTypeRadiance::Reader::
supports_read_row() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::Reader::read_row
//       Access: Public, Virtual
//  Description: If supports_read_row(), above, returns true, this
//               function may be called repeatedly to read the image,
//               one horizontal row at a time, beginning from the top.
//               Returns true if the row is successfully read, false
//               if there is an error or end of file.
////////////////////////////////////////////////////////////////////
bool PNMFileTypeRadiance::Reader::
read_row(xel *row_data, xelval *) {
  if (!is_valid()) {
    return false;
  }

  COLR *scanin;
  int x;

  scanin = (COLR *)alloca(_x_size * sizeof(COLR));

  if (freadcolrs(scanin, _x_size, _file) < 0) {
    return false;
  }
  
  if (radiance_brightness_adjustment) {
    shiftcolrs(scanin, _x_size, radiance_brightness_adjustment);
  }
  
  colrs_gambs(scanin, _x_size);      /* gamma correction */
  
  for (x = 0; x < _x_size; x++) {
    PPM_ASSIGN(row_data[x], scanin[x][RED], scanin[x][GRN], scanin[x][BLU]);
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::Writer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeRadiance::Writer::
Writer(PNMFileType *type, FILE *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::Writer::supports_write_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMWriter supports a
//               streaming interface to writing the data: that is, it
//               is capable of writing the image one row at a time,
//               via repeated calls to write_row().  Returns false if
//               the only way to write from this file is all at once,
//               via write_data().
////////////////////////////////////////////////////////////////////
bool PNMFileTypeRadiance::Writer::
supports_write_row() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::Writer::write_header
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
bool PNMFileTypeRadiance::Writer::
write_header() {
  setcolrgam(radiance_gamma_correction);

  /* put our header */
  newheader("RADIANCE", _file);
  fputs("Generated via pnmimage\n", _file);
  fputformat(COLRFMT, _file);
  putc('\n', _file);
  fprtresolu(_x_size, _y_size, _file);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeRadiance::Writer::write_row
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
bool PNMFileTypeRadiance::Writer::
write_row(xel *row_data, xelval *) {
  /* convert file */
  bool grayscale = is_grayscale();

  COLR *scanout;
  int x;

  /* allocate scanline */
  scanout = (COLR *)alloca(_x_size * sizeof(COLR));

  /* convert image */
  for (x = 0; x < _x_size; x++) {
    if (grayscale) {
      scanout[x][RED] =
	scanout[x][GRN] =
	scanout[x][BLU] = (BYTE)((int)COLR_MAX * PPM_GETB(row_data[x]) / _maxval);
    } else {
      scanout[x][RED] = (BYTE)((int)COLR_MAX * PPM_GETR(row_data[x]) / _maxval);
      scanout[x][GRN] = (BYTE)((int)COLR_MAX * PPM_GETG(row_data[x]) / _maxval);
      scanout[x][BLU] = (BYTE)((int)COLR_MAX * PPM_GETB(row_data[x]) / _maxval);
    }
  }

  /* undo gamma */
  gambs_colrs(scanout, _x_size);
  if (radiance_brightness_adjustment)
    shiftcolrs(scanout, _x_size, radiance_brightness_adjustment);
  if (fwritecolrs(scanout, _x_size, _file) < 0)
    return false;

  return true;
}
