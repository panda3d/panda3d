/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dOsxSplashWindow.cxx
 * @author drose
 * @date 2009-07-16
 */

#include "p3dOsxSplashWindow.h"

#if defined(__APPLE__) && !__LP64__

#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>

#ifndef CGFLOAT_DEFINED
  #if defined(__LP64__) && __LP64__
    typedef double CGFloat;
  #else
    typedef float CGFloat;
  #endif
#endif

using std::string;

/**
 *
 */
P3DOsxSplashWindow::
P3DOsxSplashWindow(P3DInstance *inst, bool make_visible) :
  P3DSplashWindow(inst, make_visible)
{
  _font_attribs = nullptr;
  _install_progress = 0;
  _progress_known = true;
  _received_data = 0;
  _got_wparams = false;
  // We have to start with _mouse_active true; firefox doesn't send activate
  // events.
  _mouse_active = true;
  _toplevel_window = nullptr;
}

/**
 *
 */
P3DOsxSplashWindow::
~P3DOsxSplashWindow() {
  if (_toplevel_window != nullptr) {
    SetWRefCon(_toplevel_window, 0);
    HideWindow(_toplevel_window);
    DisposeWindow(_toplevel_window);
    _toplevel_window = nullptr;
  }
  if (_font_attribs != nullptr) {
    CFRelease(_font_attribs);
  }
}

/**
 * Changes the window parameters, e.g.  to resize or reposition the window; or
 * sets the parameters for the first time, creating the initial window.
 */
void P3DOsxSplashWindow::
set_wparams(const P3DWindowParams &wparams) {
  P3DSplashWindow::set_wparams(wparams);
  _got_wparams = true;

  if (_wparams.get_window_type() == P3D_WT_toplevel ||
      _wparams.get_window_type() == P3D_WT_fullscreen) {
    // Creating a toplevel splash window.
    if (_toplevel_window == nullptr) {
      Rect r;
      r.top = _wparams.get_win_y();
      r.left = _wparams.get_win_x();

      // These are the same defaults used by Panda's osxGraphicsWindow.
      if (r.left == -1) r.left = 10;
      if (r.top == -1) r.top = 50;
      // A coordinate of -2 means to center the window on the screen.
      CGRect display_bounds = CGDisplayBounds(kCGDirectMainDisplay);
      if (r.left == -2) {
        r.left = (short)(0.5 * (CGRectGetWidth(display_bounds) - _win_width));
      }
      if (r.top == -2) {
        r.top = (short)(0.5 * (CGRectGetHeight(display_bounds) - _win_height));
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

  // Determine the attributes of the font we're going to use.
  int symbolic = 0;
  if (_font_style != FS_normal) {
    symbolic |= kCTFontItalicTrait;
  }
  if (_font_weight > 500) {
    symbolic |= kCTFontBoldTrait;
  }
  CFNumberRef symbolic_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &symbolic);

  CFStringRef traits_keys[1] = { kCTFontSymbolicTrait };
  CFTypeRef traits_values[1] = { symbolic_ref };
  CFDictionaryRef traits = CFDictionaryCreate(kCFAllocatorDefault, (const void **)&traits_keys, (const void **)&traits_values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

  CFStringRef family = CFStringCreateWithCString(nullptr, _font_family.c_str(), kCFStringEncodingUTF8);

  CFStringRef attribs_keys[2] = { kCTFontFamilyNameAttribute, kCTFontTraitsAttribute };
  CFTypeRef attribs_values[2] = { family, traits };
  CFDictionaryRef attribs = CFDictionaryCreate(kCFAllocatorDefault, (const void **)&attribs_keys, (const void **)&attribs_values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

  // Create the font object.
  CTFontDescriptorRef font_desc = CTFontDescriptorCreateWithAttributes(attribs);
  CTFontRef font = CTFontCreateWithFontDescriptor(font_desc, _font_size, nullptr);

  CFStringRef keys[1] = { kCTFontAttributeName };
  CFTypeRef values[1] = { font };
  _font_attribs = CFDictionaryCreate(kCFAllocatorDefault, (const void **)&keys, (const void **)&values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

  CFRelease(font);
  CFRelease(font_desc);
  CFRelease(attribs);
  CFRelease(traits);
  CFRelease(family);
  CFRelease(symbolic_ref);
}

/**
 * Makes the splash window visible or invisible, so as not to compete with the
 * embedded Panda window in the same space.
 */
void P3DOsxSplashWindow::
set_visible(bool visible) {
  P3DSplashWindow::set_visible(visible);

  if (_toplevel_window != nullptr) {
    if (_visible) {
      ShowWindow(_toplevel_window);
    } else {
      HideWindow(_toplevel_window);
    }
  }
  refresh();
}

/**
 * Specifies the name of a JPEG image file that is displayed in the center of
 * the splash window.
 */
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

  default:
    return;
  }

  refresh();
}

/**
 * Specifies the text that is displayed above the install progress bar.
 */
void P3DOsxSplashWindow::
set_install_label(const string &install_label) {
  _install_label = install_label;
  refresh();
}

/**
 * Moves the install progress bar from 0.0 to 1.0.
 */
void P3DOsxSplashWindow::
set_install_progress(double install_progress,
                     bool is_progress_known, size_t received_data) {
  // Only request a refresh if we're changing substantially.
  if (is_progress_known != _progress_known) {
    refresh();
  } else if (_progress_known) {
    if ((int)(install_progress * 500.0) != (int)(_install_progress * 500.0)) {
      refresh();
    }
  } else {
    if ((int)(received_data * _unknown_progress_rate) !=
        (int)(_received_data * _unknown_progress_rate)) {
      refresh();
    }
  }

  _install_progress = install_progress;
  _progress_known = is_progress_known;
  _received_data = received_data;
}

/**
 * Deals with the event callback from the OS window system.  Returns true if
 * the event is handled, false if ignored.
 */
bool P3DOsxSplashWindow::
handle_event(const P3D_event_data &event) {
  bool retval = false;

  if (event._event_type == P3D_ET_osx_event_record) {
    retval = handle_event_osx_event_record(event);
  } else if (event._event_type == P3D_ET_osx_cocoa) {
    retval = handle_event_osx_cocoa(event);
  } else {
    assert(false);
  }

  return retval;
}

/**
 * Requests that the window will be repainted.
 */
void P3DOsxSplashWindow::
refresh() {
  if (!_visible) {
    return;
  }
  if (_toplevel_window != nullptr) {
    Rect r = { 0, 0, (short)_win_height, (short)_win_width };
    InvalWindowRect(_toplevel_window, &r);

  } else {
    _inst->request_refresh();
  }
}

/**
 * Redraws the current splash window.
 */
void P3DOsxSplashWindow::
paint_window() {
  if (!_visible) {
    return;
  }

  if (_toplevel_window != nullptr ||
      _wparams.get_parent_window()._window_handle_type == P3D_WHT_osx_port) {

    // The old QuickDraw-style window handle.  We use CreateCGContextForPort()
    // to map this to the new CoreGraphics-style.
    GrafPtr out_port = nullptr;
    if (_toplevel_window != nullptr) {
      GetPort(&out_port);

    } else {
      const P3D_window_handle &handle = _wparams.get_parent_window();
      assert(handle._window_handle_type == P3D_WHT_osx_port);
      out_port = handle._handle._osx_port._port;
    }

    CGContextRef context;
    OSStatus err = CreateCGContextForPort(out_port, &context);
    if (err != noErr) {
      nout << "Couldn't create CG context\n";
      return;
    }

    // Adjust for any SetOrigin calls on out_port
    SyncCGContextOriginWithPort(context, out_port);

    // Move the CG origin to the upper left of the port
    Rect port_rect;
    GetPortBounds(out_port, &port_rect);
    CGContextTranslateCTM(context, 0, (float)(port_rect.bottom - port_rect.top));

    // Flip the y axis so that positive Y points down
    CGContextScaleCTM(context, 1.0, -1.0);

    paint_window_osx_cgcontext(context);

    // We need to synchronize, or we don't see the update every frame.
    CGContextSynchronize(context);
    CGContextRelease(context);

  } else {
    // The new CoreGraphics-style window handle.  We can draw to this
    // directly.

    const P3D_window_handle &handle = _wparams.get_parent_window();
    assert(handle._window_handle_type == P3D_WHT_osx_cgcontext);
    CGContextRef context = handle._handle._osx_cgcontext._context;

    paint_window_osx_cgcontext(context);
  }
}

/**
 * Redraws the current splash window, using the new CoreGraphics interface.
 */
void P3DOsxSplashWindow::
paint_window_osx_cgcontext(CGContextRef context) {
  // Clear the whole region to the background color before beginning.
  CGFloat bg_components[] = { _bgcolor_r / 255.0f, _bgcolor_g / 255.0f, _bgcolor_b / 255.0f, 1 };
  CGColorSpaceRef rgb_space = CGColorSpaceCreateDeviceRGB();
  CGColorRef bg = CGColorCreate(rgb_space, bg_components);

  CGRect region = { { 0, 0 }, { (CGFloat)_win_width, (CGFloat)_win_height } };
  CGContextSetFillColorWithColor(context, bg);
  CGContextFillRect(context, region);

  CGColorRelease(bg);
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

  // Draw the progress bar.  We don't draw this bar at all unless we have
  // nonzero progress.
  if (!_progress_known || _install_progress != 0.0) {
    paint_progress_bar(context);
  }
}

/**
 * Responds to the deprecated Carbon event types in Mac OSX.
 */
bool P3DOsxSplashWindow::
handle_event_osx_event_record(const P3D_event_data &event) {
  assert(event._event_type == P3D_ET_osx_event_record);
  EventRecord *er = event._event._osx_event_record._event;

  Point pt = er->where;

  // Need to ensure we have the correct port set, in order to convert the
  // mouse coordinates successfully via GlobalToLocal().
  const P3D_window_handle &handle = _wparams.get_parent_window();
  if (handle._window_handle_type == P3D_WHT_osx_port) {
    GrafPtr out_port = handle._handle._osx_port._port;
    GrafPtr port_save = nullptr;
    Boolean port_changed = QDSwapPort(out_port, &port_save);

    GlobalToLocal(&pt);

    if (port_changed) {
      QDSwapPort(port_save, nullptr);
    }

  } else if (handle._window_handle_type == P3D_WHT_osx_cgcontext) {
    // First, convert the coordinates from screen coordinates to browser
    // window coordinates.
    WindowRef window = handle._handle._osx_cgcontext._window;
    CGPoint cgpt = { (CGFloat)pt.h, (CGFloat)pt.v };
    HIPointConvert(&cgpt, kHICoordSpaceScreenPixel, nullptr,
                   kHICoordSpaceWindow, window);

    // Then convert to plugin coordinates.
    pt.h = (short)(cgpt.x - _wparams.get_win_x());
    pt.v = (short)(cgpt.y - _wparams.get_win_y());
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

/**
 * Responds to the new Cocoa event types in Mac OSX.
 */
bool P3DOsxSplashWindow::
handle_event_osx_cocoa(const P3D_event_data &event) {
  assert(event._event_type == P3D_ET_osx_cocoa);
  const P3DCocoaEvent &ce = event._event._osx_cocoa._event;

  switch (ce.type) {
  case P3DCocoaEventDrawRect:
    if (_visible) {
      CGContextRef context = ce.data.draw.context;
      paint_window_osx_cgcontext(context);
      return true;
    } else {
      return false;
    }

  case P3DCocoaEventMouseDown:
    set_mouse_data((int)ce.data.mouse.pluginX, (int)ce.data.mouse.pluginY, true);
    return true;

  case P3DCocoaEventMouseUp:
    set_mouse_data((int)ce.data.mouse.pluginX, (int)ce.data.mouse.pluginY, false);
    return true;

  case P3DCocoaEventMouseMoved:
  case P3DCocoaEventMouseDragged:
    set_mouse_data((int)ce.data.mouse.pluginX, (int)ce.data.mouse.pluginY, _mouse_down);
    return true;

  case P3DCocoaEventFocusChanged:
    _mouse_active = (ce.data.focus.hasFocus != 0);
    return true;

  default:
    return false;
  }
}

/**
 * Loads the named image file into an OsxImageData object.
 */
void P3DOsxSplashWindow::
load_image(OsxImageData &image, const string &image_filename) {
  image.dump_image();
  string string_data;
  if (!read_image_data(image, string_data, image_filename)) {
    return;
  }

  // Now we need to copy from the RGB (or RGBA) source image into the BGRA
  // target image.  We also flip the image upside-down here to compensate for
  // the upside-down vertical scale on CGContextScaleCTM.
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
    CFDataCreateWithBytesNoCopy(nullptr, (const UInt8 *)image._raw_data,
                                image._height * new_row_stride, kCFAllocatorNull);
  image._provider = CGDataProviderCreateWithCFData(image._data);
  image._color_space = CGColorSpaceCreateDeviceRGB();

  image._image =
    CGImageCreate(image._width, image._height, 8, 32,
                  new_row_stride, image._color_space,
                  kCGImageAlphaFirst | kCGBitmapByteOrder32Little,
                  image._provider, nullptr, false, kCGRenderingIntentDefault);
}

/**
 * Draws the indicated image, centered within the window.  Returns true on
 * success, false if the image is not defined.
 */
bool P3DOsxSplashWindow::
paint_image(CGContextRef context, const OsxImageData &image) {
  if (image._image == nullptr) {
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
    double scale = std::min(x_scale, y_scale);
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

/**
 * Draws the progress bar and the label within the window.
 */
void P3DOsxSplashWindow::
paint_progress_bar(CGContextRef context) {
  CGFloat fg_components[] = { _fgcolor_r / 255.0f, _fgcolor_g / 255.0f, _fgcolor_b / 255.0f, 1 };
  CGFloat bg_components[] = { _bgcolor_r / 255.0f, _bgcolor_g / 255.0f, _bgcolor_b / 255.0f, 1 };
  CGFloat bar_components[] = { _barcolor_r / 255.0f, _barcolor_g / 255.0f, _barcolor_b / 255.0f, 1 };
  CGFloat bar_bg_components[] = { _bar_bgcolor_r / 255.0f, _bar_bgcolor_g / 255.0f, _bar_bgcolor_b / 255.0f, 1 };
  CGColorSpaceRef rgb_space = CGColorSpaceCreateDeviceRGB();
  CGColorRef fg = CGColorCreate(rgb_space, fg_components);
  CGColorRef bg = CGColorCreate(rgb_space, bg_components);
  CGColorRef bar = CGColorCreate(rgb_space, bar_components);
  CGColorRef bar_bg = CGColorCreate(rgb_space, bar_bg_components);

  // Get the proper placement for the progress bar.
  int bar_x, bar_y, bar_width, bar_height;
  get_bar_placement(bar_x, bar_y, bar_width, bar_height);

  CGRect bar_rect = { { (CGFloat)bar_x, (CGFloat)bar_y }, { (CGFloat)bar_width, (CGFloat)bar_height } };

  // Clear the entire progress bar to white (or the background color).
  CGContextSetFillColorWithColor(context, bar_bg);
  CGContextFillRect(context, bar_rect);

  // Draw the interior of the progress bar in blue (or the bar color).
  if (_progress_known) {
    int progress_width = (int)(bar_width * _install_progress + 0.5);
    if (progress_width != 0) {
      CGRect prog = { { (CGFloat)bar_x, (CGFloat)bar_y }, { (CGFloat)progress_width, (CGFloat)bar_height } };
      CGContextSetFillColorWithColor(context, bar);
      CGContextFillRect(context, prog);
    }
  } else {
    // Progress is unknown.  Draw a moving block, not a progress bar filling
    // up.
    int block_width = (int)(bar_width * 0.1 + 0.5);
    int block_travel = bar_width - block_width;
    int progress = (int)(_received_data * _unknown_progress_rate);
    progress = progress % (block_travel * 2);
    if (progress > block_travel) {
      progress = block_travel * 2 - progress;
    }

    CGRect prog = { { (CGFloat)(bar_x + progress), (CGFloat)bar_y }, { (CGFloat)block_width, (CGFloat)bar_height } };
    CGContextSetFillColorWithColor(context, bar);
    CGContextFillRect(context, prog);
  }

  // Draw the black stroke around the progress bar.
  if (_bar_border > 0) {
    // We offset the border by half a pixel, so we'll be drawing the one-pixel
    // line through the middle of a pixel, and it won't try to antialias
    // itself into a half-black two-pixel line.
    CGRect border_rect = { { (CGFloat)(bar_x - 0.5), (CGFloat)(bar_y - 0.5) },
                           { (CGFloat)(bar_width + 1), (CGFloat)(bar_height + 1) } };

    CGContextBeginPath(context);
    CGContextSetLineWidth(context, 1);
    CGContextSetStrokeColorWithColor(context, fg);

    for (int i = 0; i < _bar_border; ++i) {
      CGContextAddRect(context, border_rect);
      border_rect.origin.x -= 1;
      border_rect.origin.y -= 1;
      border_rect.size.width += 2;
      border_rect.size.height += 2;
    }
    CGContextStrokePath(context);
  }

  if (!_install_label.empty()) {
    // Need to invert the text so it won't be upside-down.
    CGAffineTransform text_xform = CGAffineTransformMakeScale(1, -1);
    CGContextSetTextMatrix(context, text_xform);

    // Now draw the install_label right above it.
    CFStringRef string = CFStringCreateWithCString(nullptr, _install_label.c_str(), kCFStringEncodingUTF8);
    CFAttributedStringRef attr_string = CFAttributedStringCreate(nullptr, string, _font_attribs);
    CTLineRef line = CTLineCreateWithAttributedString(attr_string);

    // Determine the placement based on the size of the text.
    CGRect bounds = CTLineGetImageBounds(line, context);
    float text_x = (_win_width - bounds.size.width) / 2.0f - bounds.origin.x;
    float text_y = bar_y + bounds.origin.y - 4 - _bar_border;

    // And finally, draw the text.
    CGContextSetTextPosition(context, text_x, text_y);
    CTLineDraw(line, context);

    CFRelease(line);
    CFRelease(attr_string);
    CFRelease(string);
  }

  CGColorRelease(bar_bg);
  CGColorRelease(bar);
  CGColorRelease(bg);
  CGColorRelease(fg);
  CGColorSpaceRelease(rgb_space);
}

/**
 * The event callback on the toplevel window.
 */
pascal OSStatus P3DOsxSplashWindow::
st_event_callback(EventHandlerCallRef my_handler, EventRef event,
                  void *user_data) {
  return ((P3DOsxSplashWindow *)user_data)->event_callback(my_handler, event);
}

/**
 * The event callback on the toplevel window.
 */
OSStatus P3DOsxSplashWindow::
event_callback(EventHandlerCallRef my_handler, EventRef event) {
  OSStatus result = eventNotHandledErr;

  WindowRef window = nullptr;
  UInt32 the_class = GetEventClass(event);
  UInt32 kind = GetEventKind(event);
  GetEventParameter(event, kEventParamWindowRef, typeWindowRef, nullptr,
                    sizeof(WindowRef), nullptr, (void*) &window);
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

      // We seem to get the mouse-down, but lose the mouse-up, event in this
      // case, so infer it.
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
        GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, nullptr,
                          sizeof(Point), nullptr, (void *)&point);

        GrafPtr port;
        assert(_toplevel_window != nullptr);
        port = GetWindowPort(_toplevel_window);
        GrafPtr port_save = nullptr;
        Boolean port_changed = QDSwapPort(port, &port_save);
        GlobalToLocal(&point);
        if (port_changed) {
          QDSwapPort(port_save, nullptr);
        }

        set_mouse_data(point.h, point.v, _mouse_down);
      }
      break;
    }
  }

  return result;
}

/**
 * Frees the previous image data.
 */
void P3DOsxSplashWindow::OsxImageData::
dump_image() {
  if (_image != nullptr) {
    CGImageRelease(_image);
    _image = nullptr;
  }
  if (_color_space != nullptr) {
    CGColorSpaceRelease(_color_space);
    _color_space = nullptr;
  }
  if (_provider != nullptr) {
    CGDataProviderRelease(_provider);
    _provider = nullptr;
  }
  if (_data != nullptr) {
    CFRelease(_data);
    _data = nullptr;
  }
  if (_raw_data != nullptr) {
    delete[] _raw_data;
    _raw_data = nullptr;
  }
}

#endif  // __APPLE__
