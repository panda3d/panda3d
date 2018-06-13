/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeJPGReader.cxx
 * @author mike
 * @date 2000-06-19
 */

#include "pnmFileTypeJPG.h"

#ifdef HAVE_JPEG

#include "config_pnmimagetypes.h"
#include "thread.h"

// The following bit of code, for setting up jpeg_istream_src(), was lifted
// from jpeglib, and modified to work with istream instead of stdio.

/*
 * jdatasrc.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression data source routines for the case of
 * reading JPEG data from a file (or any stdio stream).  While these routines
 * are sufficient for most applications, some will want to use a different
 * source manager.
 * IMPORTANT: we assume that fread() will correctly transcribe an array of
 * JOCTETs from 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}


/* Expanded data source object for stdio input */

typedef struct {
  struct jpeg_source_mgr pub;   /* public fields */

  std::istream * infile;             /* source stream */
  JOCTET * buffer;              /* start of buffer */
  boolean start_of_file;        /* have we gotten any data yet? */
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

#define INPUT_BUF_SIZE  4096    /* choose an efficiently fread'able size */


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->start_of_file = TRUE;
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;
  size_t nbytes;

  src->infile->read((char *)src->buffer, INPUT_BUF_SIZE);
  nbytes = src->infile->gcount();
  Thread::consider_yield();

  if (nbytes <= 0) {
    if (src->start_of_file)     /* Treat empty input file as fatal error */
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = FALSE;

  return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

GLOBAL(void)
jpeg_istream_src (j_decompress_ptr cinfo, std::istream * infile)
{
  my_src_ptr src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == nullptr) {     /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  sizeof(my_source_mgr));
    src = (my_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  INPUT_BUF_SIZE * sizeof(JOCTET));
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->infile = infile;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = nullptr; /* until buffer loaded */
}



// The rest of the code in this file is new to Panda.

/**
 *
 */
PNMFileTypeJPG::Reader::
Reader(PNMFileType *type, std::istream *file, bool owns_file, std::string magic_number) :
  PNMReader(type, file, owns_file)
{
  // Hope we can putback() more than one character.
  for (std::string::reverse_iterator mi = magic_number.rbegin();
       mi != magic_number.rend();
       ++mi) {
    _file->putback(*mi);
  }
  if (_file->fail()) {
    pnmimage_jpg_cat.error()
      << "Unable to put back magic number.\n";
    _is_valid = false;
    return;
  }
  _is_valid = true;

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  _cinfo.err = jpeg_std_error(&_jerr.pub);

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&_cinfo);

  /* Step 2: specify data source (eg, a file) */
  jpeg_istream_src(&_cinfo, file);

  /* Step 3: let lib jpeg know that we want to read the comment field */
  jpeg_save_markers(&_cinfo, JPEG_COM, 0xffff);

  /* Step 4: read file parameters with jpeg_read_header() */
  jpeg_read_header(&_cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  _num_channels = _cinfo.num_components;
  _x_size = (int)_cinfo.image_width;
  _y_size = (int)_cinfo.image_height;
  _maxval = MAXJSAMPLE;

  /* Step 6: set parameters for decompression */
  _cinfo.scale_num = 1;
  _cinfo.scale_denom = 1;
}

/**
 * This method will be called before read_data() or read_row() is called.  It
 * instructs the reader to initialize its data structures as necessary to
 * actually perform the read operation.
 *
 * After this call, _x_size and _y_size should reflect the actual size that
 * will be filled by read_data() (as possibly modified by set_read_size()).
 */
void PNMFileTypeJPG::Reader::
prepare_read() {
  if (_has_read_size && _read_x_size != 0 && _read_y_size != 0) {
    // Attempt to get the scale close to our target scale.
    int x_reduction = _cinfo.image_width / _read_x_size;
    int y_reduction = _cinfo.image_height / _read_y_size;
    _cinfo.scale_denom = std::max(std::min(x_reduction, y_reduction), 1);
  }

  /* Step 7: Start decompressor */

  jpeg_start_decompress(&_cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  _num_channels = _cinfo.output_components;
  _x_size = (int)_cinfo.output_width;
  _y_size = (int)_cinfo.output_height;
}

/**
 *
 */
PNMFileTypeJPG::Reader::
~Reader() {
  if (_is_valid) {
    jpeg_destroy_decompress(&_cinfo);
    _is_valid = false;
  }
}

/**
 * Reads in an entire image all at once, storing it in the pre-allocated
 * _x_size * _y_size array and alpha pointers.  (If the image type has no
 * alpha channel, alpha is ignored.)  Returns the number of rows correctly
 * read.
 *
 * Derived classes need not override this if they instead provide
 * supports_read_row() and read_row(), below.
 */
int PNMFileTypeJPG::Reader::
read_data(xel *array, xelval *) {
  if (!_is_valid) {
    return 0;
  }
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
    // put_scanline_someplace(buffer[0], row_stride);
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
    Thread::consider_yield();
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
      << "Jpeg data may be corrupt" << std::endl;
  }

  return _y_size;
}

#endif  // HAVE_JPEG
