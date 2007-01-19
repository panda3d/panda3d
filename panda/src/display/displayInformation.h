// Filename: displayInformation.h
// Created by:  aignacio (17Jan07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights 
// reserved.
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef DISPLAYINFORMATION_H
#define DISPLAYINFORMATION_H

#include "typedef.h"

typedef struct {
  int width;
  int height;
  int bits_per_pixel;
  int refresh_rate;
  int fullscreen_only;
}
DisplayMode;

////////////////////////////////////////////////////////////////////
//       Class : DisplayInformation
// Description : This class contains various display information.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA DisplayInformation {

PUBLISHED:

  enum DetectionState {
    DS_unknown,  
    DS_success,  

    DS_direct_3d_create_error,  
    DS_create_window_error,
    DS_create_device_error,
  };

  DisplayInformation::~DisplayInformation();
  DisplayInformation::DisplayInformation();

  int DisplayInformation::get_display_state();

  int DisplayInformation::get_maximum_window_width();
  int DisplayInformation::get_maximum_window_height();
  int DisplayInformation::get_window_bits_per_pixel();

  int DisplayInformation::get_total_display_modes();
  int DisplayInformation::get_display_mode_width(int display_index);
  int DisplayInformation::get_display_mode_height(int display_index);
  int DisplayInformation::get_display_mode_bits_per_pixel(int display_index);
  int DisplayInformation::get_display_mode_refresh_rate(int display_index);
  int DisplayInformation::get_display_mode_fullscreen_only(int display_index);

  int DisplayInformation::get_shader_model();
  int DisplayInformation::get_video_memory();
  int DisplayInformation::get_texture_memory();

  PN_uint64 DisplayInformation::get_physical_memory();
  PN_uint64 DisplayInformation::get_available_physical_memory();

public:
  DetectionState _state;
  int _get_adapter_display_mode_state;
  int _get_device_caps_state;
  int _maximum_window_width;
  int _maximum_window_height;
  int _window_bits_per_pixel;
  int _total_display_modes;  
  DisplayMode *_display_mode_array;
  int _shader_model;
  int _video_memory;
  int _texture_memory;
  PN_uint64 _physical_memory;
  PN_uint64 _available_physical_memory;
};

#endif
