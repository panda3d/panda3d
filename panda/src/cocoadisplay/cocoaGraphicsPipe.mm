/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGraphicsPipe.mm
 * @author rdb
 * @date 2012-05-14
 */

#include "cocoaGraphicsPipe.h"
#include "cocoaGraphicsBuffer.h"
#include "cocoaGraphicsWindow.h"
#include "cocoaGraphicsStateGuardian.h"
#include "config_cocoadisplay.h"
#include "frameBufferProperties.h"
#include "displayInformation.h"

#import <Foundation/NSAutoreleasePool.h>
#import <AppKit/NSApplication.h>
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
#import <AppKit/NSRunningApplication.h>
#endif

#include <mach-o/arch.h>

TypeHandle CocoaGraphicsPipe::_type_handle;

/**
 * Takes a CoreGraphics display ID, which defaults to the main display.
 */
CocoaGraphicsPipe::
CocoaGraphicsPipe(CGDirectDisplayID display) : _display(display) {
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
  _is_valid = true;

  [[NSAutoreleasePool alloc] init];

  // Put Cocoa into thread-safe mode by spawning a thread which immediately
  // exits.
  NSThread* thread = [[NSThread alloc] init];
  [thread start];
  [thread autorelease];

  // We used to also obtain the corresponding NSScreen here, but this causes
  // the application icon to start bouncing, which may be undesirable for
  // apps that will never open a window.

  _display_width = CGDisplayPixelsWide(_display);
  _display_height = CGDisplayPixelsHigh(_display);
  load_display_information();

  if (cocoadisplay_cat.is_debug()) {
    cocoadisplay_cat.debug()
      << "Creating CocoaGraphicsPipe for display ID " << _display << "\n";
  }
}

/**
 * Fills in _display_information.
 */
void CocoaGraphicsPipe::
load_display_information() {
  _display_information->_vendor_id = CGDisplayVendorNumber(_display);
  // _display_information->_device_id = CGDisplayUnitNumber(_display);
  // _display_information->_device_id = CGDisplaySerialNumber(_display);

  // Display modes
  size_t num_modes = 0;
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
  CFArrayRef modes = CGDisplayCopyAllDisplayModes(_display, NULL);
  if (modes != NULL) {
    num_modes = CFArrayGetCount(modes);
    _display_information->_total_display_modes = num_modes;
    _display_information->_display_mode_array = new DisplayMode[num_modes];
  }

  for (size_t i = 0; i < num_modes; ++i) {
    CGDisplayModeRef mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);

    _display_information->_display_mode_array[i].width = CGDisplayModeGetWidth(mode);
    _display_information->_display_mode_array[i].height = CGDisplayModeGetHeight(mode);
    _display_information->_display_mode_array[i].refresh_rate = CGDisplayModeGetRefreshRate(mode);
    _display_information->_display_mode_array[i].fullscreen_only = false;

    // Read number of bits per pixels from the pixel encoding
    CFStringRef encoding = CGDisplayModeCopyPixelEncoding(mode);
    if (CFStringCompare(encoding, CFSTR(kIO64BitDirectPixels),
                              kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
      _display_information->_display_mode_array[i].bits_per_pixel = 64;

    } else if (CFStringCompare(encoding, CFSTR(kIO32BitFloatPixels),
                              kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
      _display_information->_display_mode_array[i].bits_per_pixel = 32;

    } else if (CFStringCompare(encoding, CFSTR(kIO16BitFloatPixels),
                              kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
      _display_information->_display_mode_array[i].bits_per_pixel = 16;

    } else if (CFStringCompare(encoding, CFSTR(IOYUV422Pixels),
                              kCFCompareCaseInsensitive) == kCFCompareEqualTo ||
               CFStringCompare(encoding, CFSTR(IO8BitOverlayPixels),
                              kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
      _display_information->_display_mode_array[i].bits_per_pixel = 8;
    } else {
      // The other possible pixel formats in IOKitIOGraphicsTypes.h have
      // strings like "PPPP" or "-RRRRRGGGGGBBBBB", so the number of bits per
      // pixel can be deduced from the string length.  Nifty!
      _display_information->_display_mode_array[i].bits_per_pixel = CFStringGetLength(encoding);
    }
    CFRelease(encoding);
  }
  if (modes != NULL) {
    CFRelease(modes);
  }

#else
  CFArrayRef modes = CGDisplayAvailableModes(_display);
  if (modes != NULL) {
    num_modes = CFArrayGetCount(modes);
    _display_information->_total_display_modes = num_modes;
    _display_information->_display_mode_array = new DisplayMode[num_modes];
  }

  for (size_t i = 0; i < num_modes; ++i) {
    CFDictionaryRef mode = (CFDictionaryRef) CFArrayGetValueAtIndex(modes, i);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayWidth),
      kCFNumberIntType, &_display_information->_display_mode_array[i].width);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayHeight),
      kCFNumberIntType, &_display_information->_display_mode_array[i].height);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayBitsPerPixel),
      kCFNumberIntType, &_display_information->_display_mode_array[i].bits_per_pixel);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayRefreshRate),
      kCFNumberIntType, &_display_information->_display_mode_array[i].refresh_rate);

    _display_information->_display_mode_array[i].fullscreen_only = false;
  }
#endif

  // Get processor information
  const NXArchInfo *ainfo = NXGetLocalArchInfo();
  _display_information->_cpu_brand_string = strdup(ainfo->description);

  // Get version of Mac OS X
  SInt32 major, minor, bugfix;
  Gestalt(gestaltSystemVersionMajor, &major);
  Gestalt(gestaltSystemVersionMinor, &minor);
  Gestalt(gestaltSystemVersionBugFix, &bugfix);
  _display_information->_os_version_major = major;
  _display_information->_os_version_minor = minor;
  _display_information->_os_version_build = bugfix;
}

/**
 *
 */
CocoaGraphicsPipe::
~CocoaGraphicsPipe() {
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string CocoaGraphicsPipe::
get_interface_name() const {
  return "OpenGL";
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default CocoaGraphicsPipe.
 */
PT(GraphicsPipe) CocoaGraphicsPipe::
pipe_constructor() {
  return new CocoaGraphicsPipe;
}

/**
 * Returns an indication of the thread in which this GraphicsPipe requires its
 * window processing to be performed: typically either the app thread (e.g.
 * X) or the draw thread (Windows).
 */
GraphicsPipe::PreferredWindowThread
CocoaGraphicsPipe::get_preferred_window_thread() const {
  // The NSView and NSWindow classes are not completely thread-safe, they can
  // only be called from the main thread!
  return PWT_app;
}

/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) CocoaGraphicsPipe::
make_output(const std::string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {

  if (!_is_valid) {
    return NULL;
  }

  CocoaGraphicsStateGuardian *cocoagsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(cocoagsg, gsg, NULL);
  }

  // First thing to try: a CocoaGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)||
        ((flags&BF_can_bind_layered)!=0)) {
      return NULL;
    }
    return new CocoaGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                   flags, gsg, host);
  }

  // Second thing to try: a GLGraphicsBuffer.  This requires a context, so if
  // we don't have a host window, we instead create a CocoaGraphicsBuffer,
  // which wraps around GLGraphicsBuffer and manages a context.

  if (retry == 1) {
    if (!gl_support_fbo ||
        (flags & (BF_require_parasite | BF_require_window)) != 0) {
      return NULL;
    }
    // Early failure - if we are sure that this buffer WONT meet specs, we can
    // bail out early.
    if ((flags & BF_fb_props_optional) == 0) {
      if (fb_prop.get_indexed_color() ||
          fb_prop.get_back_buffers() > 0 ||
          fb_prop.get_accum_bits() > 0) {
        return NULL;
      }
    }
    if (cocoagsg != NULL && cocoagsg->is_valid() && !cocoagsg->needs_reset()) {
      if (!cocoagsg->_supports_framebuffer_object ||
          cocoagsg->_glDrawBuffers == NULL) {
        return NULL;
      } else if (fb_prop.is_basic()) {
        // Early success - if we are sure that this buffer WILL meet specs, we
        // can precertify it.
        precertify = true;
      }
    }
    if (host != NULL) {
      return new GLGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                  flags, gsg, host);
    } else {
      return new CocoaGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                     flags, gsg, host);
    }
  }

  // Nothing else left to try.
  return NULL;
}

/**
 * This is called when make_output() is used to create a
 * CallbackGraphicsWindow.  If the GraphicsPipe can construct a GSG that's not
 * associated with any particular window object, do so now, assuming the
 * correct graphics context has been set up externally.
 */
PT(GraphicsStateGuardian) CocoaGraphicsPipe::
make_callback_gsg(GraphicsEngine *engine) {
  return new CocoaGraphicsStateGuardian(engine, this, NULL);
}
