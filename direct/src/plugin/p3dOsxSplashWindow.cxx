// Filename: p3dOsxSplashWindow.cxx
// Created by:  drose (16Jul09)
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

#include "p3dOsxSplashWindow.h"

#ifdef __APPLE__

#include <Carbon/Carbon.h>

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DOsxSplashWindow::
P3DOsxSplashWindow(P3DInstance *inst) : 
  P3DSplashWindow(inst)
{
  _image = NULL;
  _image_data = NULL;
  _install_progress = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DOsxSplashWindow::
~P3DOsxSplashWindow() {
  if (_image != NULL) {
    DisposeGWorld(_image);
  }

  if (_image_data != NULL) {
    delete[] _image_data;
    _image_data = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::set_image_filename
//       Access: Public, Virtual
//  Description: Specifies the name of a JPEG image file that is
//               displayed in the center of the splash window.  If
//               image_filename_temp is true, the file is immediately
//               deleted after it has been read.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
set_image_filename(const string &image_filename,
                   bool image_filename_temp) {
  int num_channels, row_stride;
  string data;
  if (!read_image(image_filename, image_filename_temp, 
                  _image_height, _image_width, num_channels, row_stride,
                  data)) {
    return;
  }

  QDErr err;
  Rect src_rect = { 0, 0, _image_height, _image_width };

  if (_image != NULL) {
    DisposeGWorld(_image);
    _image = NULL;
  }

  if (_image_data != NULL) {
    delete[] _image_data;
    _image_data = NULL;
  }

  // Now we need to copy from the RGB source image into the BGRA target image.
  int new_row_stride = _image_width * 4;
  _image_data = new char[new_row_stride * _image_height];
  for (int yi = 0; yi < _image_height; ++yi) {
    char *dest = _image_data + yi * new_row_stride;
    const char *source = data.data() + yi * row_stride;
    for (int xi = 0; xi < _image_width; ++xi) {
      char r = source[0];
      char g = source[1];
      char b = source[2];
#ifndef __BIG_ENDIAN__
      // Little-endian.
      dest[0] = b;
      dest[1] = g;
      dest[2] = r;
      dest[3] = 0xff;
#else  // __BIG_ENDIAN__
      // Big-endian.
      dest[0] = 0xff;
      dest[1] = r;
      dest[2] = g;
      dest[3] = b;
#endif  // __BIG_ENDIAN__
      source += 3;
      dest += 4;
    }
  }

  err = NewGWorldFromPtr(&_image, k32BGRAPixelFormat, &src_rect, 0, 0, 0, 
                         _image_data, new_row_stride);
  if (err != noErr) {
    nout << " error in NewGWorldFromPtr, called from set_image_filename()\n";
    return;
  }

  _inst->request_refresh();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::set_install_label
//       Access: Public, Virtual
//  Description: Specifies the text that is displayed above the
//               install progress bar.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
set_install_label(const string &install_label) {
  _install_label = install_label;
  _inst->request_refresh();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::set_install_progress
//       Access: Public, Virtual
//  Description: Moves the install progress bar from 0.0 to 1.0.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
set_install_progress(double install_progress) {
  if ((int)(install_progress * 500.0) != (int)(_install_progress * 500.0)) {
    // Only request a refresh if we're changing substantially.
    _inst->request_refresh();
  }
  _install_progress = install_progress;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::handle_event
//       Access: Public, Virtual
//  Description: Deals with the event callback from the OS window
//               system.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
handle_event(P3D_event_data event) {
  EventRecord *er = event._event;
  if (er->what == updateEvt) {
    paint_window();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::paint_window
//       Access: Private
//  Description: Redraws the current splash window.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
paint_window() {
  cerr << "paint_window, _image = " << _image << ", label = " << _install_label << "\n";
  GrafPtr out_port = _wparams.get_parent_window()._port;
  GrafPtr portSave = NULL;
  Boolean portChanged = QDSwapPort(out_port, &portSave);

  int win_width = _wparams.get_win_width();
  int win_height = _wparams.get_win_height();

  Rect r = { 0, 0, win_height, win_width }; 
  ClipRect(&r);

  EraseRect(&r);

  if (_image != NULL) {
    Rect src_rect = { 0, 0, _image_height, _image_width };
    Rect dest_rect;
    
    // Determine the relative size of image and window.
    int win_cx = win_width / 2;
    int win_cy = win_height / 2;
    
    if (_image_width <= win_width && _image_height <= win_height) {
      // The bitmap fits within the window; center it.
      
      // This is the top-left corner of the bitmap in window coordinates.
      int p_x = win_cx - _image_width / 2;
      int p_y = win_cy - _image_height / 2;

      dest_rect.left = p_x;
      dest_rect.top = p_y;
      dest_rect.right = p_x + _image_width;
      dest_rect.bottom = p_y + _image_height;
      
    } else {
      // The bitmap is larger than the window; scale it down.
      double x_scale = (double)win_width / (double)_image_width;
      double y_scale = (double)win_height / (double)_image_height;
      double scale = min(x_scale, y_scale);
      int sc_width = (int)(_image_width * scale);
      int sc_height = (int)(_image_height * scale);
      
      int p_x = win_cx - sc_width / 2;
      int p_y = win_cy - sc_height / 2;

      dest_rect.left = p_x;
      dest_rect.top = p_y;
      dest_rect.right = p_x + sc_width;
      dest_rect.bottom = p_y + sc_height;
    }

    CopyBits(GetPortBitMapForCopyBits(_image), 
             GetPortBitMapForCopyBits(out_port), 
             &src_rect, &dest_rect, srcCopy, 0);
  }

  int bar_width = min((int)(win_width * 0.6), 400);
  int bar_height = min((int)(win_height * 0.1), 24);
  int bar_x = (win_width - bar_width) / 2;
  int bar_y = (win_height - bar_height * 2);

  int progress = bar_x + 1 + (int)((bar_width - 2) * _install_progress);

  Rect rbar = { bar_y, bar_x, bar_y + bar_height, bar_x + bar_width };
  Rect rneed = { bar_y + 1, progress + 1, bar_y + bar_height - 1, bar_x + bar_width - 1 };
  Rect rdone = { bar_y + 1, bar_x + 1, bar_y + bar_height - 1, progress };
  FrameRect(&rbar);
  PaintRect(&rdone);
  EraseRect(&rneed);

  TextFont(0);
  TextFace(bold);
  TextMode(srcOr);
  TextSize(0);

  Point numer, denom;
  FontInfo font_info;
  StdTxMeas(_install_label.size(), _install_label.data(), &numer, &denom, &font_info);

  int text_width = TextWidth(_install_label.data(), 0, _install_label.size());
  int text_x = (win_width - text_width) / 2;
  int text_y = bar_y - font_info.descent - 8;

  Rect rtext = { text_y - font_info.ascent - 2, text_x - 2, 
                 text_y + font_info.descent + 2, text_x + text_width + 2 }; 
  EraseRect(&rtext);

  MoveTo(text_x, text_y);
  DrawText(_install_label.data(), 0, _install_label.size());

  if (portChanged) {
    QDSwapPort(portSave, NULL);
  }
}


#endif  // __APPLE__
