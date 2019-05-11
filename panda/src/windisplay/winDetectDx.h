/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winDetectDx.h
 * @author aignacio
 * @date 2007-01-18
 */

#include <time.h>
#include "displayInformation.h"

typedef struct {
  D3DFORMAT d3d_format;
  int bits_per_pixel;
  int fullscreen_only;
}
DISPLAY_FORMAT;

static DISPLAY_FORMAT display_format_array [ ] = {
  D3DFMT_X8R8G8B8,    32, FALSE,
  D3DFMT_R5G6B5,      16, FALSE,
  D3DFMT_X1R5G5B5,    16, FALSE,

  D3DFMT_A2R10G10B10, 32, TRUE,

  // terminator
  D3DFMT_UNKNOWN,      0, FALSE,
};

typedef BOOL (WINAPI *GlobalMemoryStatusExType) (LPMEMORYSTATUSEX lpBuffer);

static int d3d_format_to_bits_per_pixel (D3DFORMAT d3d_format) {
  int format_index;
  int bits_per_pixel;

  format_index = 0;
  bits_per_pixel = 0;
  while (display_format_array [format_index].d3d_format != D3DFMT_UNKNOWN) {
    if (d3d_format == display_format_array [format_index].d3d_format) {
      bits_per_pixel = display_format_array [format_index].bits_per_pixel;
      break;
    }

    format_index++;
  }

  return bits_per_pixel;
}

static DWORD _GetLastError (char *message_prefix) {
  LPVOID ptr;
  DWORD error;

  ptr = 0;
  error = GetLastError ( );
  if (FormatMessage (
        FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr, error, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
        (LPTSTR)&ptr,0, nullptr)) {
    std::cout << "ERROR: "<< message_prefix << " result = " << (char*) ptr << "\n";
    LocalFree( ptr );
  }

  return error;
}

static LRESULT CALLBACK window_procedure (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

static DWORD print_GetLastError (char *message_prefix)
{
  LPVOID ptr;
  DWORD error;

  ptr = 0;
  error = GetLastError ( );
  if (FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM,
                  nullptr,
                  error,
                  MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
                  (LPTSTR)&ptr,
                  0, nullptr))
  {
    std::cout << "ERROR: "<< message_prefix << " result = " << (char*) ptr << "\n";
    LocalFree( ptr );
  }

  return error;
}

static int get_display_information (DisplaySearchParameters &display_search_parameters, DisplayInformation *display_information) {

  int debug = false;

  int success;
  DisplayInformation::DetectionState state;
  int get_adapter_display_mode_state;
  int get_device_caps_state;

  GraphicsStateGuardian::ShaderModel shader_model;
  UINT minimum_width;
  UINT maximum_width;
  UINT minimum_height;
  UINT maximum_height;
  int minimum_bits_per_pixel;
  int maximum_bits_per_pixel;

  UINT texture_memory;
  UINT video_memory;

  int window_width;
  int window_height;
  int window_bits_per_pixel;
  int total_display_modes;
  DisplayMode *display_mode_array;

  uint64_t physical_memory;
  uint64_t available_physical_memory;

  int vendor_id;
  int device_id;

  int product;
  int version;
  int sub_version;
  int build;

  int month;
  int day;
  int year;

  success = false;
  window_width = 0;
  window_height = 0;
  window_bits_per_pixel = 0;
  total_display_modes = 0;
  display_mode_array = nullptr;

  minimum_width = display_search_parameters._minimum_width;
  minimum_height = display_search_parameters._minimum_height;
  maximum_width = display_search_parameters._maximum_width;
  maximum_height = display_search_parameters._maximum_height;
  minimum_bits_per_pixel = display_search_parameters._minimum_bits_per_pixel;
  maximum_bits_per_pixel = display_search_parameters._maximum_bits_per_pixel;

  shader_model = GraphicsStateGuardian::SM_00;
  video_memory = 0;
  texture_memory = 0;

  state = DisplayInformation::DS_unknown;
  get_adapter_display_mode_state = false;
  get_device_caps_state = false;

  physical_memory = 0;
  available_physical_memory = 0;

  vendor_id = 0;
  device_id = 0;

  product = 0;
  version = 0;
  sub_version = 0;
  build = 0;

  month = 0;
  day = 0;
  year = 0;

  HMODULE d3d_dll;
  DIRECT_3D_CREATE Direct3DCreate;

  d3d_dll = LoadLibrary (d3d_dll_name);
  if (d3d_dll) {
    Direct3DCreate = (DIRECT_3D_CREATE) GetProcAddress (d3d_dll, direct_3d_create_function_name);
    if (Direct3DCreate) {
      DIRECT_3D direct_3d;

      direct_3d = Direct3DCreate (D3D_SDK_VERSION);
      if (direct_3d != nullptr) {
        DWORD flags;
        UINT adapter;
        D3DDEVTYPE device_type;
        D3DDISPLAYMODE current_d3d_display_mode;
        D3DADAPTER_IDENTIFIER d3d_adapter_identifier;
        D3DCAPS d3d_caps;

        adapter = D3DADAPTER_DEFAULT;
        device_type = D3DDEVTYPE_HAL;

        // windowed mode max res and format
        if (direct_3d -> GetAdapterDisplayMode (adapter, &current_d3d_display_mode) == D3D_OK) {
          if (debug) {
            printf ("current mode  w = %d h = %d r = %d f = %d \n",
              current_d3d_display_mode.Width,
              current_d3d_display_mode.Height,
              current_d3d_display_mode.RefreshRate,
              current_d3d_display_mode.Format);
          }

          window_width = current_d3d_display_mode.Width;
          window_height = current_d3d_display_mode.Height;
          window_bits_per_pixel = d3d_format_to_bits_per_pixel (current_d3d_display_mode.Format);

          get_adapter_display_mode_state = true;
        }
        else {
          get_adapter_display_mode_state = false;
        }

        flags = 0;
        if (direct_3d -> GetAdapterIdentifier (adapter, flags, &d3d_adapter_identifier) == D3D_OK) {
          // print adapter info
          d3d_adapter_identifier.Driver;
          d3d_adapter_identifier.Description;
          d3d_adapter_identifier.DeviceName;
          d3d_adapter_identifier.DriverVersion;
          d3d_adapter_identifier.VendorId;
          d3d_adapter_identifier.DeviceId;
          d3d_adapter_identifier.SubSysId;
          d3d_adapter_identifier.Revision;
          d3d_adapter_identifier.DeviceIdentifier;
          d3d_adapter_identifier.WHQLLevel;

          if (debug) {
            printf ("Driver: %s\n", d3d_adapter_identifier.Driver);
            printf ("Description: %s\n", d3d_adapter_identifier.Description);
          }

          char system_directory [MAX_PATH];
          char dll_file_path [MAX_PATH];

          // find the dll in the system directory if possible and get the date
          // of the file
          if (GetSystemDirectory (system_directory, MAX_PATH) > 0) {
            if (debug) {
              printf ("system_directory = %s \n", system_directory);
            }
            sprintf (dll_file_path, "%s\\%s", system_directory, d3d_adapter_identifier.Driver);

            intptr_t find;
            struct _finddata_t find_data;

            find = _findfirst (dll_file_path, &find_data);
            if (find != -1) {
              struct tm *dll_time;

              dll_time = localtime(&find_data.time_write);

              month = dll_time -> tm_mon + 1;
              day = dll_time -> tm_mday;
              year = dll_time -> tm_year + 1900;

              if (debug) {
                printf ("Driver Date: %d/%d/%d\n",  month, day, year);
              }

              _findclose (find);
            }
          }

          /*
          HMODULE driver_dll;

          driver_dll = LoadLibrary (d3d_adapter_identifier.Driver);
          if (driver_dll)
          {
            DWORD length;
            TCHAR file_path [MAX_PATH];

            length = GetModuleFileName(driver_dll, file_path, MAX_PATH);
            if (length > 0)
            {
              printf ("DLL file path = %s \n", file_path);
            }
            else
            {
              printf ("ERROR: could not get GetModuleFileName for %s \n", d3d_adapter_identifier.Driver);
            }

            FreeLibrary (driver_dll);
          }
          else
          {
            print_GetLastError ("");
            printf ("ERROR: could not load = %s \n", d3d_adapter_identifier.Driver);
          }
          */

          if (debug) {
            printf ("VendorId = 0x%x\n", d3d_adapter_identifier.VendorId);
            printf ("DeviceId = 0x%x\n", d3d_adapter_identifier.DeviceId);
          }

          vendor_id = d3d_adapter_identifier.VendorId;
          device_id = d3d_adapter_identifier.DeviceId;

          product = HIWORD(d3d_adapter_identifier.DriverVersion.HighPart);
          version = LOWORD(d3d_adapter_identifier.DriverVersion.HighPart);
          sub_version = HIWORD(d3d_adapter_identifier.DriverVersion.LowPart);
          build = LOWORD(d3d_adapter_identifier.DriverVersion.LowPart);

          if (debug) {
            printf ("DRIVER VERSION: %d.%d.%d.%d \n", product, version, sub_version, build);
          }
        }

        if (direct_3d -> GetDeviceCaps (adapter, device_type, &d3d_caps) == D3D_OK) {

          int vertex_shader_version_major;
          int vertex_shader_version_minor;
          int pixel_shader_version_major;
          int pixel_shader_version_minor;

          vertex_shader_version_major = D3DSHADER_VERSION_MAJOR (d3d_caps.VertexShaderVersion);
          vertex_shader_version_minor = D3DSHADER_VERSION_MINOR (d3d_caps.VertexShaderVersion);
          pixel_shader_version_major = D3DSHADER_VERSION_MAJOR (d3d_caps.PixelShaderVersion);
          pixel_shader_version_minor = D3DSHADER_VERSION_MINOR (d3d_caps.PixelShaderVersion);

          switch (pixel_shader_version_major)
          {
            case 0:
              shader_model = GraphicsStateGuardian::SM_00;
              break;
            case 1:
              shader_model = GraphicsStateGuardian::SM_11;
              break;
            case 2:
              // minimim specification for pixel shader 2.0 is 96 instruction
              // slots
              shader_model = GraphicsStateGuardian::SM_20;
              if (d3d_caps.PS20Caps.NumInstructionSlots >= 512) {
                shader_model = GraphicsStateGuardian::SM_2X;
              }
              break;
            case 3:
              shader_model = GraphicsStateGuardian::SM_30;
              break;
            case 4:
            default:
              shader_model = GraphicsStateGuardian::SM_40;
              break;
          }

          if (debug) {
            printf ("shader_model = %d \n", shader_model);
          }
          get_device_caps_state = true;
        }
        else {
          get_device_caps_state = false;
        }

        // count display modes
        int format_index;
        int maximum_display_modes;
        UINT display_mode_count;
        D3DFORMAT d3d_format;

        format_index = 0;
        maximum_display_modes = 0;

        while (display_format_array [format_index].d3d_format != D3DFMT_UNKNOWN) {
          d3d_format = display_format_array [format_index].d3d_format;

          display_mode_count = direct_3d -> GetAdapterModeCount (adapter, d3d_format);
          if (display_mode_count > 0) {
            UINT mode_index;
            D3DDISPLAYMODE d3d_display_mode;

            for (mode_index = 0; mode_index < display_mode_count; mode_index++) {
              if (direct_3d -> EnumAdapterModes (adapter, d3d_format, mode_index, &d3d_display_mode) == D3D_OK) {
                if (d3d_display_mode.Width >= minimum_width && d3d_display_mode.Height >= minimum_height &&
                    d3d_display_mode.Width <= maximum_width && d3d_display_mode.Height <= maximum_height) {
                  if (display_format_array [format_index].bits_per_pixel >= minimum_bits_per_pixel &&
                      display_format_array [format_index].bits_per_pixel <= maximum_bits_per_pixel) {
                    if (d3d_format == d3d_display_mode.Format) {
                      maximum_display_modes++;
                    }
                  }
                }
              }
            }
          }

          format_index++;
        }

        if (debug) {
          printf ("maximum_display_modes %d \n", maximum_display_modes);
        }

        display_mode_array = new DisplayMode [maximum_display_modes];

        format_index = 0;
        while (display_format_array [format_index].d3d_format != D3DFMT_UNKNOWN) {
          d3d_format = display_format_array [format_index].d3d_format;
          display_mode_count = direct_3d -> GetAdapterModeCount (adapter, d3d_format);
          if (display_mode_count > 0) {
            UINT mode_index;
            D3DDISPLAYMODE d3d_display_mode;

            for (mode_index = 0; mode_index < display_mode_count; mode_index++) {
              if (direct_3d -> EnumAdapterModes (adapter, d3d_format, mode_index, &d3d_display_mode) == D3D_OK) {
                if (d3d_display_mode.Width >= minimum_width && d3d_display_mode.Height >= minimum_height &&
                    d3d_display_mode.Width <= maximum_width && d3d_display_mode.Height <= maximum_height) {
                  if (display_format_array [format_index].bits_per_pixel >= minimum_bits_per_pixel &&
                      display_format_array [format_index].bits_per_pixel <= maximum_bits_per_pixel) {
                    if (debug) {
                      printf ("w = %d h = %d r = %d f = %d \n",
                        d3d_display_mode.Width,
                        d3d_display_mode.Height,
                        d3d_display_mode.RefreshRate,
                        d3d_display_mode.Format);
                    }

                    if (d3d_format == d3d_display_mode.Format) {
                      DisplayMode *display_mode;

                      display_mode = &display_mode_array [total_display_modes];
                      display_mode -> width = d3d_display_mode.Width;
                      display_mode -> height = d3d_display_mode.Height;
                      display_mode -> bits_per_pixel = display_format_array [format_index].bits_per_pixel;
                      display_mode -> refresh_rate = d3d_display_mode.RefreshRate;
                      display_mode -> fullscreen_only = display_format_array [format_index].fullscreen_only;

                      total_display_modes++;
                    }
                  }
                }
              }
            }
          }

          format_index++;
        }

        UINT width;
        UINT height;

        width = 640;
        height = 480;

        // make a window
        WNDCLASSEX window_class =
        {
          sizeof (WNDCLASSEX), CS_CLASSDC, window_procedure, 0L, 0L,
          GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
          "class_name", nullptr
        };
        RegisterClassEx (&window_class);

        HWND window_handle;

        window_handle = CreateWindow ("class_name", "window_name", WS_DISABLED, 0, 0, width, height, (HWND) nullptr, (HMENU) nullptr, window_class.hInstance, nullptr);
        if (window_handle != nullptr) {
          ShowWindow (window_handle, SW_HIDE);

          DIRECT_3D_DEVICE direct_3d_device;
          D3DPRESENT_PARAMETERS present_parameters;
          DWORD behavior_flags;

          direct_3d_device = 0;
          memset (&present_parameters, 0, sizeof (D3DPRESENT_PARAMETERS));

          present_parameters.BackBufferWidth = width;
          present_parameters.BackBufferHeight = height;
          present_parameters.BackBufferFormat = D3DFMT_X8R8G8B8;
          present_parameters.BackBufferCount = 1;

          present_parameters.SwapEffect = D3DSWAPEFFECT_FLIP;
          present_parameters.hDeviceWindow = window_handle;

          present_parameters.Windowed = true;
          present_parameters.EnableAutoDepthStencil = true;
          present_parameters.AutoDepthStencilFormat = D3DFMT_D24S8;

          present_parameters.FullScreen_RefreshRateInHz;
          present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

          if (d3d_caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
            behavior_flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
          }
          else {
            behavior_flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
          }
          // This is important to prevent DirectX from forcing the FPU into
          // single-precision mode.
          behavior_flags |= D3DCREATE_FPU_PRESERVE;

          HRESULT result;

          result = direct_3d -> CreateDevice (adapter, device_type, window_handle, behavior_flags, &present_parameters, &direct_3d_device);
          if (result == D3D_OK) {

            // allocate 512x512 32-bit textures (1MB size) until we run out or
            // hit the limit
            #define MAXIMUM_TEXTURES (2048 - 1)

            int total_textures;
            HRESULT texture_result;
            IDirect3DTexture9 *texture_array [MAXIMUM_TEXTURES];

            total_textures = 0;
            while (total_textures < MAXIMUM_TEXTURES) {

              texture_result = direct_3d_device -> CreateTexture (
                512,
                512,
                1,
                D3DUSAGE_RENDERTARGET,
                D3DFMT_A8R8G8B8,
                D3DPOOL_DEFAULT,
                &texture_array [total_textures],
                nullptr);
              if (texture_result == D3D_OK) {
                total_textures++;
              }
              else {
                if (texture_result == D3DERR_OUTOFVIDEOMEMORY) {
                  if (debug) {
                    printf ("D3DERR_OUTOFVIDEOMEMORY \n");
                  }
                }
                break;
              }
            }

            // free all allocated textures
            int index;
            for (index = 0; index < total_textures; index++) {
              texture_array [index] -> Release ( );
            }

            video_memory = (total_textures * 1024 * 1024);

            if (debug) {
              printf ("video_memory = %d \n", video_memory);
            }

            texture_memory = direct_3d_device -> GetAvailableTextureMem ( );
            if (debug) {
              printf ("texture_memory = %d \n", texture_memory);
            }

            direct_3d_device -> Release ( );

            state = DisplayInformation::DS_success;
            success = true;
          }
          else
          {
            if (debug) {
              printf ("CreateDevice failed.\n");
            }

            state = DisplayInformation::DS_create_device_error;
            success = true;
          }

          DestroyWindow (window_handle);
        }
        else {
          _GetLastError ("CreateWindow");
          state = DisplayInformation::DS_create_window_error;
        }

        direct_3d -> Release ( );
      }
      else {
        state = DisplayInformation::DS_direct_3d_create_error;
      }
    }
    else {
      state = DisplayInformation::DS_direct_3d_create_error;
    }

    FreeLibrary (d3d_dll);
  }
  else {
    state = DisplayInformation::DS_direct_3d_create_error;
  }

  if (success) {
    display_information -> _state = state;
    display_information -> _get_adapter_display_mode_state = get_adapter_display_mode_state;
    display_information -> _get_device_caps_state = get_device_caps_state;
    display_information -> _maximum_window_width = window_width;
    display_information -> _maximum_window_height = window_height;
    display_information -> _window_bits_per_pixel = window_bits_per_pixel;
    display_information -> _total_display_modes = total_display_modes;
    display_information -> _display_mode_array = display_mode_array;
    display_information -> _shader_model = shader_model;
    display_information -> _video_memory = video_memory;
    display_information -> _texture_memory = texture_memory;
    display_information -> _vendor_id = vendor_id;
    display_information -> _device_id = device_id;
    display_information -> _driver_product = product;
    display_information -> _driver_version = version;
    display_information -> _driver_sub_version = sub_version;
    display_information -> _driver_build = build;

    display_information -> _driver_date_month = month;
    display_information -> _driver_date_day = day;
    display_information -> _driver_date_year = year;
  }

  // memory
  bool memory_state;
  HMODULE kernel32_dll;

  memory_state = false;
  kernel32_dll = LoadLibrary ("kernel32.dll");
  if (kernel32_dll) {
    GlobalMemoryStatusExType GlobalMemoryStatusExFunction;

    GlobalMemoryStatusExFunction = (GlobalMemoryStatusExType) GetProcAddress (kernel32_dll, "GlobalMemoryStatusEx");
    if (GlobalMemoryStatusExFunction) {
      MEMORYSTATUSEX memory_status;

      memory_status.dwLength = sizeof (MEMORYSTATUSEX);
      if (GlobalMemoryStatusExFunction (&memory_status)) {
        physical_memory = memory_status.ullTotalPhys;
        available_physical_memory = memory_status.ullAvailPhys;
        memory_state = true;
      }
    }
    FreeLibrary (kernel32_dll);
  }
  if (memory_state == false) {
    MEMORYSTATUS memory_status;

    memory_status.dwLength = sizeof (MEMORYSTATUS);
    GlobalMemoryStatus (&memory_status);

    physical_memory = memory_status.dwTotalPhys;
    available_physical_memory = memory_status.dwAvailPhys;
  }

  if (debug) {
    printf ("physical_memory %I64d \n", physical_memory);
    printf ("available_physical_memory %I64d \n", available_physical_memory);
  }

  display_information -> _physical_memory = physical_memory;
  display_information -> _available_physical_memory = available_physical_memory;

  return success;
}
