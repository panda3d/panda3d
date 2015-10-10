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

#include "osxGraphicsPipe.h"
#include "config_osxdisplay.h"
#include "osxGraphicsWindow.h"
#include "osxGraphicsBuffer.h"
#include "osxGraphicsStateGuardian.h"
#include "pnmImage.h"
#include "subprocessWindow.h"
#include "nativeWindowHandle.h"
#include "displayInformation.h"
#import <Carbon/Carbon.h>

// some macros to make code more readable.
#define GetModeWidth(mode) GetDictionaryLong((mode), kCGDisplayWidth)
#define GetModeHeight(mode) GetDictionaryLong((mode), kCGDisplayHeight)
#define GetModeRefreshRate(mode) GetDictionaryLong((mode), kCGDisplayRefreshRate)
#define GetModeBitsPerPixel(mode) GetDictionaryLong((mode), kCGDisplayBitsPerPixel)
#define GetModeSafeForHardware(mode) GetDictionaryBoolean((mode), kCGDisplayModeIsSafeForHardware)
#define GetModeStretched(mode) GetDictionaryBoolean((mode), kCGDisplayModeIsStretched)
#define MAX_DISPLAYS 32

Boolean GetDictionaryBoolean(CFDictionaryRef theDict, const void* key) {
  // get a boolean from the dictionary
  Boolean value = false;
  CFBooleanRef boolRef;
  boolRef = (CFBooleanRef)CFDictionaryGetValue(theDict, key);
  if (boolRef != NULL)
    value = CFBooleanGetValue(boolRef);   
  return value;
}

long GetDictionaryLong(CFDictionaryRef theDict, const void* key) {
  // get a long from the dictionary
  long value = 0;
  CFNumberRef numRef;
  numRef = (CFNumberRef)CFDictionaryGetValue(theDict, key);
  if (numRef != NULL)
    CFNumberGetValue(numRef, kCFNumberLongType, &value);   
  return value;
}

static CFComparisonResult CompareModes (const void *val1,const void *val2,void *context) {
  // CFArray comparison callback for sorting display modes.
#pragma unused(context)
  CFDictionaryRef thisMode = (CFDictionaryRef)val1;
  CFDictionaryRef otherMode = (CFDictionaryRef)val2;
   
  long width = GetModeWidth(thisMode);
  long otherWidth = GetModeWidth(otherMode);
  long height = GetModeHeight(thisMode);
  long otherHeight = GetModeHeight(otherMode);
   
  // sort modes in screen size order
  if (width * height < otherWidth * otherHeight) {
    return kCFCompareLessThan;
  } else if (width * height > otherWidth * otherHeight) {
    return kCFCompareGreaterThan;
  }
   
  // sort modes by bits per pixel
  long bitsPerPixel = GetModeBitsPerPixel(thisMode);
  long otherBitsPerPixel = GetModeBitsPerPixel(otherMode);   
  if (bitsPerPixel < otherBitsPerPixel) {
    return kCFCompareLessThan;
  } else if (bitsPerPixel > otherBitsPerPixel) {
    return kCFCompareGreaterThan;
  }
   
  // sort modes by refresh rate.
  long refreshRate = GetModeRefreshRate(thisMode);
  long otherRefreshRate = GetModeRefreshRate(otherMode);   
  if (refreshRate < otherRefreshRate) {
    return kCFCompareLessThan;
  } else if (refreshRate > otherRefreshRate) {
    return kCFCompareGreaterThan;
  }
     
  return kCFCompareEqualTo;
}

CFArrayRef GSCGDisplayAvailableModesUsefulForOpenGL(CGDirectDisplayID display) {
  // get a list of all possible display modes for this system.
  CFArrayRef availableModes = CGDisplayAvailableModes(display);
  unsigned int numberOfAvailableModes = CFArrayGetCount(availableModes);
 
  // creat mutable array to hold the display modes we are interested int.
  CFMutableArrayRef usefulModes = CFArrayCreateMutable(kCFAllocatorDefault, numberOfAvailableModes, NULL);
 
  // get the current bits per pixel.
  long currentModeBitsPerPixel = GetModeBitsPerPixel(CGDisplayCurrentMode(display));
 
  unsigned int i;
  for (i= 0; i<numberOfAvailableModes; ++i) {
    // look at each mode in the available list
    CFDictionaryRef mode = (CFDictionaryRef)CFArrayGetValueAtIndex(availableModes, i);
     
    // we are only interested in modes with the same bits per pixel as current.
    //   to allow for switching from fullscreen to windowed modes.
    // that are safe for this hardward
    // that are not stretched.
    long bitsPerPixel = GetModeBitsPerPixel(mode);
    Boolean safeForHardware = GetModeSafeForHardware(mode);
    Boolean stretched = GetModeStretched(mode);
   
    if ((bitsPerPixel != currentModeBitsPerPixel) || (!safeForHardware) || (stretched)) {
      continue; // skip this mode
    }
     
    long width = GetModeWidth(mode);
    long height = GetModeHeight(mode);
    long refreshRate = GetModeRefreshRate(mode);     
    Boolean replaced = false;
    Boolean skipped = false;
     
    // now check to see if we already added a mode like this one.
    //   we want the highest refresh rate for this width/height
    unsigned int j;
    unsigned int currentNumberOfUsefulModes =  CFArrayGetCount(usefulModes);
    for (j = 0; j < currentNumberOfUsefulModes; ++j) {
      CFDictionaryRef otherMode = (CFDictionaryRef)CFArrayGetValueAtIndex(usefulModes, j);       
      long otherWidth = GetModeWidth(otherMode);
      long otherHeight = GetModeHeight(otherMode);       
      if ((otherWidth == width) && (otherHeight == height)) {
        long otherRefreshRate = GetModeRefreshRate(otherMode);         
        if (otherRefreshRate < refreshRate) {
          // replace lower refresh rate.
          const void* value = mode;
          CFArrayReplaceValues(usefulModes, CFRangeMake(j ,1), &value, 1);
          replaced = true;
          break;
        }
        else if (otherRefreshRate > refreshRate) {
          skipped = true;
          break;
        }
      }
    }     
    // this is a useful mode so add it to the array.
    if (!replaced && !skipped) {
      CFArrayAppendValue(usefulModes, mode);
    }     
  }   
  // now sort the useful mode array, using the comparison callback.
  CFArraySortValues( usefulModes,
    CFRangeMake(0, CFArrayGetCount(usefulModes)),
    (CFComparatorFunction) CompareModes, NULL); 
  // return the CFArray of the useful display modes.
  return usefulModes;
}

TypeHandle osxGraphicsPipe::_type_handle;
  
////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
osxGraphicsPipe::
osxGraphicsPipe() {
  CGRect display_bounds = CGDisplayBounds(kCGDirectMainDisplay);
  _display_width = CGRectGetWidth(display_bounds);
  _display_height = CGRectGetHeight(display_bounds);
 
  CGDirectDisplayID display, displayArray[MAX_DISPLAYS] ;
  CGDisplayCount numDisplays;
  CFDictionaryRef displayMode; 
  CFArrayRef displayModeArray;
  int number, i; 
  CGGetActiveDisplayList (MAX_DISPLAYS, displayArray, &numDisplays);
  display = displayArray [numDisplays - 1];
  displayModeArray = GSCGDisplayAvailableModesUsefulForOpenGL( display );   
  number = CFArrayGetCount( displayModeArray );
  DisplayMode *displays = new DisplayMode[ number ]; 
  for(i = 0; i < number; i++) {     
     displayMode = (CFDictionaryRef) CFArrayGetValueAtIndex (displayModeArray, i);
     _display_information -> _total_display_modes++;     
     displays[i].width = (signed int)GetModeWidth (displayMode);
     displays[i].height = (signed int)GetModeHeight (displayMode);
     displays[i].bits_per_pixel = (signed int)GetModeBitsPerPixel (displayMode);
     displays[i].refresh_rate = (signed int)GetModeRefreshRate (displayMode);
  }   
  _display_information -> _display_mode_array = displays;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
osxGraphicsPipe::
~osxGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string osxGraphicsPipe::
get_interface_name() const {
  return "OpenGL";
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               osxGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) osxGraphicsPipe::
pipe_constructor() {
  return new osxGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::get_preferred_window_thread
//       Access: Public, Virtual
//  Description: Returns an indication of the thread in which this
//               GraphicsPipe requires its window processing to be
//               performed: typically either the app thread (e.g. X)
//               or the draw thread (Windows).
////////////////////////////////////////////////////////////////////
GraphicsPipe::PreferredWindowThread 
osxGraphicsPipe::get_preferred_window_thread() const {
  return PWT_app;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::create_cg_image
//       Access: Public, Static
//  Description: Creates a new Quartz bitmap image with the data in
//               the indicated PNMImage.  The caller should eventually
//               free this image via CGImageRelease.
////////////////////////////////////////////////////////////////////
CGImageRef osxGraphicsPipe::
create_cg_image(const PNMImage &pnm_image) {
  size_t width = pnm_image.get_x_size();
  size_t height = pnm_image.get_y_size();

#ifdef PGM_BIGGRAYS
  size_t bytes_per_component = 2;
#else
  size_t bytes_per_component = 1;
#endif
  size_t bits_per_component = bytes_per_component * 8;
  size_t num_components = pnm_image.get_num_channels();

  size_t bits_per_pixel = num_components * bits_per_component;
  size_t bytes_per_row = num_components * bytes_per_component * width;

  size_t num_bytes = bytes_per_row * height;
  bool has_alpha;
  bool is_grayscale;

  CFStringRef color_space_name = NULL;
  switch (pnm_image.get_color_type()) {
  case PNMImage::CT_grayscale:
    color_space_name = kCGColorSpaceGenericGray;
    has_alpha = false;
    is_grayscale = true;
    break;

  case PNMImage::CT_two_channel:
    color_space_name = kCGColorSpaceGenericGray;
    has_alpha = true;
    is_grayscale = true;
    break;

  case PNMImage::CT_color:
    color_space_name = kCGColorSpaceGenericRGB;
    has_alpha = false;
    is_grayscale = false;
    break;

  case PNMImage::CT_four_channel:
    color_space_name = kCGColorSpaceGenericRGB;
    has_alpha = true;
    is_grayscale = false;
    break;

  case PNMImage::CT_invalid:
    // Shouldn't get here.
    nassertr(false, NULL);
    break;
  }
  nassertr(color_space_name != NULL, NULL);

  CGColorSpaceRef color_space = CGColorSpaceCreateWithName(color_space_name);
  nassertr(color_space != NULL, NULL);

  CGBitmapInfo bitmap_info = 0;
#ifdef PGM_BIGGRAYS
  bitmap_info |= kCGBitmapByteOrder16Host;
#endif
  if (has_alpha) {
    bitmap_info |= kCGImageAlphaLast;
  }

  // Now convert the pixel data to a format friendly to
  // CGImageCreate().
  char *char_array = (char *)PANDA_MALLOC_ARRAY(num_bytes);

  xelval *dp = (xelval *)char_array;
  for (size_t yi = 0; yi < height; ++yi) {
    for (size_t xi = 0; xi < width; ++xi) {
      if (is_grayscale) {
        *dp++ = (xelval)(pnm_image.get_gray(xi, yi) * PGM_MAXMAXVAL);
      } else {
        *dp++ = (xelval)(pnm_image.get_red(xi, yi) * PGM_MAXMAXVAL);
        *dp++ = (xelval)(pnm_image.get_green(xi, yi) * PGM_MAXMAXVAL);
        *dp++ = (xelval)(pnm_image.get_blue(xi, yi) * PGM_MAXMAXVAL);
      }
      if (has_alpha) {
        *dp++ = (xelval)(pnm_image.get_alpha(xi, yi) * PGM_MAXMAXVAL);
      }
    }
  }
  nassertr((void *)dp == (void *)(char_array + num_bytes), NULL);

  CGDataProviderRef provider = 
    CGDataProviderCreateWithData(NULL, char_array, num_bytes, release_data);
  nassertr(provider != NULL, NULL);

  CGImageRef image = CGImageCreate
    (width, height, bits_per_component, bits_per_pixel, bytes_per_row,
     color_space, bitmap_info, provider,
     NULL, false, kCGRenderingIntentDefault);
  nassertr(image != NULL, NULL);

  CGColorSpaceRelease(color_space);
  CGDataProviderRelease(provider);

  return image;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::release_data
//       Access: Private, Static
//  Description: This callback is assigned to delete the data array
//               allocated within create_cg_image().
////////////////////////////////////////////////////////////////////
void osxGraphicsPipe::
release_data(void *info, const void *data, size_t size) {
  char *char_array = (char *)data;
  PANDA_FREE_ARRAY(char_array);
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) osxGraphicsPipe::
make_output(const string &name,
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
  
  osxGraphicsStateGuardian *osxgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(osxgsg, gsg, NULL);
  }

  // First thing to try: an osxGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)||
        ((flags&BF_can_bind_layered)!=0)) {
      return NULL;
    }
    WindowHandle *window_handle = win_prop.get_parent_window();
    if (window_handle != NULL) {
      osxdisplay_cat.info()
        << "Got parent_window " << *window_handle << "\n";
#ifdef SUPPORT_SUBPROCESS_WINDOW
      WindowHandle::OSHandle *os_handle = window_handle->get_os_handle();
      if (os_handle != NULL && 
          os_handle->is_of_type(NativeWindowHandle::SubprocessHandle::get_class_type())) {
        return new SubprocessWindow(engine, this, name, fb_prop, win_prop,
                                    flags, gsg, host);
      }
#endif  // SUPPORT_SUBPROCESS_WINDOW
    }
    return new osxGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }

  // Second thing to try: a GLGraphicsBuffer

  if (retry == 1) {
    if (!osx_support_gl_buffer || !gl_support_fbo || host == NULL ||
        (flags & (BF_require_parasite | BF_require_window)) != 0) {
      return NULL;
    }
    // Early failure - if we are sure that this buffer WONT
    // meet specs, we can bail out early.
    if ((flags & BF_fb_props_optional) == 0) {
      if (fb_prop.get_indexed_color() ||
          fb_prop.get_back_buffers() > 0 ||
          fb_prop.get_accum_bits() > 0) {
        return NULL;
      }
    }
    if (osxgsg != NULL && osxgsg->is_valid() && !osxgsg->needs_reset()) {
      if (!osxgsg->_supports_framebuffer_object ||
          osxgsg->_glDrawBuffers == NULL) {
        return NULL;
      } else if (fb_prop.is_basic()) {
        // Early success - if we are sure that this buffer WILL
        // meet specs, we can precertify it.
        precertify = true;
      }
    }
    return new GLGraphicsBuffer(engine, this, name, fb_prop, win_prop, flags, gsg, host);
  }
  
  // Third thing to try: an osxGraphicsBuffer
  if (retry == 2) {
    if ((!support_render_texture)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_can_bind_every)!=0)||
        ((flags&BF_can_bind_layered)!=0)) {
      return NULL;
    }
    return new osxGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }

  // Nothing else left to try.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsPipe::make_callback_gsg
//       Access: Protected, Virtual
//  Description: This is called when make_output() is used to create a
//               CallbackGraphicsWindow.  If the GraphicsPipe can
//               construct a GSG that's not associated with any
//               particular window object, do so now, assuming the
//               correct graphics context has been set up externally.
////////////////////////////////////////////////////////////////////
PT(GraphicsStateGuardian) osxGraphicsPipe::
make_callback_gsg(GraphicsEngine *engine) {
  return new osxGraphicsStateGuardian(engine, this, NULL);
}
