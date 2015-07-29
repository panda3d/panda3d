// Filename: tinyOsxGraphicsPipe.cxx
// Created by:  drose (12May08)
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

#include "pandabase.h"

#if defined(IS_OSX) && !defined(BUILD_IPHONE) && defined(HAVE_CARBON) && !__LP64__

#include "tinyOsxGraphicsPipe.h"
#include "config_tinydisplay.h"
#include "tinyOsxGraphicsWindow.h"
#include "tinyGraphicsBuffer.h"
#include "pnmImage.h"
#include "subprocessWindow.h"
#include "nativeWindowHandle.h"

TypeHandle TinyOsxGraphicsPipe::_type_handle;
  
////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TinyOsxGraphicsPipe::
TinyOsxGraphicsPipe() {
  CGRect display_bounds = CGDisplayBounds(kCGDirectMainDisplay);
  _display_width = CGRectGetWidth(display_bounds);
  _display_height = CGRectGetHeight(display_bounds);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TinyOsxGraphicsPipe::
~TinyOsxGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string TinyOsxGraphicsPipe::
get_interface_name() const {
  return "TinyPanda";
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               TinyOsxGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) TinyOsxGraphicsPipe::
pipe_constructor() {
  return new TinyOsxGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsPipe::create_cg_image
//       Access: Public, Static
//  Description: Creates a new Quartz bitmap image with the data in
//               the indicated PNMImage.  The caller should eventually
//               free this image via CGImageRelease.
////////////////////////////////////////////////////////////////////
CGImageRef TinyOsxGraphicsPipe::
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
//     Function: TinyOsxGraphicsPipe::release_data
//       Access: Private, Static
//  Description: This callback is assigned to delete the data array
//               allocated within create_cg_image().
////////////////////////////////////////////////////////////////////
void TinyOsxGraphicsPipe::
release_data(void *info, const void *data, size_t size) {
  char *char_array = (char *)data;
  PANDA_FREE_ARRAY(char_array);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyOsxGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window or buffer on the pipe, if possible.
//               This routine is only called from GraphicsEngine::make_output.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) TinyOsxGraphicsPipe::
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

  TinyGraphicsStateGuardian *tinygsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(tinygsg, gsg, NULL);
  }

  // First thing to try: a TinyOsxGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    if ((flags & BF_fb_props_optional)==0) {
      if ((fb_prop.get_aux_rgba() > 0)||
          (fb_prop.get_aux_hrgba() > 0)||
          (fb_prop.get_aux_float() > 0)) {
        return NULL;
      }
    }
    WindowHandle *window_handle = win_prop.get_parent_window();
    if (window_handle != NULL) {
      tinydisplay_cat.info()
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
    return new TinyOsxGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                     flags, gsg, host);
  }
  
  // Second thing to try: a TinyGraphicsBuffer
  if (retry == 1) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return NULL;
    }
    return new TinyGraphicsBuffer(engine, this, name, fb_prop, win_prop, flags, gsg, host);
  }
  
  // Nothing else left to try.
  return NULL;
}

#endif  // IS_OSX
