/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file displayInformation.cxx
 * @author aignacio
 * @date 2007-01-17
 */

#include "graphicsStateGuardian.h"
#include "displayInformation.h"

// For __rdtsc
#if defined(__i386) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#ifdef _MSC_VER
#include <intrin.h>
#elif defined(__GNUC__) && !defined(__clang__)
#include <x86intrin.h>
#endif
#endif

/**
 * Returns true if these two DisplayModes are identical.
 */
bool DisplayMode::
operator == (const DisplayMode &other) const {
  return (width == other.width && height == other.height &&
          bits_per_pixel == other.bits_per_pixel &&
          refresh_rate == other.refresh_rate &&
          fullscreen_only == other.fullscreen_only);
}

/**
 * Returns false if these two DisplayModes are identical.
 */
bool DisplayMode::
operator != (const DisplayMode &other) const {
  return !operator == (other);
}

/**
 *
 */
void DisplayMode::
output(std::ostream &out) const {
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

/**
 *
 */
DisplayInformation::
~DisplayInformation() {
  if (_display_mode_array != nullptr) {
    delete[] _display_mode_array;
  }
}

/**
 *
 */
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
  uint64_t physical_memory;
  uint64_t available_physical_memory;

  state = DisplayInformation::DS_unknown;
  get_adapter_display_mode_state = false;
  get_device_caps_state = false;
  window_width = 0;
  window_height = 0;
  window_bits_per_pixel = 0;
  total_display_modes = 0;
  display_mode_array = nullptr;
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

  _cpu_version_information = 0;
  _cpu_brand_index = 0;

  _cpu_frequency = 0;

  _maximum_cpu_frequency = 0;
  _current_cpu_frequency = 0;

  _num_cpu_cores = 0;
  _num_logical_cpus = 0;

  _get_memory_information_function = nullptr;
  _update_cpu_frequency_function = nullptr;

  _os_version_major = -1;
  _os_version_minor = -1;
  _os_version_build = -1;
  _os_platform_id = -1;
}

/**
 *
 */
int DisplayInformation::get_display_state() {
  return _state;
}

/**
 *
 */
int DisplayInformation::
get_maximum_window_width() {
  return _maximum_window_width;
}

/**
 *
 */
int DisplayInformation::
get_maximum_window_height() {
  return _maximum_window_height;
}

/**
 *
 */
int DisplayInformation::
get_window_bits_per_pixel() {
  return _window_bits_per_pixel;
}

/**
 *
 */
int DisplayInformation::
get_total_display_modes() {
  return _total_display_modes;
}

/**
 *
 */
const DisplayMode &DisplayInformation::
get_display_mode(int display_index) {
#ifndef NDEBUG
  static DisplayMode err_mode = {0};
  nassertr(display_index >= 0 && display_index < _total_display_modes, err_mode);
#endif

  return _display_mode_array[display_index];
}

/**
 *
 */
int DisplayInformation::
get_display_mode_width (int display_index) {
  int value;

  value = 0;
  if (display_index >= 0 && display_index < _total_display_modes) {
    value = _display_mode_array [display_index].width;
  }

  return value;
}

/**
 *
 */
int DisplayInformation::
get_display_mode_height (int display_index) {
  int value;

  value = 0;
  if (display_index >= 0 && display_index < _total_display_modes) {
    value = _display_mode_array [display_index].height;
  }

  return value;
}

/**
 *
 */
int DisplayInformation::
get_display_mode_bits_per_pixel (int display_index) {
  int value;

  value = 0;
  if (display_index >= 0 && display_index < _total_display_modes) {
    value = _display_mode_array [display_index].bits_per_pixel;
  }

  return value;
}

/**
 *
 */
int DisplayInformation::
get_display_mode_refresh_rate (int display_index) {
  int value;

  value = 0;
  if (display_index >= 0 && display_index < _total_display_modes) {
    value = _display_mode_array [display_index].refresh_rate;
  }

  return value;
}

/**
 *
 */
int DisplayInformation::
get_display_mode_fullscreen_only (int display_index) {
  int value;

  value = 0;
  if (display_index >= 0 && display_index < _total_display_modes) {
    value = _display_mode_array [display_index].fullscreen_only;
  }

  return value;
}

/**
 *
 */
GraphicsStateGuardian::ShaderModel DisplayInformation::
get_shader_model() {
  return _shader_model;
}

/**
 *
 */
int DisplayInformation::
get_video_memory ( ) {
  return _video_memory;
}

/**
 *
 */
int DisplayInformation::
get_texture_memory() {
  return _texture_memory;
}

/**
 *
 */
void DisplayInformation::
update_memory_information() {
  if (_get_memory_information_function) {
    _get_memory_information_function (this);
  }
}

/**
 *
 */
uint64_t DisplayInformation::
get_physical_memory() {
  return _physical_memory;
}

/**
 *
 */
uint64_t DisplayInformation::
get_available_physical_memory() {
  return _available_physical_memory;
}

/**
 *
 */
uint64_t DisplayInformation::
get_page_file_size() {
  return _page_file_size;
}

/**
 *
 */
uint64_t DisplayInformation::
get_available_page_file_size() {
  return _available_page_file_size;
}

/**
 *
 */
uint64_t DisplayInformation::
get_process_virtual_memory() {
  return _process_virtual_memory;
}

/**
 *
 */
uint64_t DisplayInformation::
get_available_process_virtual_memory() {
  return _available_process_virtual_memory;
}

/**
 *
 */
int DisplayInformation::
get_memory_load() {
  return _memory_load;
}

/**
 *
 */
uint64_t DisplayInformation::
get_page_fault_count() {
  return _page_fault_count;
}

/**
 *
 */
uint64_t DisplayInformation::
get_process_memory() {
  return _process_memory;
}

/**
 *
 */
uint64_t DisplayInformation::
get_peak_process_memory() {
  return _peak_process_memory;
}

/**
 *
 */
uint64_t DisplayInformation::
get_page_file_usage() {
  return _page_file_usage;
}

/**
 *
 */
uint64_t DisplayInformation::
get_peak_page_file_usage() {
  return _peak_page_file_usage;
}

/**
 *
 */
int DisplayInformation::
get_vendor_id() {
  return _vendor_id;
}

/**
 *
 */
int DisplayInformation::
get_device_id() {
  return _device_id;
}

/**
 *
 */
int DisplayInformation::
get_driver_product() {
  return _driver_product;
}

/**
 *
 */
int DisplayInformation::
get_driver_version() {
  return _driver_version;
}

/**
 *
 */
int DisplayInformation::
get_driver_sub_version() {
  return _driver_sub_version;
}

/**
 *
 */
int DisplayInformation::
get_driver_build() {
  return _driver_build;
}

/**
 *
 */
int DisplayInformation::
get_driver_date_month() {
  return _driver_date_month;
}

/**
 *
 */
int DisplayInformation::
get_driver_date_day() {
  return _driver_date_day;
}

/**
 *
 */
int DisplayInformation::
get_driver_date_year() {
  return _driver_date_year;
}

/**
 *
 */
const std::string &DisplayInformation::
get_cpu_vendor_string() const {
  return _cpu_vendor_string;
}

/**
 *
 */
const std::string &DisplayInformation::
get_cpu_brand_string() const {
  return _cpu_brand_string;
}

/**
 *
 */
unsigned int DisplayInformation::
get_cpu_version_information() {
  return _cpu_version_information;
}

/**
 *
 */
unsigned int DisplayInformation::
get_cpu_brand_index() {
  return _cpu_brand_index;
}

/**
 *
 */
uint64_t DisplayInformation::
get_cpu_frequency() {
  return _cpu_frequency;
}

/**
 * Equivalent to the rdtsc processor instruction.
 */
uint64_t DisplayInformation::
get_cpu_time() {
#if defined(__i386) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#if defined(_MSC_VER) || (defined(__GNUC__) && !defined(__clang__))
  return __rdtsc();
#else
  unsigned int lo, hi = 0;
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
#endif
#else
  return 0;
#endif
}

/**
 *
 */
uint64_t DisplayInformation::
get_maximum_cpu_frequency() {
  return _maximum_cpu_frequency;
}

/**
 *
 */
uint64_t DisplayInformation::
get_current_cpu_frequency() {
  return _current_cpu_frequency;
}

/**
 *
 */
void DisplayInformation::
update_cpu_frequency(int processor_number) {
  if (_update_cpu_frequency_function) {
    _update_cpu_frequency_function (processor_number, this);
  }
}

/**
 * Returns the number of individual CPU cores in the system, or 0 if this
 * number is not available.  A hyperthreaded CPU counts once here.
 */
int DisplayInformation::
get_num_cpu_cores() {
  return _num_cpu_cores;
}

/**
 * Returns the number of logical CPU's in the system, or 0 if this number is
 * not available.  A hyperthreaded CPU counts as two or more here.
 */
int DisplayInformation::
get_num_logical_cpus() {
  return _num_logical_cpus;
}

/**
 * Returns -1 if not set.
 */
int DisplayInformation::
get_os_version_major() {
  return _os_version_major;
}

/**
 * Returns -1 if not set.
 */
int DisplayInformation::
get_os_version_minor() {
  return _os_version_minor;
}

/**
 * Returns -1 if not set.
 */
int DisplayInformation::
get_os_version_build() {
  return _os_version_build;
}

/**
 * Returns -1 if not set.
 */
int DisplayInformation::
get_os_platform_id() {
  return _os_platform_id;
}
