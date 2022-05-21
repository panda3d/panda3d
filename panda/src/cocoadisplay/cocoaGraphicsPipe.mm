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
#include "config_cocoadisplay.h"
#include "displayInformation.h"

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSThread.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSRunningApplication.h>
#import <AppKit/NSScreen.h>

#include <mach-o/arch.h>

TypeHandle CocoaGraphicsPipe::_type_handle;

/**
 * Takes a CoreGraphics display ID, which defaults to the main display.
 */
CocoaGraphicsPipe::
CocoaGraphicsPipe(CGDirectDisplayID display) : _display(display) {
  [[NSAutoreleasePool alloc] init];

  // Put Cocoa into thread-safe mode by spawning a thread which immediately
  // exits.
  NSThread* thread = [[NSThread alloc] init];
  [thread start];
  [thread autorelease];

  // If the application is dpi-aware, iterate over all the screens to find the
  // one with our display ID and get the backing scale factor to configure the
  // detected display zoom. Otherwise the detected display zoom keeps its
  // default value of 1.0

  if (dpi_aware) {
    NSScreen *screen;
    NSEnumerator *e = [[NSScreen screens] objectEnumerator];
    while (screen = (NSScreen *) [e nextObject]) {
      NSNumber *num = [[screen deviceDescription] objectForKey: @"NSScreenNumber"];
      if (_display == (CGDirectDisplayID) [num longValue]) {
        set_detected_display_zoom([screen backingScaleFactor]);
        if (cocoadisplay_cat.is_debug()) {
          cocoadisplay_cat.debug()
            << "Display zoom is " << [screen backingScaleFactor] << "\n";
        }
        break;
      }
    }
  }

  // We used to also obtain the corresponding NSScreen here, but this causes
  // the application icon to start bouncing, which may be undesirable for
  // apps that will never open a window.

  // Although the name of these functions mention pixels, they actually return
  // display points, we use the detected display zoom to transform the values
  // into pixels.
  _display_width = CGDisplayPixelsWide(_display) * _detected_display_zoom;
  _display_height = CGDisplayPixelsHigh(_display) * _detected_display_zoom;
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
  CFDictionaryRef options = NULL;
  const CFStringRef dictkeys[] = {kCGDisplayShowDuplicateLowResolutionModes};
  const CFBooleanRef dictvalues[] = {kCFBooleanTrue};
  options = CFDictionaryCreate(NULL,
                               (const void **)dictkeys,
                               (const void **)dictvalues,
                               1,
                               &kCFCopyStringDictionaryKeyCallBacks,
                               &kCFTypeDictionaryValueCallBacks);
  size_t num_modes = 0;
  CFArrayRef modes = CGDisplayCopyAllDisplayModes(_display, options);
  if (modes != NULL) {
    num_modes = CFArrayGetCount(modes);
    _display_information->_total_display_modes = num_modes;
    _display_information->_display_mode_array = new DisplayMode[num_modes];
  }
  if (options != NULL) {
    CFRelease(options);
  }

  for (size_t i = 0; i < num_modes; ++i) {
    CGDisplayModeRef mode = (CGDisplayModeRef) CFArrayGetValueAtIndex(modes, i);

    if (dpi_aware) {
      _display_information->_display_mode_array[i].width = CGDisplayModeGetPixelWidth(mode);
      _display_information->_display_mode_array[i].height = CGDisplayModeGetPixelHeight(mode);
    } else {
      _display_information->_display_mode_array[i].width = CGDisplayModeGetWidth(mode);
      _display_information->_display_mode_array[i].height = CGDisplayModeGetHeight(mode);
    }
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
  if (modes != nullptr) {
    CFRelease(modes);
  }

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
