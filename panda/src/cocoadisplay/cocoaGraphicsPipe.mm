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

  // It takes a while to fire up the display link, so let's fire it up now if
  // we expect to need VSync.
  if (sync_video) {
    uint32_t counter;
    init_vsync(counter);
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
  int32_t current_mode_id = -1;
  CFArrayRef modes = CGDisplayCopyAllDisplayModes(_display, options);
  if (modes != NULL) {
    num_modes = CFArrayGetCount(modes);
    _display_information->_total_display_modes = num_modes;
    _display_information->_display_mode_array = new DisplayMode[num_modes];

    // Get information about the current mode.
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(_display);
    if (mode) {
      current_mode_id = CGDisplayModeGetIODisplayModeID(mode);
      CGDisplayModeRelease(mode);
    }
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

    if (current_mode_id >= 0 && current_mode_id == CGDisplayModeGetIODisplayModeID(mode)) {
      _display_information->_current_display_mode_index = i;

      // Stop checking
      current_mode_id = -1;
    }

    CFRelease(encoding);
  }
  if (modes != nullptr) {
    CFRelease(modes);
  }

  // Get processor information
  const NXArchInfo *ainfo = NXGetLocalArchInfo();
  _display_information->_cpu_brand_string.assign(ainfo->description);

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
  if (_display_link != nil) {
    CVDisplayLinkRelease(_display_link);
    _display_link = nil;

    // Unblock any threads that may be waiting on the VSync counter.
    __atomic_fetch_add(&_vsync_counter, 1u, __ATOMIC_SEQ_CST);
    patomic_notify_all(&_vsync_counter);
  }
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
 * Ensures a CVDisplayLink is created, which tells us when the display will
 * want a frame, to avoid tearing (vertical blanking interval).
 * Initializes the counter with the value that can be passed to wait_vsync
 * to wait for the next interval.
 */
bool CocoaGraphicsPipe::
init_vsync(uint32_t &counter) {
  if (_display_link != nil) {
    // Already set up.
    __atomic_load(&_vsync_counter, &counter, __ATOMIC_SEQ_CST);
    return true;
  }

  counter = 0;
  _vsync_counter = 0;

  CVDisplayLinkRef display_link;
  CVReturn result = CVDisplayLinkCreateWithActiveCGDisplays(&display_link);
  if (result != kCVReturnSuccess) {
    cocoadisplay_cat.error() << "Failed to create CVDisplayLink.\n";
    display_link = nil;
    return false;
  }

  result = CVDisplayLinkSetCurrentCGDisplay(display_link, _display);
  if (result != kCVReturnSuccess) {
    cocoadisplay_cat.error() << "Failed to set CVDisplayLink's current display.\n";
    CVDisplayLinkRelease(display_link);
    display_link = nil;
    return false;
  }

  result = CVDisplayLinkSetOutputCallback(display_link, &display_link_cb, this);
  if (result != kCVReturnSuccess) {
    cocoadisplay_cat.error() << "Failed to set CVDisplayLink output callback.\n";
    CVDisplayLinkRelease(display_link);
    display_link = nil;
    return false;
  }

  result = CVDisplayLinkStart(display_link);
  if (result != kCVReturnSuccess) {
    cocoadisplay_cat.error() << "Failed to start the CVDisplayLink.\n";
    CVDisplayLinkRelease(display_link);
    display_link = nil;
    return false;
  }

  _display_link = display_link;
  return true;
}

/**
 * The first time this method is called in a frame, waits for the vertical
 * blanking interval.  If init_vsync has not first been called, does nothing.
 *
 * The given counter will be updated with the vblank counter.  If adaptive is
 * true and the value differs from the current, no wait will occur.
 */
void CocoaGraphicsPipe::
wait_vsync(uint32_t &counter, bool adaptive) {
  if (_display_link == nil) {
    return;
  }

  // Use direct atomic operations since we need this to be thread-safe even
  // when compiling without thread support.
  uint32_t current_count = __atomic_load_n(&_vsync_counter, __ATOMIC_SEQ_CST);
  uint32_t diff = current_count - counter;
  if (diff > 0) {
    if (cocoadisplay_cat.is_spam()) {
      cocoadisplay_cat.spam()
        << "Missed vertical blanking interval by " << diff << " frames.\n";
    }
    if (adaptive) {
      counter = current_count;
      return;
    }
  }

  // We only wait for the first window that gets flipped in a single frame,
  // otherwise we end up halving our FPS when we have multiple windows!
  int cur_frame = ClockObject::get_global_clock()->get_frame_count();
  if (_last_wait_frame.exchange(cur_frame) == cur_frame) {
    counter = current_count;
    return;
  }

  patomic_wait(&_vsync_counter, current_count);
  __atomic_load(&_vsync_counter, &counter, __ATOMIC_SEQ_CST);
}

/**
 * Called whenever a display wants a frame.  The context argument contains the
 * applicable CocoaGraphicsPipe.
 */
CVReturn CocoaGraphicsPipe::
display_link_cb(CVDisplayLinkRef link, const CVTimeStamp *now,
                const CVTimeStamp *output_time, CVOptionFlags flags_in,
                CVOptionFlags *flags_out, void *context) {

  CocoaGraphicsPipe *pipe = (CocoaGraphicsPipe *)context;
  __atomic_fetch_add(&pipe->_vsync_counter, 1u, __ATOMIC_SEQ_CST);
  patomic_notify_all(&pipe->_vsync_counter);

  return kCVReturnSuccess;
}
