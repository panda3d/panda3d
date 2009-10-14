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
#include <ApplicationServices/ApplicationServices.h>

#ifndef MAC_OS_X_VERSION_10_5
  typedef float CGFloat;
#endif

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DOsxSplashWindow::
P3DOsxSplashWindow(P3DInstance *inst, bool make_visible) : 
  P3DSplashWindow(inst, make_visible)
{
  _install_progress = 0;
  _got_wparams = false;
  // We have to start with _mouse_active true; firefox doesn't send
  // activate events.
  _mouse_active = true;
  _toplevel_window = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DOsxSplashWindow::
~P3DOsxSplashWindow() {
  if (_toplevel_window != NULL) {
    SetWRefCon(_toplevel_window, 0);
    HideWindow(_toplevel_window);
    DisposeWindow(_toplevel_window);
    _toplevel_window = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::set_wparams
//       Access: Public, Virtual
//  Description: Changes the window parameters, e.g. to resize or
//               reposition the window; or sets the parameters for the
//               first time, creating the initial window.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
set_wparams(const P3DWindowParams &wparams) {
  P3DSplashWindow::set_wparams(wparams);
  _got_wparams = true;

  if (_wparams.get_window_type() == P3D_WT_toplevel ||
      _wparams.get_window_type() == P3D_WT_fullscreen) {
    // Creating a toplevel splash window.
    if (_toplevel_window == NULL) {
      Rect r;
      r.top = _wparams.get_win_y();
      r.left = _wparams.get_win_x();
      if (r.top == 0 && r.left == 0) {
        // These are the same defaults used by Panda's osxGraphicsWindow.
        r.top = 50;
        r.left = 10;
      }

      r.right = r.left + _win_width;
      r.bottom = r.top + _win_height;
      WindowAttributes attrib = 
        kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute;
      CreateNewWindow(kDocumentWindowClass, attrib, &r, &_toplevel_window);
 
      EventHandlerRef application_event_ref_ref1;
      EventTypeSpec list1[] = { 
        { kEventClassWindow, kEventWindowDrawContent },
        { kEventClassWindow, kEventWindowBoundsChanged },
        { kEventClassWindow, kEventWindowClose },
        { kEventClassMouse, kEventMouseUp },
        { kEventClassMouse, kEventMouseDown },
        { kEventClassMouse, kEventMouseMoved },
        { kEventClassMouse, kEventMouseDragged },
      };
        
      EventHandlerUPP gEvtHandler = NewEventHandlerUPP(st_event_callback);
      InstallWindowEventHandler(_toplevel_window, gEvtHandler, 
                                GetEventTypeCount(list1), list1, this, &application_event_ref_ref1);

      ProcessSerialNumber psn = { 0, kCurrentProcess };
      TransformProcessType(&psn, kProcessTransformToForegroundApplication);
      SetFrontProcess(&psn);

      if (_visible) {
        ShowWindow(_toplevel_window);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::set_visible
//       Access: Public, Virtual
//  Description: Makes the splash window visible or invisible, so as
//               not to compete with the embedded Panda window in the
//               same space.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
set_visible(bool visible) {
  P3DSplashWindow::set_visible(visible);

  if (_visible) {
    ShowWindow(_toplevel_window);
  } else {
    HideWindow(_toplevel_window);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::set_image_filename
//       Access: Public, Virtual
//  Description: Specifies the name of a JPEG image file that is
//               displayed in the center of the splash window.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
set_image_filename(const string &image_filename, ImagePlacement image_placement) {
  switch (image_placement) {
  case IP_background:
    load_image(_background_image, image_filename);
    break;

  case IP_button_ready:
    load_image(_button_ready_image, image_filename);
    set_button_range(_button_ready_image);
    break;

  case IP_button_rollover:
    load_image(_button_rollover_image, image_filename);
    break;
   
  case IP_button_click:
    load_image(_button_click_image, image_filename);
    break;
  }

  refresh();
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
  refresh();
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
    refresh();
  }
  _install_progress = install_progress;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::handle_event
//       Access: Public, Virtual
//  Description: Deals with the event callback from the OS window
//               system.  Returns true if the event is handled, false
//               if ignored.
////////////////////////////////////////////////////////////////////
bool P3DOsxSplashWindow::
handle_event(P3D_event_data event) {
  EventRecord *er = event._event;

  // Need to ensure we have the correct port set, in order to
  // convert the mouse coordinates successfully via
  // GlobalToLocal().
  GrafPtr out_port = _wparams.get_parent_window()._port;
  GrafPtr port_save = NULL;
  Boolean port_changed = QDSwapPort(out_port, &port_save);
  
  Point pt = er->where;
  GlobalToLocal(&pt);

  if (port_changed) {
    QDSwapPort(port_save, NULL);
  }
  
  switch (er->what) {
  case updateEvt:
    paint_window();
    break;

  case mouseDown:
    set_mouse_data(_mouse_x, _mouse_y, true);
    break;

  case mouseUp:
    set_mouse_data(_mouse_x, _mouse_y, false);
    break;

  case activateEvt:
    _mouse_active = ((er->modifiers & 1) != 0);
    break;

  default:
    break;
  }

  if (_mouse_active) {
    set_mouse_data(pt.h, pt.v, _mouse_down);
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::refresh
//       Access: Protected, Virtual
//  Description: Requests that the window will be repainted.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
refresh() {
  if (!_visible) {
    return;
  }
  if (_toplevel_window != NULL) {
    Rect r = { 0, 0, _win_height, _win_width }; 
    InvalWindowRect(_toplevel_window, &r);
    
  } else {
    _inst->request_refresh();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::paint_window
//       Access: Private
//  Description: Redraws the current splash window.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
paint_window() {
  if (!_visible) {
    return;
  }

  GrafPtr out_port = NULL;
  if (_toplevel_window != NULL) {
    GetPort(&out_port);

  } else {
    out_port = _wparams.get_parent_window()._port;
  }

  CGContextRef context;
  OSStatus err = CreateCGContextForPort(out_port, &context);
  if (err != noErr) {
    nout << "Couldn't create CG context\n";
    return;
  }

  //  Adjust for any SetOrigin calls on out_port
  SyncCGContextOriginWithPort(context, out_port);
  
  //  Move the CG origin to the upper left of the port
  Rect port_rect;
  GetPortBounds(out_port, &port_rect);
  CGContextTranslateCTM(context, 0, (float)(port_rect.bottom - port_rect.top));
  
  //  Flip the y axis so that positive Y points down
  CGContextScaleCTM(context, 1.0, -1.0);

  // Clear the whole region to white before beginning.
  static const CGFloat white_components[] = { 1, 1, 1, 1 };
  CGColorSpaceRef rgb_space = CGColorSpaceCreateDeviceRGB();
  CGColorRef white = CGColorCreate(rgb_space, white_components);

  CGRect region = { { 0, 0 }, { _win_width, _win_height } };
  CGContextBeginPath(context);
  CGContextSetFillColorWithColor(context, white);
  CGContextAddRect(context, region);
  CGContextFillPath(context);

  CGColorRelease(white);
  CGColorSpaceRelease(rgb_space);

  // Now paint in the image(s).
  paint_image(context, _background_image);
  switch (_bstate) {
  case BS_hidden:
    break;
  case BS_ready:
    paint_image(context, _button_ready_image);
    break;
  case BS_rollover:
    if (!paint_image(context, _button_rollover_image)) {
      paint_image(context, _button_ready_image);
    }
    break;
  case BS_click:
    if (!paint_image(context, _button_click_image)) {
      paint_image(context, _button_ready_image);
    }
    break;
  }

  // Draw the progress bar.  We don't draw this bar at all unless we
  // have nonzero progress.
  if (_install_progress != 0.0) {
    paint_progress_bar(context);
  }

  CGContextSynchronize(context);
  CGContextRelease(context);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::load_image
//       Access: Private
//  Description: Loads the named image file into an OsxImageData object.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
load_image(OsxImageData &image, const string &image_filename) {
  image.dump_image();
  string string_data;
  if (!read_image_data(image, string_data, image_filename)) {
    return;
  }

  // Now we need to copy from the RGB (or RGBA) source image into the
  // BGRA target image.  We also flip the image upside-down here to
  // compensate for the upside-down vertical scale on
  // CGContextScaleCTM.
  int row_stride = image._width * image._num_channels;
  int new_row_stride = image._width * 4;
  image._raw_data = new char[new_row_stride * image._height];
  for (int yi = 0; yi < image._height; ++yi) {
    char *dest = image._raw_data + yi * new_row_stride;
    const char *source = string_data.data() + (image._height - 1 - yi) * row_stride;
    if (image._num_channels == 3) {
      // Source is RGB.
      for (int xi = 0; xi < image._width; ++xi) {
        char r = source[0];
        char g = source[1];
        char b = source[2];
        dest[0] = b;
        dest[1] = g;
        dest[2] = r;
        dest[3] = 0xff;
        source += 3;
        dest += 4;
      }
    } else if (image._num_channels == 4) {
      // Source is RGBA.
      for (int xi = 0; xi < image._width; ++xi) {
        char r = source[0];
        char g = source[1];
        char b = source[2];
        char a = source[3];
        // Little-endian.
        dest[0] = b;
        dest[1] = g;
        dest[2] = r;
        dest[3] = a;
        source += 4;
        dest += 4;
      }
    }
  }

  image._data =
    CFDataCreateWithBytesNoCopy(NULL, (const UInt8 *)image._raw_data, 
                                image._height * new_row_stride, kCFAllocatorNull);
  image._provider = CGDataProviderCreateWithCFData(image._data);
  image._color_space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

  image._image =
    CGImageCreate(image._width, image._height, 8, 32, 
                  new_row_stride, image._color_space,
                  kCGImageAlphaFirst | kCGBitmapByteOrder32Little, 
                  image._provider, NULL, false, kCGRenderingIntentDefault);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::paint_image
//       Access: Private
//  Description: Draws the indicated image, centered within the
//               window.  Returns true on success, false if the image
//               is not defined.
////////////////////////////////////////////////////////////////////
bool P3DOsxSplashWindow::
paint_image(CGContextRef context, const OsxImageData &image) {
  if (image._image == NULL) {
    return false;
  }
    
  // Determine the relative size of image and window.
  int win_cx = _win_width / 2;
  int win_cy = _win_height / 2;

  CGRect rect = { { 0, 0 }, { 0, 0 } };
    
  if (image._width <= _win_width && image._height <= _win_height) {
    // The bitmap fits within the window; center it.
      
    // This is the top-left corner of the bitmap in window coordinates.
    int p_x = win_cx - image._width / 2;
    int p_y = win_cy - image._height / 2;

    rect.origin.x += p_x;
    rect.origin.y += p_y;
    rect.size.width = image._width;
    rect.size.height = image._height;
      
  } else {
    // The bitmap is larger than the window; scale it down.
    double x_scale = (double)_win_width / (double)image._width;
    double y_scale = (double)_win_height / (double)image._height;
    double scale = min(x_scale, y_scale);
    int sc_width = (int)(image._width * scale);
    int sc_height = (int)(image._height * scale);
      
    int p_x = win_cx - sc_width / 2;
    int p_y = win_cy - sc_height / 2;

    rect.origin.x += p_x;
    rect.origin.y += p_y;
    rect.size.width = sc_width;
    rect.size.height = sc_height;
  }

  CGContextDrawImage(context, rect, image._image);
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::paint_progress_bar
//       Access: Private
//  Description: Draws the progress bar and the label within the
//               window.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::
paint_progress_bar(CGContextRef context) {
  // Get some colors we'll need.  We can't just use
  // CGColorGetConstantColor(), since that requires 10.5.
  static const CGFloat black_components[] = { 0, 0, 0, 1 };
  static const CGFloat white_components[] = { 1, 1, 1, 1 };
  static const CGFloat blue_components[] = { 0.424, 0.647, 0.878, 1 };
  CGColorSpaceRef rgb_space = CGColorSpaceCreateDeviceRGB();
  CGColorRef black = CGColorCreate(rgb_space, black_components);
  CGColorRef white = CGColorCreate(rgb_space, white_components);
  CGColorRef blue = CGColorCreate(rgb_space, blue_components);

  int bar_x, bar_y, bar_width, bar_height;
  get_bar_placement(bar_x, bar_y, bar_width, bar_height);

  // We offset the bar by half a pixel, so we'll be drawing the
  // one-pixel line through the middle of a pixel, and it won't try to
  // antialias itself into a half-black two-pixel line.
  float bar_xf = bar_x + 0.5;
  float bar_yf = bar_y - 0.5;

  CGRect bar = { { bar_xf, bar_yf }, { bar_width, bar_height } };

  // Clear the entire progress bar to white.
  CGContextBeginPath(context);
  CGContextSetFillColorWithColor(context, white);
  CGContextAddRect(context, bar);
  CGContextFillPath(context);

  // Draw the interior of the progress bar in blue.
  int progress_width = (int)((bar_width - 2) * _install_progress);
  if (progress_width != 0) {
    CGRect prog = { { bar_xf, bar_yf }, { progress_width, bar_height } };
    CGContextBeginPath(context);
    CGContextSetFillColorWithColor(context, blue);
    CGContextAddRect(context, prog);
    CGContextFillPath(context);
  }

  // Draw the black stroke around the progress bar.
  CGContextBeginPath(context);
  CGContextSetLineWidth(context, 1);
  CGContextSetStrokeColorWithColor(context, black);
  CGContextAddRect(context, bar);
  CGContextStrokePath(context);

  if (!_install_label.empty()) {
    // Now draw the install_label right above it.

    CGContextSetTextMatrix(context, CGContextGetCTM(context));

    // Choose a suitable font.
    float text_height = 15.0;
    CGContextSelectFont(context, "Helvetica", text_height, kCGEncodingMacRoman);

    // Measure the text, for centering.
    CGContextSetTextPosition(context, 0, 0);
    CGContextSetTextDrawingMode(context, kCGTextInvisible);
    CGContextShowText(context, _install_label.data(), _install_label.length());
    CGPoint end_point = CGContextGetTextPosition(context);
    float text_width = end_point.x;

    int text_x = (_win_width - text_width) / 2;
    int text_y = bar_y - text_height * 1.5;

    // Clear the rectangle behind the text to white.
    CGRect text_rect = { { text_x - 2, text_y - 2 }, { text_width + 4, text_height + 4 } };

    CGContextBeginPath(context);
    CGContextAddRect(context, text_rect);
    CGContextSetFillColorWithColor(context, white);
    CGContextFillPath(context);

    // And finally, draw the text.
    CGContextSetTextDrawingMode(context, kCGTextFill);
    CGContextSetFillColorWithColor(context, black);

    CGContextShowTextAtPoint(context, text_x, text_y + text_height, 
                             _install_label.data(), _install_label.length());
  }

  CGColorRelease(blue);
  CGColorRelease(white);
  CGColorRelease(black);
  CGColorSpaceRelease(rgb_space);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::st_event_callback
//       Access: Private, Static
//  Description: The event callback on the toplevel window.
////////////////////////////////////////////////////////////////////
pascal OSStatus P3DOsxSplashWindow::
st_event_callback(EventHandlerCallRef my_handler, EventRef event, 
                  void *user_data) {
  return ((P3DOsxSplashWindow *)user_data)->event_callback(my_handler, event);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::event_callback
//       Access: Private
//  Description: The event callback on the toplevel window.
////////////////////////////////////////////////////////////////////
OSStatus P3DOsxSplashWindow::
event_callback(EventHandlerCallRef my_handler, EventRef event) {
  OSStatus result = eventNotHandledErr;

  WindowRef window = NULL; 
  UInt32 the_class = GetEventClass(event);
  UInt32 kind = GetEventKind(event);
  GetEventParameter(event, kEventParamWindowRef, typeWindowRef, NULL, 
                    sizeof(WindowRef), NULL, (void*) &window);
  switch (the_class) {
  case kEventClassWindow:
    switch (kind) {
    case kEventWindowBoundsChanged:
      // If the window changes size, we have to repaint it.
      refresh();

      // Also determine the new size.
      {
        Rect port_rect;
        GrafPtr out_port = GetWindowPort(_toplevel_window);
        GetPortBounds(out_port, &port_rect);
        int width = port_rect.right - port_rect.left;
        int height = port_rect.bottom - port_rect.top;
        if (width != _win_width || height != _win_height) {
          _win_width = width;
          _win_height = height;
          set_button_range(_button_ready_image);
        }
      }

      // We seem to get the mouse-down, but lose the mouse-up, event
      // in this case, so infer it.
      set_mouse_data(_mouse_x, _mouse_y, false);
      result = noErr;
      break;

    case kEventWindowDrawContent:
      paint_window();
      break;

    case kEventWindowClose:
      // When the user closes the splash window, stop the instance.
      _inst->request_stop_sub_thread();
      break;
    };
    break;

  case kEventClassMouse:
    switch (kind) {
    case kEventMouseUp:
      set_mouse_data(_mouse_x, _mouse_y, false);
      break;

    case kEventMouseDown:
      set_mouse_data(_mouse_x, _mouse_y, true);
      break;

    case kEventMouseMoved:
    case kEventMouseDragged:
      {
        Point point;
        GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL,
                          sizeof(Point), NULL, (void *)&point);

        GrafPtr port = _wparams.get_parent_window()._port;
        if (_toplevel_window != NULL) {
          port = GetWindowPort(_toplevel_window);
        }
        GrafPtr port_save = NULL;
        Boolean port_changed = QDSwapPort(port, &port_save);
        GlobalToLocal(&point);
        if (port_changed) {
          QDSwapPort(port_save, NULL);
        }

        set_mouse_data(point.h, point.v, _mouse_down);
      }
      break;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::OsxImageData::dump_image
//       Access: Public
//  Description: Frees the previous image data.
////////////////////////////////////////////////////////////////////
void P3DOsxSplashWindow::OsxImageData::
dump_image() {
  if (_image != NULL) {
    CGImageRelease(_image);
    _image = NULL;
  }
  if (_color_space != NULL) {
    CGColorSpaceRelease(_color_space);
    _color_space = NULL;
  }
  if (_provider != NULL) {
    CGDataProviderRelease(_provider);
    _provider = NULL;
  }
  if (_data != NULL) {
    CFRelease(_data);
    _data = NULL;
  }
  if (_raw_data != NULL) {
    delete[] _raw_data;
    _raw_data = NULL;
  }
}

#endif  // __APPLE__
