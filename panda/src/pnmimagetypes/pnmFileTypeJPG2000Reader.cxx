// Filename: pnmFileTypeJPGReader.cxx
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

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::Reader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypeJPG2000::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  // Put the magic number bytes back into the file
  file->seekg(0);

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  _cinfo.err = jpeg_std_error(&_jerr.pub);

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&_cinfo);

  /* Step 2: specify data source (eg, a file) */
  // This is broken, and won't compile.  We need to drop in an
  // iostream replacement, like we did for JPG.
  jpeg_stdio_src(&_cinfo, file);

  /* Step 3: read file parameters with jpeg_read_header() */

  jpeg_read_header(&_cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */
  _cinfo.scale_num = jpeg_scale_num;
  _cinfo.scale_denom = jpeg_scale_denom;

  /* Step 5: Start decompressor */

  jpeg_start_decompress(&_cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  _num_channels = _cinfo.output_components;
  _x_size = (int)_cinfo.output_width;
  _y_size = (int)_cinfo.output_height;
  _maxval = MAXJSAMPLE;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::Reader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypeJPG2000::Reader::
~Reader(void) {
  jpeg_destroy_decompress(&_cinfo);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::Reader::read_data
//       Access: Public, Virtual
//  Description: Reads in an entire image all at once, storing it in
//               the pre-allocated _x_size * _y_size array and alpha
//               pointers.  (If the image type has no alpha channel,
//               alpha is ignored.)  Returns the number of rows
//               correctly read.
//
//               Derived classes need not override this if they
//               instead provide supports_read_row() and read_row(),
//               below.
////////////////////////////////////////////////////////////////////
int PNMFileTypeJPG2000::Reader::
read_data(xel *array, xelval *) {
  JSAMPARRAY buffer;            /* Output row buffer */
  int row_stride;               /* physical row width in output buffer */

  nassertr(_cinfo.output_components == 1 || _cinfo.output_components == 3, 0);

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */
  /* JSAMPLEs per row in output buffer */
  row_stride = _cinfo.output_width * _cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */

  buffer = (*_cinfo.mem->alloc_sarray)
                ((j_common_ptr) &_cinfo, JPOOL_IMAGE, row_stride, 1);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  int x = 0;
  while (_cinfo.output_scanline < _cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    jpeg_read_scanlines(&_cinfo, buffer, 1);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
    //put_scanline_someplace(buffer[0], row_stride);
    JSAMPROW bufptr = buffer[0];
    for (int i = 0; i < row_stride; i += _cinfo.output_components) {
      if (_cinfo.output_components == 1) {
        xelval val = (xelval)bufptr[i];
        nassertr(x < _x_size * _y_size, 0);
        PNM_ASSIGN1(array[x], val);
      } else {
        xelval red, grn, blu;
        red = (xelval)bufptr[i];
        grn = (xelval)bufptr[i+1];
        blu = (xelval)bufptr[i+2];
        nassertr(x < _x_size * _y_size, 0);
        PPM_ASSIGN(array[x], red, grn, blu);
      }
      x++;
    }
  }

  /* Step 7: Finish decompression */

  jpeg_finish_decompress(&_cinfo);

  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */
  if (_jerr.pub.num_warnings) {
    pnmimage_jpg_cat.warning()
      << "Jpeg data may be corrupt" << endl;
  }

  return _y_size;
}
