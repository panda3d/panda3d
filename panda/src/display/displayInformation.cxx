// Filename: displayInformation.cxx
// Created by:  aignacio (17Jan07)
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

#include "graphicsStateGuardian.h"
#include "displayInformation.h"

////////////////////////////////////////////////////////////////////
//     Function: DisplayMode::Comparison Operator
//       Access: Published
//  Description: Returns true if these two DisplayModes are identical.
////////////////////////////////////////////////////////////////////
bool DisplayMode::
operator == (const DisplayMode &other) const {
  return (width == other.width && height == other.height &&
          bits_per_pixel == other.bits_per_pixel &&
          refresh_rate == other.refresh_rate &&
          fullscreen_only == other.fullscreen_only);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayMode::Comparison Operator
//       Access: Published
//  Description: Returns false if these two DisplayModes are identical.
////////////////////////////////////////////////////////////////////
bool DisplayMode::
operator != (const DisplayMode &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayMode::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void DisplayMode::
output(ostream &out) const {
  out << width << 'x' << height;
  if (bits_per_pixel > 0) {
    out << ' ' << bits_per_pixel << "bpp";
  }
  if (refresh_rate > 0) {
    out << ' ' << refresh_rate << "Hz";
  }
  if (fullscreen_only > 0) {
    out << " (fullscreen only)";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DisplayInformation::
~DisplayInformation() {
  if (_display_mode_array != NULL) {
    delete[] _display_mode_array;
  }
  if (_cpu_id_data != NULL) {
    delete _cpu_id_data;
  }
  if (_cpu_brand_string != NULL) {
    delete _cpu_brand_string;
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
  _shader_model = GraphicsStateGuardian::SM_00;
  _video_memory = video_memory;
  _texture_memory = texture_memory;

  _physical_memory = physical_memory;
  _available_physical_memory = available_physical_memory;
  _page_file_size = 0;
  _available_page_file_size = 0;
  _process_virtual_memory = 0;
  _available_process_virtual_memory = 0;
  _memory_load = 0;
  _page_fault_count = 0;
  _process_memory = 0;
  _peak_process_memory = 0;
  _page_file_usage = 0;
  _peak_page_file_usage = 0;

  _vendor_id = 0;
  _device_id = 0;

  _driver_product = 0;
  _driver_version = 0;
  _driver_sub_version = 0;
  _driver_build = 0;

  _driver_date_month = 0;
  _driver_date_day = 0;
  _driver_date_year = 0;

  _cpu_id_version = 1;
  _cpu_id_size = 0;
  _cpu_id_data = 0;

  _cpu_vendor_string = 0;
  _cpu_brand_string = 0;
  _cpu_version_information = 0;
  _cpu_brand_index = 0;

  _cpu_frequency = 0;

  _maximum_cpu_frequency = 0;
  _current_cpu_frequency = 0;

  _num_cpu_cores = 0;
  _num_logical_cpus = 0;

  _get_memory_information_function = 0;
  _cpu_time_function = 0;
  _update_cpu_frequency_function = 0;

  _os_version_major = -1;
  _os_version_minor = -1;
  _os_version_build = -1;
  _os_platform_id = -1;
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
//     Function: DisplayInformation::get_display_mode
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
const DisplayMode &DisplayInformation::
get_display_mode(int display_index) {
#ifndef NDEBUG
  static DisplayMode err_mode = {0};
  nassertr(display_index >= 0 && display_index < _total_display_modes, err_mode);
#endif

  return _display_mode_array[display_index];
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
GraphicsStateGuardian::ShaderModel DisplayInformation::
get_shader_model() {
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
//     Function: DisplayInformation::update_memory_information
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void DisplayInformation::
update_memory_information() {
  if (_get_memory_information_function) {
    _get_memory_information_function (this);
  }
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

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_page_file_size
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_page_file_size() {
  return _page_file_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_available_page_file_size
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_available_page_file_size() {
  return _available_page_file_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::_process_virtual_memory
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_process_virtual_memory() {
  return _process_virtual_memory;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_available_process_virtual_memory
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_available_process_virtual_memory() {
  return _available_process_virtual_memory;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_memory_load
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_memory_load() {
  return _memory_load;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_page_fault_count
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_page_fault_count() {
  return _page_fault_count;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_process_memory
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_process_memory() {
  return _process_memory;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_peak_process_memory
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_peak_process_memory() {
  return _peak_process_memory;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_page_file_usage
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_page_file_usage() {
  return _page_file_usage;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_peak_page_file_usage
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_peak_page_file_usage() {
  return _peak_page_file_usage;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_vendor_id
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_vendor_id() {
  return _vendor_id;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_device_id
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_device_id() {
  return _device_id;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_driver_product
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_driver_product() {
  return _driver_product;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_driver_version
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_driver_version() {
  return _driver_version;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_driver_sub_version
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_driver_sub_version() {
  return _driver_sub_version;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_driver_build
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_driver_build() {
  return _driver_build;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_driver_date_month
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_driver_date_month() {
  return _driver_date_month;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_driver_date_day
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_driver_date_day() {
  return _driver_date_day;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_driver_date_year
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_driver_date_year() {
  return _driver_date_year;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_cpu_id_version
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_cpu_id_version() {
  return _cpu_id_version;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_cpu_id_size
//       Access: Published
//  Description: Returns the number of 32-bit values for cpu id 
//               binary data.
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_cpu_id_size() {
  return _cpu_id_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_cpu_id_data
//       Access: Published
//  Description: Returns part of cpu id binary data based on the 
//               index.  
////////////////////////////////////////////////////////////////////
unsigned int DisplayInformation::
get_cpu_id_data(int index) {
  unsigned int data;

  data = 0;
  if (index >= 0 && index < _cpu_id_size) {
    data = _cpu_id_data [index];
  }
  
  return data;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_cpu_vendor_string
//       Access: Published
//  Description:  
////////////////////////////////////////////////////////////////////
const char *DisplayInformation::
get_cpu_vendor_string() {  
  const char *string;
  
  string = _cpu_vendor_string;
  if (string == 0) {
    string = "";
  }
  
  return string;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_cpu_brand_string
//       Access: Published
//  Description:  
////////////////////////////////////////////////////////////////////
const char *DisplayInformation::
get_cpu_brand_string() {  
  const char *string;
  
  string = _cpu_brand_string;
  if (string == 0) {
    string = "";
  }
  
  return string;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_cpu_version_information
//       Access: Published
//  Description:  
////////////////////////////////////////////////////////////////////
unsigned int DisplayInformation::
get_cpu_version_information() {  
  return _cpu_version_information;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_cpu_brand_index
//       Access: Published
//  Description:  
////////////////////////////////////////////////////////////////////
unsigned int DisplayInformation::
get_cpu_brand_index() {  
  return _cpu_brand_index;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_cpu_frequency
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_cpu_frequency() {
  return _cpu_frequency;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_cpu_time
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_cpu_time() {
  PN_uint64 cpu_time;
  
  cpu_time = 0;
  if (_cpu_time_function) {
    cpu_time = _cpu_time_function();
  }
  
  return cpu_time;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_maximum_cpu_frequency
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_maximum_cpu_frequency() {
  return _maximum_cpu_frequency;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_current_cpu_frequency
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PN_uint64 DisplayInformation::
get_current_cpu_frequency() {
  return _current_cpu_frequency;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::update_cpu_frequency
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void DisplayInformation::
update_cpu_frequency(int processor_number) {
  if (_update_cpu_frequency_function) {
    _update_cpu_frequency_function (processor_number, this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_num_cpu_cores
//       Access: Published
//  Description: Returns the number of individual CPU cores in the
//               system, or 0 if this number is not available.  A
//               hyperthreaded CPU counts once here.
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_num_cpu_cores() {
  return _num_cpu_cores;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_num_logical_cpus
//       Access: Published
//  Description: Returns the number of logical CPU's in the
//               system, or 0 if this number is not available.  A
//               hyperthreaded CPU counts as two or more here.
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_num_logical_cpus() {
  return _num_logical_cpus;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_os_version_major
//       Access: Published
//  Description: Returns -1 if not set. 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_os_version_major() {  
  return _os_version_major;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_os_version_minor
//       Access: Published
//  Description: Returns -1 if not set. 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_os_version_minor() {  
  return _os_version_minor;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_os_version_build
//       Access: Published
//  Description: Returns -1 if not set. 
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_os_version_build() {  
  return _os_version_build;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayInformation::get_os_platform_id
//       Access: Published
//  Description: Returns -1 if not set.
////////////////////////////////////////////////////////////////////
int DisplayInformation::
get_os_platform_id() {  
  return _os_platform_id;
}
