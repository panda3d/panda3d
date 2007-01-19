// Filename: displayInformation.cxx
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

#include "graphicsStateGuardian.h"
#include "displayInformation.h"

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
DisplayInformation::
~DisplayInformation() {
  if (_display_mode_array != NULL) {
    delete _display_mode_array;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
DisplayInformation::
DisplayInformation() {
  DisplayInformation::DetectionState state;
  int get_adapter_display_mode_state;
  int get_device_caps_state;
  int window_width;
  int window_height;
  int window_bits_per_pixel;
  int total_display_modes;
  DisplayMode *display_mode_array;
  int shader_model;
  int video_memory;
  int texture_memory;
  PN_uint64 physical_memory;
  PN_uint64 available_physical_memory;

  state = DisplayInformation::DS_unknown;    
  get_adapter_display_mode_state = false;
  get_device_caps_state = false;
  window_width = 0;
  window_height = 0;
  window_bits_per_pixel = 0;
  total_display_modes = 0;
  display_mode_array = NULL;
  shader_model = GraphicsStateGuardian::SM_00;
  video_memory = 0;
  texture_memory = 0;
  physical_memory = 0;
  available_physical_memory = 0;

  _state = state;
  _get_adapter_display_mode_state = get_adapter_display_mode_state;
  _get_device_caps_state = get_device_caps_state;
  _maximum_window_width = window_width;
  _maximum_window_height = window_height;
  _window_bits_per_pixel = window_bits_per_pixel;
  _total_display_modes = total_display_modes;
  _display_mode_array = display_mode_array;
  _shader_model = shader_model;
  _video_memory = video_memory;
  _texture_memory = texture_memory;
  _physical_memory = physical_memory;
  _available_physical_memory = available_physical_memory;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::get_display_state() {
  return _state;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_maximum_window_width
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_maximum_window_width() {
  return _maximum_window_width;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_maximum_window_height
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_maximum_window_height() {
  return _maximum_window_height;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_window_bits_per_pixel
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_window_bits_per_pixel() {
  return _window_bits_per_pixel;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_total_display_modes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_total_display_modes() {
  return _total_display_modes;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_display_mode_width
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_display_mode_width (int display_index) {
  int value;
  
  value = 0;
  if (display_index >= 0 && display_index < _total_display_modes) {
    value = _display_mode_array [display_index].width;
  }
  
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_display_mode_height
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_display_mode_height (int display_index) {
  int value;
  
  value = 0;
  if (display_index >= 0 && display_index < _total_display_modes) {
    value = _display_mode_array [display_index].height;
  }
  
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_display_mode_bits_per_pixel
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_display_mode_bits_per_pixel (int display_index) {
  int value;
  
  value = 0;
  if (display_index >= 0 && display_index < _total_display_modes) {
    value = _display_mode_array [display_index].bits_per_pixel;
  }
  
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_display_mode_refresh_rate
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_display_mode_refresh_rate (int display_index) {
  int value;
  
  value = 0;
  if (display_index >= 0 && display_index < _total_display_modes) {
    value = _display_mode_array [display_index].refresh_rate;
  }
  
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_display_mode_fullscreen_only
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_display_mode_fullscreen_only (int display_index) {
  int value;
  
  value = 0;
  if (display_index >= 0 && display_index < _total_display_modes) {
    value = _display_mode_array [display_index].fullscreen_only;
  }
  
  return value;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_shader_model
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_shader_model ( ) {
  return _shader_model;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_video_memory
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_video_memory ( ) {
  return _video_memory;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_texture_memory
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_texture_memory() {
  return _texture_memory;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_physical_memory
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_physical_memory() {
  return _physical_memory;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_available_physical_memory
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_available_physical_memory() {
  return _available_physical_memory;
}
