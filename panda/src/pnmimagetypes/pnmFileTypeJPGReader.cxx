// Filename: pnmFileTypeJPGReader.cxx
// Created by:  mike (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "pnmFileTypeJPG.h"
#include "config_pnmimagetypes.h"
#include <typedef.h>

extern "C" {
#include <jpeglib.h>
#include <setjmp.h>
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit(j_common_ptr cinfo) {
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG::Reader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeJPG::Reader::
Reader(PNMFileType *type, FILE *file, bool owns_file, string magic_number) : 
  PNMReader(type, file, owns_file)
{
  // Put the magic number bytes back into the file
  fseek(file, 0, SEEK_SET);

  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(file);
    return;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, file);

  /* Step 3: read file parameters with jpeg_read_header() */

  jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  _num_channels = cinfo.output_components;
  _x_size = (int)cinfo.output_width;
  _y_size = (int)cinfo.output_height;
  _my_file = file;
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG::Reader::read_data
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
int PNMFileTypeJPG::Reader::
read_data(xel *array, xelval *) {
  JSAMPARRAY buffer;            /* Output row buffer */
  int row_stride;               /* physical row width in output buffer */

  nassertr(cinfo.output_components == 1 || cinfo.output_components == 3, 0);

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */

  buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    jpeg_read_scanlines(&cinfo, buffer, 1);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
    //put_scanline_someplace(buffer[0], row_stride);
    int x = 0;
    for (int i = 0; i < cinfo.output_width; i += cinfo.output_components) {
      if (cinfo.output_components == 1) {
	xelval val = (uchar)buffer[i];
	PNM_ASSIGN1(array[x++], val);
      } else {
        xelval red, grn, blu;
        red = (uchar)buffer[i];
        grn = (uchar)buffer[i+1];
        blu = (uchar)buffer[i+2];
        PPM_ASSIGN(array[x++], red, grn, blu);
      }
    } 
  }

  /* Step 7: Finish decompression */

  jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */
  fclose(_my_file);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */
  return _y_size;
}
