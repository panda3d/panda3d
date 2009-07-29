// Filename: p3dSplashWindow.cxx
// Created by:  drose (17Jun09)
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

#include "p3dSplashWindow.h"


// Stuff to use libjpeg.
extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}

#include <setjmp.h>

struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo) {
  // cinfo->err really points to a my_error_mgr struct, so coerce pointer
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  // Return control to the setjmp point
  longjmp(myerr->setjmp_buffer, 1);
}



////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::Constructor
//       Access: Public
//  Description: By the time the SplashWindow is created, the instance
//               has received both its fparams and its wparams.  Copy
//               them both into this class for reference.
////////////////////////////////////////////////////////////////////
P3DSplashWindow::
P3DSplashWindow(P3DInstance *inst) : 
  _inst(inst),
  _fparams(inst->get_fparams()),
  _wparams(inst->get_wparams())
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DSplashWindow::
~P3DSplashWindow() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::set_wparams
//       Access: Public, Virtual
//  Description: Changes the window parameters, e.g. to resize or
//               reposition the window; or sets the parameters for the
//               first time, creating the initial window.
////////////////////////////////////////////////////////////////////
void P3DSplashWindow::
set_wparams(const P3DWindowParams &wparams) {
  _wparams = wparams;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::set_image_filename
//       Access: Public, Virtual
//  Description: Specifies the name of a JPEG image file that is
//               displayed in the center of the splash window.  If
//               image_filename_temp is true, the file is immediately
//               deleted after it has been read.
////////////////////////////////////////////////////////////////////
void P3DSplashWindow::
set_image_filename(const string &image_filename,
                   bool image_filename_temp) {
  if (image_filename_temp) {
    unlink(image_filename.c_str());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::set_install_label
//       Access: Public, Virtual
//  Description: Specifies the text that is displayed above the
//               install progress bar.
////////////////////////////////////////////////////////////////////
void P3DSplashWindow::
set_install_label(const string &install_label) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::set_install_progress
//       Access: Public, Virtual
//  Description: Moves the install progress bar from 0.0 to 1.0.
////////////////////////////////////////////////////////////////////
void P3DSplashWindow::
set_install_progress(double install_progress) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::handle_event
//       Access: Public, Virtual
//  Description: Deals with the event callback from the OS window
//               system.  Returns true if the event is handled, false
//               if ignored.
////////////////////////////////////////////////////////////////////
bool P3DSplashWindow::
handle_event(P3D_event_data event) {
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::read_image
//       Access: Protected
//  Description: Reads the image filename and sets image parameters
//               height, width, num_channels, and data.  Returns true
//               on success, false on failure.  If image_filename_temp
//               is true, the file will be deleted after reading.
////////////////////////////////////////////////////////////////////
bool P3DSplashWindow::
read_image(const string &image_filename, bool image_filename_temp,
           int &height, int &width, int &num_channels, 
           string &data) {
  height = 0;
  width = 0;
  num_channels = 0;
  data.clear();

  // We currently only support JPEG images.  Maybe that's all we'll
  // ever support.
  FILE *fp = fopen(image_filename.c_str(), "rb");
  if (fp == NULL) {
    nout << "Couldn't open splash file image: " << image_filename << "\n";
    if (image_filename_temp) {
      unlink(image_filename.c_str());
    }
    return false;
  }

  // We set up the normal JPEG error routines, then override error_exit.
  struct jpeg_decompress_struct cinfo;

  struct my_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  JSAMPLE *buffer = NULL;
  
  // Establish the setjmp return context for my_error_exit to use
  if (setjmp(jerr.setjmp_buffer)) {
    // If we get here, the JPEG code has signaled an error.
    nout << "JPEG error decoding " << image_filename << "\n";

    // We need to clean up the JPEG object, close the input file, and return.
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);

    if (buffer != NULL) {
      delete[] buffer;
    }

    if (image_filename_temp) {
      unlink(image_filename.c_str());
    }
    return false;
  }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, fp);

  jpeg_read_header(&cinfo, true);

  cinfo.scale_num = 1;
  cinfo.scale_denom = 1;

  jpeg_start_decompress(&cinfo);

  width = cinfo.output_width;
  height = cinfo.output_height;
  num_channels = cinfo.output_components;

  int row_stride = width * num_channels;

  size_t buffer_size = height * row_stride;
  buffer = new JSAMPLE[buffer_size];
  JSAMPLE *buffer_end = buffer + buffer_size;

  JSAMPLE *rowptr = buffer;
  while (cinfo.output_scanline < cinfo.output_height) {
    assert(rowptr + row_stride <= buffer_end);
    jpeg_read_scanlines(&cinfo, &rowptr, 1);
    rowptr += row_stride;
  }

  jpeg_finish_decompress(&cinfo);

  fclose(fp);
  if (image_filename_temp) {
    unlink(image_filename.c_str());
  }

  data.append((const char *)buffer, buffer_size);
  delete[] buffer;

  return true;
}
