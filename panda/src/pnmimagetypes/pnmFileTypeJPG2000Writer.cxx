// Filename: pnmFileTypeJPGWriter.cxx
// Created by:  mike (19Jun00)
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

#include "pnmFileTypeJPG2000.h"
#include "config_pnmimagetypes.h"

#include "pnmImage.h"
#include "pnmWriter.h"

extern "C" {
#include <jpeglib.h>
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::Writer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypeJPG2000::Writer::
Writer(PNMFileType *type, ostream *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::Writer::write_data
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
int PNMFileTypeJPG2000::Writer::
write_data(xel *array, xelval *) {
  if (_y_size<=0 || _x_size<=0) {
    return 0;
  }

  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  JSAMPROW row_pointer[1];      /* pointer to JSAMPLE row[s] */
  int row_stride;               /* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);

  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */
  // This is broken, and won't compile.  We need to drop in an
  // iostream replacement, like we did for JPG.
  jpeg_stdio_dest(&cinfo, _file);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = _x_size;      /* image width and height, in pixels */
  cinfo.image_height = _y_size;
  if (is_grayscale()) {
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
  } else {
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
  }
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, jpeg_quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = _x_size * cinfo.input_components; /* JSAMPLEs per row in image_buffer */

  int x = 0;
  JSAMPROW row = new JSAMPLE[row_stride];
  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    for (int i = 0; i < row_stride; i += cinfo.input_components) {
      if (cinfo.input_components == 1) {
        row[i] = (JSAMPLE)(MAXJSAMPLE * PPM_GETB(array[x])/_maxval);
      } else {
        row[i] = (JSAMPLE)(MAXJSAMPLE * PPM_GETR(array[x])/_maxval);
        row[i+1] = (JSAMPLE)(MAXJSAMPLE * PPM_GETG(array[x])/_maxval);
        row[i+2] = (JSAMPLE)(MAXJSAMPLE * PPM_GETB(array[x])/_maxval);
      }
      x++;
    }
    //row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    //(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    row_pointer[0] = row;
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  delete row;

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  return _y_size;
}
