// Filename: pnmWriter.cxx
// Created by:  drose (14Jun00)
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

#include "pnmWriter.h"
#include "thread.h"

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PNMWriter::
~PNMWriter() {
  if (_owns_file) {
    delete _file;
  }
  _file = (ostream *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::supports_floating_point
//       Access: Public, Virtual
//  Description: Returns true if this PNMFileType can accept a
//               floating-point image type, false if it can only
//               accept a normal, integer type.  If this returns true,
//               write_pfm() is implemented.
////////////////////////////////////////////////////////////////////
bool PNMWriter::
supports_floating_point() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::supports_integer
//       Access: Public, Virtual
//  Description: Returns true if this PNMFileType can accept an
//               integer image type, false if it can only
//               accept a floating-point type.  If this returns true,
//               write_data() or write_row() is implemented.
////////////////////////////////////////////////////////////////////
bool PNMWriter::
supports_integer() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::write_pfm
//       Access: Public, Virtual
//  Description: Writes floating-point data from the indicated
//               PfmFile.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PNMWriter::
write_pfm(const PfmFile &pfm) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::write_data
//       Access: Public, Virtual
//  Description: Writes out an entire image all at once, including the
//               header, based on the image data stored in the given
//               _x_size * _y_size array and alpha pointers.  (If the
//               image type has no alpha channel, alpha is ignored.)
//               Returns the number of rows correctly written.
//
//               It is the user's responsibility to fill in the header
//               data via calls to set_x_size(), set_num_channels(),
//               etc., or copy_header_from(), before calling
//               write_data().
//
//               It is important to delete the PNMWriter class after
//               successfully writing the data.  Failing to do this
//               may result in some data not getting flushed!
//
//               Derived classes need not override this if they
//               instead provide supports_streaming() and write_row(),
//               below.
////////////////////////////////////////////////////////////////////
int PNMWriter::
write_data(xel *array, xelval *alpha) {
  if (_x_size <= 0 || _y_size <= 0) {
    return 0;
  }

  if (!write_header()) {
    return 0;
  }

  int y;
  for (y = 0; y < _y_size; y++) {
    if (!write_row(array + y * _x_size, alpha + y * _x_size)) {
      Thread::consider_yield();
      return y;
    }
  }

  return _y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::supports_write_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMWriter supports a
//               streaming interface to writing the data: that is, it
//               is capable of writing the image one row at a time,
//               via repeated calls to write_row().  Returns false if
//               the only way to write from this file is all at once,
//               via write_data().
////////////////////////////////////////////////////////////////////
bool PNMWriter::
supports_write_row() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::supports_grayscale
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMWriter understands
//               grayscale images.  If this is false, then the rgb
//               values of the xel array will be pre-filled with the
//               same value across all three channels, to allow the
//               writer to simply write out RGB data for a grayscale
//               image.
////////////////////////////////////////////////////////////////////
bool PNMWriter::
supports_grayscale() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::write_header
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
bool PNMWriter::
write_header() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::write_row
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
bool PNMWriter::
write_row(xel *, xelval *) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMWriter::supports_stream_write
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMWriter can write
//               to a general stream (including pipes, etc.), or
//               false if the writer must occasionally fseek() on its
//               output stream, and thus only disk streams are
//               supported.
////////////////////////////////////////////////////////////////////
bool PNMWriter::
supports_stream_write() const {
  return false;
}
