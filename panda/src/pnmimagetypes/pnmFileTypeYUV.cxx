// Filename: pnmFileTypeYUV.cxx
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////


// Much code in this file is borrowed from Netpbm, specifically yuvtoppm.c
// and ppmtoyuv.c.

/* yuvtoppm.c - convert Abekas YUV bytes into a portable pixmap
**
** by Marc Boucher
** Internet: marc@PostImage.cxxOM
** 
** Based on Example Conversion Program, A60/A64 Digital Video Interface
** Manual, page 69
**
** Uses integer arithmetic rather than floating point for better performance
**
** Copyright (C) 1991 by DHD PostImage Inc.
** Copyright (C) 1987 by Abekas Video Systems Inc.
** Copyright (C) 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

/* ppmtoyuv.c - convert a portable pixmap into an Abekas YUV file
**
** by Marc Boucher
** Internet: marc@PostImage.cxxOM
** 
** Based on Example Conversion Program, A60/A64 Digital Video Interface
** Manual, page 69.
**
** Copyright (C) 1991 by DHD PostImage Inc.
** Copyright (C) 1987 by Abekas Video Systems Inc.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include "pnmFileTypeYUV.h"
#include "config_pnmimagetypes.h"

/* x must be signed for the following to work correctly */
#define limit(x) (xelval)(((x>0xffffff)?0xff0000:((x<=0xffff)?0:x&0xff0000))>>16)

static const char * const extensions[] = {
  "yuv"
};
static const int num_extensions = sizeof(extensions) / sizeof(const char *);

TypeHandle PNMFileTypeYUV::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeYUV::
PNMFileTypeYUV() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::get_name
//       Access: Public, Virtual
//  Description: Returns a few words describing the file type.
////////////////////////////////////////////////////////////////////
string PNMFileTypeYUV::
get_name() const {
  return "Abekas YUV";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::get_num_extensions
//       Access: Public, Virtual
//  Description: Returns the number of different possible filename
//               extensions associated with this particular file type.
////////////////////////////////////////////////////////////////////
int PNMFileTypeYUV::
get_num_extensions() const {
  return num_extensions;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::get_extension
//       Access: Public, Virtual
//  Description: Returns the nth possible filename extension
//               associated with this particular file type, without a
//               leading dot.
////////////////////////////////////////////////////////////////////
string PNMFileTypeYUV::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions, string());
  return extensions[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::get_suggested_extension
//       Access: Public, Virtual
//  Description: Returns a suitable filename extension (without a
//               leading dot) to suggest for files of this type, or
//               empty string if no suggestions are available.
////////////////////////////////////////////////////////////////////
string PNMFileTypeYUV::
get_suggested_extension() const {
  return "yuv";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::make_reader
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMReader suitable for
//               reading from this file type, if possible.  If reading
//               from this file type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMReader *PNMFileTypeYUV::
make_reader(FILE *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::make_writer
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMWriter suitable for
//               reading from this file type, if possible.  If writing
//               files of this type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMFileTypeYUV::
make_writer(FILE *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::Reader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeYUV::Reader::
Reader(PNMFileType *type, FILE *file, bool owns_file, string magic_number) : 
  PNMReader(type, file, owns_file)
{
  yuvbuf = NULL;

  // Hope we can ungetc() more than one character.
  for (string::reverse_iterator mi = magic_number.rbegin();
       mi != magic_number.rend();
       mi++) {
    ungetc(*mi, _file);
  }

  _x_size = yuv_xsize;
  _y_size = yuv_ysize;
  _num_channels = 3;

  if (_x_size <= 0 || _y_size <= 0) {
    _is_valid = false;
    return;
  }

  nassertv(255 <= PGM_MAXMAXVAL);

  yuvbuf = (unsigned char *) pm_allocrow(_x_size, 2);

  _maxval = 255;

  if (pnmimage_yuv_cat.is_debug()) {
    pnmimage_yuv_cat.debug()
      << "Reading YUV " << *this << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::Reader::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeYUV::Reader::
~Reader() {
  if (yuvbuf!=NULL) {
    pm_freerow((char *)yuvbuf);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::Reader::supports_read_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMReader supports a
//               streaming interface to reading the data: that is, it
//               is capable of returning the data one row at a time,
//               via repeated calls to read_row().  Returns false if
//               the only way to read from this file is all at once,
//               via read_data().
////////////////////////////////////////////////////////////////////
bool PNMFileTypeYUV::Reader::
supports_read_row() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::Reader::read_row
//       Access: Public, Virtual
//  Description: If supports_read_row(), above, returns true, this
//               function may be called repeatedly to read the image,
//               one horizontal row at a time, beginning from the top.
//               Returns true if the row is successfully read, false
//               if there is an error or end of file.
////////////////////////////////////////////////////////////////////
bool PNMFileTypeYUV::Reader::
read_row(xel *row_data, xelval *) {
  long y, u, v, y1, r, g, b;
  unsigned char *yuvptr;
  int col;

  if (fread(yuvbuf, _x_size * 2, 1, _file) != 1) {
    // Short file--perhaps it's just a field instead of a full frame.
    // Since the YUV format does not include a length designation, we'll
    // have to assume this is not a problem and just truncate here.
    return false;
  }

  yuvptr = yuvbuf;
  for (col = 0; col < _x_size; col += 2) {
    u = (int)yuvptr[0] - 128;
    y = (int)yuvptr[1] - 16;
    if (y < 0) y = 0;

    v = (int)yuvptr[2] - 128;
    y1 = (int)yuvptr[3] - 16;
    if (y1 < 0) y1 = 0;
    
    r = 104635 * v;
    g = -25690 * u + -53294 * v;
    b = 132278 * u;
    
    y*=76310; y1*=76310;
    
    PPM_ASSIGN(row_data[col], limit(r+y), limit(g+y), limit(b+y));
    PPM_ASSIGN(row_data[col+1], limit(r+y1), limit(g+y1), limit(b+y1));

    yuvptr += 4;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::Writer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeYUV::Writer::
Writer(PNMFileType *type, FILE *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
  yuvbuf = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::Writer::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeYUV::Writer::
~Writer() {
  if (yuvbuf!=NULL) {
    pm_freerow((char *)yuvbuf);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::Writer::supports_write_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMWriter supports a
//               streaming interface to writing the data: that is, it
//               is capable of writing the image one row at a time,
//               via repeated calls to write_row().  Returns false if
//               the only way to write from this file is all at once,
//               via write_data().
////////////////////////////////////////////////////////////////////
bool PNMFileTypeYUV::Writer::
supports_write_row() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::Writer::write_header
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
bool PNMFileTypeYUV::Writer::
write_header() {
  if (yuvbuf!=NULL) {
    pm_freerow((char *)yuvbuf);
  }

  yuvbuf = (unsigned char *) pm_allocrow( _x_size, 2 );

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeYUV::Writer::write_row
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
bool PNMFileTypeYUV::Writer::
write_row(xel *row_data, xelval *) {
  int             col;
  unsigned long   y1, y2=0, u=0, v=0, u0=0, u1, u2, v0=0, v1, v2;
  static const int max_byte = 255;

  unsigned char *yuvptr;
    
  for (col = 0, yuvptr=yuvbuf; col < _x_size; col += 2) {
    pixval r, g, b;
      
    /* first pixel gives Y and 0.5 of chroma */
    r = (pixval)(max_byte * PPM_GETR(row_data[col])/_maxval);
    g = (pixval)(max_byte * PPM_GETG(row_data[col])/_maxval);
    b = (pixval)(max_byte * PPM_GETB(row_data[col])/_maxval);
    
    y1 = 16829 * r + 33039 * g + 6416 * b + (0xffff & y2);
    u1 = -4853 * r - 9530 * g + 14383 * b;
    v1 = 14386 * r - 12046 * g - 2340 * b;
    
    /* second pixel just yields a Y and 0.25 U, 0.25 V */
    r = (pixval)(max_byte * PPM_GETR(row_data[col])/_maxval);
    g = (pixval)(max_byte * PPM_GETG(row_data[col])/_maxval);
    b = (pixval)(max_byte * PPM_GETB(row_data[col])/_maxval);
    
    y2 = 16829 * r + 33039 * g + 6416 * b + (0xffff & y1);
    u2 = -2426 * r - 4765 * g + 7191 * b;
    v2 = 7193 * r - 6023 * g - 1170 * b;
    
    /* filter the chroma */
    u = u0 + u1 + u2 + (0xffff & u);
    v = v0 + v1 + v2 + (0xffff & v);
    
    u0 = u2;
    v0 = v2;
    
    *yuvptr++ = (unsigned char)((u >> 16) + 128);
    *yuvptr++ = (unsigned char)((y1 >> 16) + 16);
    *yuvptr++ = (unsigned char)((v >> 16) + 128);
    *yuvptr++ = (unsigned char)((y2 >> 16) + 16);
  }
  fwrite(yuvbuf, _x_size*2, 1, _file);

  return true;
}


