/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winGraphicsPipe.cxx
 * @author drose
 * @date 2002-12-20
 */

#include "winGraphicsPipe.h"
#include "config_windisplay.h"
#include "displaySearchParameters.h"
#include "displayInformation.h"
#include "dtool_config.h"
#include "pbitops.h"

#include <psapi.h>
#include <powrprof.h>
#include <intrin.h>

TypeHandle WinGraphicsPipe::_type_handle;

#ifndef MAXIMUM_PROCESSORS
#define MAXIMUM_PROCESSORS 32
#endif

typedef enum _Process_DPI_Awareness {
  Process_DPI_Unaware            = 0,
  Process_System_DPI_Aware       = 1,
  Process_Per_Monitor_DPI_Aware  = 2
} Process_DPI_Awareness;

typedef struct _PROCESSOR_POWER_INFORMATION {
  ULONG Number;
  ULONG MaxMhz;
  ULONG CurrentMhz;
  ULONG MhzLimit;
  ULONG MaxIdleState;
  ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

typedef BOOL (WINAPI *GetProcessMemoryInfoType) (HANDLE Process, PROCESS_MEMORY_COUNTERS *ppsmemCounters, DWORD cb);
typedef long (__stdcall *CallNtPowerInformationType) (POWER_INFORMATION_LEVEL information_level, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength);

static int initialize = false;
static HMODULE psapi_dll = 0;
static GetProcessMemoryInfoType GetProcessMemoryInfoFunction = 0;
static CallNtPowerInformationType CallNtPowerInformationFunction = 0;

void get_memory_information (DisplayInformation *display_information) {
  if (initialize == false) {
    psapi_dll = LoadLibrary("psapi.dll");
    if (psapi_dll) {
      GetProcessMemoryInfoFunction = (GetProcessMemoryInfoType) GetProcAddress(psapi_dll, "GetProcessMemoryInfo");
    }

    initialize = true;
  }

  MEMORYSTATUSEX memory_status;

  memory_status.dwLength = sizeof(MEMORYSTATUSEX);
  if (GlobalMemoryStatusEx(&memory_status)) {
    display_information->_physical_memory = memory_status.ullTotalPhys;
    display_information->_available_physical_memory = memory_status.ullAvailPhys;
    display_information->_page_file_size = memory_status.ullTotalPageFile;
    display_information->_available_page_file_size = memory_status.ullAvailPageFile;
    display_information->_process_virtual_memory = memory_status.ullTotalVirtual;
    display_information->_available_process_virtual_memory = memory_status.ullAvailVirtual;
    display_information->_memory_load = memory_status.dwMemoryLoad;
  }

  if (GetProcessMemoryInfoFunction) {
    HANDLE process;
    DWORD process_id;
    PROCESS_MEMORY_COUNTERS process_memory_counters;

    process_id = GetCurrentProcessId();
    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
    if (process) {
      if (GetProcessMemoryInfoFunction (process, &process_memory_counters, sizeof(PROCESS_MEMORY_COUNTERS))) {
        display_information->_page_fault_count =  process_memory_counters.PageFaultCount;
        display_information->_process_memory =  process_memory_counters.WorkingSetSize;
        display_information->_peak_process_memory = process_memory_counters.PeakWorkingSetSize;
        display_information->_page_file_usage = process_memory_counters.PagefileUsage;
        display_information->_peak_page_file_usage = process_memory_counters.PeakPagefileUsage;
      }

      CloseHandle(process);
    }
  }
}

int update_cpu_frequency_function(int processor_number, DisplayInformation *display_information) {
  int update;

  update = false;
  display_information->_maximum_cpu_frequency = 0;
  display_information->_current_cpu_frequency = 0;

  if (CallNtPowerInformationFunction) {

    int i;
    PVOID input_buffer;
    PVOID output_buffer;
    ULONG input_buffer_size;
    ULONG output_buffer_size;
    POWER_INFORMATION_LEVEL information_level;
    PROCESSOR_POWER_INFORMATION *processor_power_information;
    PROCESSOR_POWER_INFORMATION processor_power_information_array [MAXIMUM_PROCESSORS];

    memset(processor_power_information_array, 0, sizeof(PROCESSOR_POWER_INFORMATION) * MAXIMUM_PROCESSORS);

    processor_power_information = processor_power_information_array;
    for (i = 0; i < MAXIMUM_PROCESSORS; i++) {
      processor_power_information->Number = 0xFFFFFFFF;
      processor_power_information++;
    }

    information_level = ProcessorInformation;
    input_buffer = nullptr;
    output_buffer = processor_power_information_array;
    input_buffer_size = 0;
    output_buffer_size = sizeof(PROCESSOR_POWER_INFORMATION) * MAXIMUM_PROCESSORS;
    if (CallNtPowerInformationFunction(information_level, input_buffer, input_buffer_size, output_buffer, output_buffer_size) == 0) {
      processor_power_information = processor_power_information_array;
      for (i = 0; i < MAXIMUM_PROCESSORS; i++) {
        if (processor_power_information->Number == processor_number) {
          uint64_t value;

          value = processor_power_information->MaxMhz;
          display_information->_maximum_cpu_frequency = value * 1000000;

          value = processor_power_information->CurrentMhz;
          display_information->_current_cpu_frequency = value * 1000000;
          update = true;

          break;
        }

        processor_power_information++;
      }
    }
  }

  return update;
}

void
count_number_of_cpus(DisplayInformation *display_information) {
  int num_cpu_cores = 0;
  int num_logical_cpus = 0;

  // Get a pointer to the GetLogicalProcessorInformation function.
  typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
                                   PDWORD);
  LPFN_GLPI glpi;
  glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")),
                                    "GetLogicalProcessorInformation");
  if (glpi == nullptr) {
    windisplay_cat.info()
      << "GetLogicalProcessorInformation is not supported.\n";
    return;
  }

  // Allocate a buffer to hold the result of the
  // GetLogicalProcessorInformation call.
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr;
  DWORD buffer_length = 0;
  DWORD rc = glpi(buffer, &buffer_length);
  while (!rc) {
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      if (buffer != nullptr) {
        PANDA_FREE_ARRAY(buffer);
      }

      buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)PANDA_MALLOC_ARRAY(buffer_length);
      nassertv(buffer != nullptr);
    } else {
      windisplay_cat.info()
        << "GetLogicalProcessorInformation failed: " << GetLastError()
        << "\n";
      return;
    }
    rc = glpi(buffer, &buffer_length);
  }

  // Now get the results.
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION end = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)((char *)buffer + buffer_length);

  while (ptr < end) {
    if (ptr->Relationship == RelationProcessorCore) {
      num_cpu_cores++;

      // A hyperthreaded core supplies more than one logical processor.
      num_logical_cpus += count_bits_in_word((uint64_t)(ptr->ProcessorMask));
    }
    ++ptr;
  }

  PANDA_FREE_ARRAY(buffer);

  windisplay_cat.info()
    << num_cpu_cores << " CPU cores, with "
    << num_logical_cpus << " logical processors.\n";

  display_information->_num_cpu_cores = num_cpu_cores;
  display_information->_num_logical_cpus = num_logical_cpus;
}


/**
 *
 */
WinGraphicsPipe::
WinGraphicsPipe() {
  char string [512];

  _supported_types = OT_window | OT_fullscreen_window;

  HMODULE user32 = GetModuleHandleA("user32.dll");
  if (user32 != nullptr) {
    if (dpi_aware) {
      typedef HRESULT (WINAPI *PFN_SETPROCESSDPIAWARENESS)(Process_DPI_Awareness);
      PFN_SETPROCESSDPIAWARENESS pfnSetProcessDpiAwareness =
        (PFN_SETPROCESSDPIAWARENESS)GetProcAddress(user32, "SetProcessDpiAwarenessInternal");

      if (pfnSetProcessDpiAwareness == nullptr) {
        if (windisplay_cat.is_debug()) {
          windisplay_cat.debug() << "Unable to find SetProcessDpiAwareness in user32.dll.\n";
        }
      } else {
        if (windisplay_cat.is_debug()) {
          windisplay_cat.debug() << "Calling SetProcessDpiAwareness().\n";
        }
        pfnSetProcessDpiAwareness(Process_Per_Monitor_DPI_Aware);
      }
    }
  }

#ifdef HAVE_DX9
  // Use D3D to get display info.  This is disabled by default as it is slow.
  if (request_dxdisplay_information) {
    if (windisplay_cat.is_debug()) {
      windisplay_cat.debug() << "Using Direct3D 9 to fetch display information.\n";
    }
    DisplaySearchParameters display_search_parameters_dx9;
    int dx9_display_information (DisplaySearchParameters &display_search_parameters_dx9, DisplayInformation *display_information);
    dx9_display_information(display_search_parameters_dx9, _display_information);
  } else
#endif
  {
    // Use the Win32 API to query the available display modes.
    if (windisplay_cat.is_debug()) {
      windisplay_cat.debug() << "Using EnumDisplaySettings to fetch display information.\n";
    }
    pvector<DisplayMode> display_modes;
    DEVMODE dm{};
    dm.dmSize = sizeof(dm);
    for (int i = 0; EnumDisplaySettings(nullptr, i, &dm) != 0; ++i) {
      DisplayMode mode;
      mode.width = dm.dmPelsWidth;
      mode.height = dm.dmPelsHeight;
      mode.bits_per_pixel = dm.dmBitsPerPel;
      mode.refresh_rate = dm.dmDisplayFrequency;
      mode.fullscreen_only = 0;
      if (i == 0 || mode != display_modes.back()) {
        display_modes.push_back(mode);
      }
    }

    // Copy this information to the DisplayInformation object.
    _display_information->_total_display_modes = display_modes.size();
    if (!display_modes.empty()) {
      _display_information->_display_mode_array = new DisplayMode[display_modes.size()];
      std::copy(display_modes.begin(), display_modes.end(),
                _display_information->_display_mode_array);
    }
  }

  if (auto_cpu_data) {
    lookup_cpu_data();
  }

  OSVERSIONINFO version_info;

  version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (GetVersionEx(&version_info)) {
    if (windisplay_cat.is_info()) {
      sprintf(string, "OS version: %lu.%lu.%lu.%lu\n", version_info.dwMajorVersion, version_info.dwMinorVersion, version_info.dwPlatformId, version_info.dwBuildNumber);
      windisplay_cat.info() << string;
      windisplay_cat.info() << "  " << version_info.szCSDVersion << "\n";
    }

    _display_information->_os_version_major = version_info.dwMajorVersion;
    _display_information->_os_version_minor = version_info.dwMinorVersion;
    _display_information->_os_version_build = version_info.dwBuildNumber;
    _display_information->_os_platform_id = version_info.dwPlatformId;
  }
  // Screen size
  _display_width = GetSystemMetrics(SM_CXSCREEN);
  _display_height = GetSystemMetrics(SM_CYSCREEN);

  HMODULE power_dll;

  power_dll = LoadLibrary("PowrProf.dll");
  if (power_dll) {
    CallNtPowerInformationFunction = (CallNtPowerInformationType) GetProcAddress(power_dll, "CallNtPowerInformation");
    if (CallNtPowerInformationFunction) {

      _display_information->_update_cpu_frequency_function = update_cpu_frequency_function;
      update_cpu_frequency_function(0, _display_information);

      sprintf(string, "max Mhz %I64d, current Mhz %I64d\n", _display_information->_maximum_cpu_frequency, _display_information->_current_cpu_frequency);

      windisplay_cat.info() << string;
    }
  }
}

/**
 * Looks up the detailed CPU information and stores it in
 * _display_information, if supported by the OS. This may take a second or
 * two.
 */
void WinGraphicsPipe::
lookup_cpu_data() {
  char string [512];

  // set callback for memory function
  _display_information->_get_memory_information_function = get_memory_information;

  // determine CPU frequency
  uint64_t time;
  uint64_t end_time;
  LARGE_INTEGER counter;
  LARGE_INTEGER end;
  LARGE_INTEGER frequency;

  time = 0;
  end_time = 0;
  counter.QuadPart = 0;
  end.QuadPart = 0;
  frequency.QuadPart = 0;

  int priority;
  HANDLE thread;

  windisplay_cat.info() << "begin QueryPerformanceFrequency\n";
  thread = GetCurrentThread();
  priority = GetThreadPriority (thread);
  SetThreadPriority(thread, THREAD_PRIORITY_TIME_CRITICAL);

  if (QueryPerformanceFrequency(&frequency)) {
    if (frequency.QuadPart > 0) {
      if (QueryPerformanceCounter (&counter)) {
        time = __rdtsc();
        end.QuadPart = counter.QuadPart + frequency.QuadPart;
        while (QueryPerformanceCounter (&counter) && counter.QuadPart < end.QuadPart) {

        }
        end_time = __rdtsc();

        _display_information->_cpu_frequency = end_time - time;
      }
    }
  }

  SetThreadPriority(thread, priority);
  sprintf(string, "QueryPerformanceFrequency: %I64d\n", frequency.QuadPart);
  windisplay_cat.info() << string;
  sprintf(string, "CPU frequency: %I64d\n", _display_information->_cpu_frequency);
  windisplay_cat.info() << string;

  // Number of CPU's
  count_number_of_cpus(_display_information);
}

/**
 *
 */
WinGraphicsPipe::
~WinGraphicsPipe() {
}
