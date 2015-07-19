// Filename: displayInformation.h
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

#ifndef DISPLAYINFORMATION_H
#define DISPLAYINFORMATION_H

#include "typedef.h"
#include "graphicsStateGuardian.h"

struct EXPCL_PANDA_DISPLAY DisplayMode {
PUBLISHED:
  int width;
  int height;
  int bits_per_pixel;
  int refresh_rate;
  int fullscreen_only;

  bool operator == (const DisplayMode &other) const;
  bool operator != (const DisplayMode &other) const;
  void output(ostream &out) const;
};

////////////////////////////////////////////////////////////////////
//       Class : DisplayInformation
// Description : This class contains various display information.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY DisplayInformation {
PUBLISHED:
  enum DetectionState {
    DS_unknown,
    DS_success,

    DS_direct_3d_create_error,
    DS_create_window_error,
    DS_create_device_error,
  };

  ~DisplayInformation();
  DisplayInformation();

  int get_display_state();

  int get_maximum_window_width();
  int get_maximum_window_height();
  int get_window_bits_per_pixel();

  int get_total_display_modes();
  const DisplayMode &get_display_mode(int display_index);
  MAKE_SEQ(get_display_modes, get_total_display_modes, get_display_mode);

  // Older interface for display modes.
  int get_display_mode_width(int display_index);
  int get_display_mode_height(int display_index);
  int get_display_mode_bits_per_pixel(int display_index);
  int get_display_mode_refresh_rate(int display_index);
  int get_display_mode_fullscreen_only(int display_index);

  GraphicsStateGuardian::ShaderModel get_shader_model();
  int get_video_memory();
  int get_texture_memory();

  void update_memory_information();
  PN_uint64 get_physical_memory();
  PN_uint64 get_available_physical_memory();
  PN_uint64 get_page_file_size();
  PN_uint64 get_available_page_file_size();
  PN_uint64 get_process_virtual_memory();
  PN_uint64 get_available_process_virtual_memory();
  int get_memory_load();
  PN_uint64 get_page_fault_count();
  PN_uint64 get_process_memory();
  PN_uint64 get_peak_process_memory();
  PN_uint64 get_page_file_usage();
  PN_uint64 get_peak_page_file_usage();

  int get_vendor_id();
  int get_device_id();

  int get_driver_product();
  int get_driver_version();
  int get_driver_sub_version();
  int get_driver_build();

  int get_driver_date_month();
  int get_driver_date_day();
  int get_driver_date_year();

  int get_cpu_id_version();
  int get_cpu_id_size();
  unsigned int get_cpu_id_data(int index);

  const char *get_cpu_vendor_string();
  const char *get_cpu_brand_string();
  unsigned int get_cpu_version_information();
  unsigned int get_cpu_brand_index();
  
  PN_uint64 get_cpu_frequency();
  PN_uint64 get_cpu_time();

  PN_uint64 get_maximum_cpu_frequency();
  PN_uint64 get_current_cpu_frequency();
  void update_cpu_frequency(int processor_number);

  int get_num_cpu_cores();
  int get_num_logical_cpus();

  int get_os_version_major();
  int get_os_version_minor();
  int get_os_version_build();
  int get_os_platform_id();
  
public:
  DetectionState _state;
  int _get_adapter_display_mode_state;
  int _get_device_caps_state;
  int _maximum_window_width;
  int _maximum_window_height;
  int _window_bits_per_pixel;
  int _total_display_modes;
  DisplayMode *_display_mode_array;
  GraphicsStateGuardian::ShaderModel _shader_model;
  int _video_memory;
  int _texture_memory;

  PN_uint64 _physical_memory;
  PN_uint64 _available_physical_memory;
  PN_uint64 _page_file_size;
  PN_uint64 _available_page_file_size;
  PN_uint64 _process_virtual_memory;
  PN_uint64 _available_process_virtual_memory;

  PN_uint64 _page_fault_count;
  PN_uint64 _process_memory;
  PN_uint64 _peak_process_memory;
  PN_uint64 _page_file_usage;
  PN_uint64 _peak_page_file_usage;
  
  int _memory_load;

  int _vendor_id;
  int _device_id;

  int _driver_product;
  int _driver_version;
  int _driver_sub_version;
  int _driver_build;

  int _driver_date_month;
  int _driver_date_day;
  int _driver_date_year;

  int _cpu_id_version;
  int _cpu_id_size;
  unsigned int *_cpu_id_data;

  char *_cpu_vendor_string;
  char *_cpu_brand_string;
  unsigned int _cpu_version_information;
  unsigned int _cpu_brand_index;
  
  PN_uint64 _cpu_frequency;
  
  PN_uint64 _maximum_cpu_frequency;
  PN_uint64 _current_cpu_frequency;

  int _num_cpu_cores;
  int _num_logical_cpus;
  
  void (*_get_memory_information_function) (DisplayInformation *display_information);
  PN_uint64 (*_cpu_time_function) (void);
  int (*_update_cpu_frequency_function) (int processor_number, DisplayInformation *display_information);
  
  int _os_version_major;
  int _os_version_minor;
  int _os_version_build;
  int _os_platform_id;
};

#endif
