// Filename: pnmFileTypeJPGWriter.cxx
// Created by:  mike (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "pnmFileTypeJPG.h"
#include "config_pnmimagetypes.h"

#include <pnmImage.h>
#include <pnmWriter.h>

extern "C" {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG::Writer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeJPG::Writer::
Writer(PNMFileType *type, FILE *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG::Writer::write_data
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
int PNMFileTypeJPG::Writer::
write_data(xel *array, xelval *) {
  if (_y_size<=0 || _x_size<=0) {
    return 0;
  }

  return _y_size;
}
