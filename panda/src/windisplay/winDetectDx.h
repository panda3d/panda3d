// Filename: winDetectDx.h
// Created by:  aignacio (18Jan07)
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

#if DX8 
#else
  D3DFMT_A2R10G10B10, 32, TRUE,
#endif

  // terminator
  D3DFMT_UNKNOWN,      0, FALSE,
};

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
        NULL, error, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
        (LPTSTR)&ptr,0, NULL)) {
    cout << "ERROR: "<< message_prefix << " result = " << (char*) ptr << "\n";
    LocalFree( ptr );
  }

  return error;
}

static LRESULT CALLBACK window_procedure (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

static int get_display_information (DisplaySearchParameters &display_search_parameters, DisplayInformation *display_information) {

  int debug = false;
  
  int success;
  DisplayInformation::DetectionState state;
  int get_adapter_display_mode_state;
  int get_device_caps_state;

  int shader_model;
  UINT minimum_width;
  UINT maximum_width;
  UINT minimum_height;
  UINT maximum_height;
  int minimum_bits_per_pixel;
  int maximum_bits_per_pixel;

  UINT texture_memory;

  int window_width;
  int window_height;
  int window_bits_per_pixel;
  int total_display_modes;
  DisplayMode *display_mode_array;

  PN_uint64 physical_memory;
  PN_uint64 available_physical_memory;

  success = false;
  window_width = 0;
  window_height = 0;
  window_bits_per_pixel = 0;
  total_display_modes = 0;
  display_mode_array = NULL;

  minimum_width = display_search_parameters._minimum_width;
  minimum_height = display_search_parameters._minimum_height;
  maximum_width = display_search_parameters._maximum_width;
  maximum_height = display_search_parameters._maximum_height;
  minimum_bits_per_pixel = display_search_parameters._minimum_bits_per_pixel;
  maximum_bits_per_pixel = display_search_parameters._maximum_bits_per_pixel;

  shader_model = GraphicsStateGuardian::SM_00;
  texture_memory = 0;

  state = DisplayInformation::DS_unknown;    
  get_adapter_display_mode_state = false;
  get_device_caps_state = false;

  physical_memory = 0;
  available_physical_memory = 0;
  
  HMODULE d3d_dll;
  DIRECT_3D_CREATE Direct3DCreate;

  d3d_dll = LoadLibrary (d3d_dll_name);
  if (d3d_dll) {
    Direct3DCreate = (DIRECT_3D_CREATE) GetProcAddress (d3d_dll, direct_3d_create_function_name);
    if (Direct3DCreate) {
      DIRECT_3D direct_3d;

      direct_3d = Direct3DCreate (D3D_SDK_VERSION);
      if (direct_3d != NULL) {
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
        }

        if (direct_3d -> GetDeviceCaps (adapter, device_type, &d3d_caps) == D3D_OK) {

#if DX8
          // shaders not supported in DX8
#else
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
              // minimim specification for pixel shader 2.0 is 96 instruction slots
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
#endif

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

#if DX8
          display_mode_count = direct_3d -> GetAdapterModeCount (adapter);
#else
          display_mode_count = direct_3d -> GetAdapterModeCount (adapter, d3d_format);
#endif
          if (display_mode_count > 0) {
            UINT mode_index;
            D3DDISPLAYMODE d3d_display_mode;

            for (mode_index = 0; mode_index < display_mode_count; mode_index++) {
#if DX8
              if (direct_3d -> EnumAdapterModes (adapter, mode_index, &d3d_display_mode) == D3D_OK)
#else
              if (direct_3d -> EnumAdapterModes (adapter, d3d_format, mode_index, &d3d_display_mode) == D3D_OK)
#endif          
              {
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
#if DX8
          display_mode_count = direct_3d -> GetAdapterModeCount (adapter);
#else
          display_mode_count = direct_3d -> GetAdapterModeCount (adapter, d3d_format);
#endif      
          if (display_mode_count > 0) {
            UINT mode_index;
            D3DDISPLAYMODE d3d_display_mode;

            for (mode_index = 0; mode_index < display_mode_count; mode_index++) {
#if DX8
              if (direct_3d -> EnumAdapterModes (adapter, mode_index, &d3d_display_mode) == D3D_OK)
#else
              if (direct_3d -> EnumAdapterModes (adapter, d3d_format, mode_index, &d3d_display_mode) == D3D_OK)
#endif          
              {
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
          GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
          "class_name", NULL 
        };
        RegisterClassEx (&window_class);

        HWND window_handle;

        window_handle = CreateWindow ("class_name", "window_name", WS_DISABLED, 0, 0, width, height, (HWND) NULL, (HMENU) NULL, window_class.hInstance, NULL);
        if (window_handle != NULL) {
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

#if DX8
#else
          present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
#endif

          if (d3d_caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
            behavior_flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
          }
          else {
            behavior_flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
          }

          HRESULT result;

          result = direct_3d -> CreateDevice (adapter, device_type, window_handle, behavior_flags, &present_parameters, &direct_3d_device);
          if (result == D3D_OK) {  
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
    display_information -> _texture_memory = texture_memory;
  }

  // memory
  MEMORYSTATUSEX memory_status;

  memory_status.dwLength = sizeof (MEMORYSTATUSEX);
  if (GlobalMemoryStatusEx (&memory_status)) {
    physical_memory = memory_status.ullTotalPhys;
    available_physical_memory = memory_status.ullAvailPhys;
  }
  else {
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
