// Filename: winGraphicsPipe.cxx
// Created by:  drose (20Dec02)
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

#include "winGraphicsPipe.h"
#include "config_windisplay.h"
#include "displaySearchParameters.h"
#include "displayInformation.h"
#include "dtool_config.h"
#include "pbitops.h"

#include "psapi.h"
#include "powrprof.h"

#ifdef _WIN64
#include <intrin.h>
#endif

TypeHandle WinGraphicsPipe::_type_handle;

#ifndef MAXIMUM_PROCESSORS
#define MAXIMUM_PROCESSORS 32
#endif

typedef struct _PROCESSOR_POWER_INFORMATION {
  ULONG Number;
  ULONG MaxMhz;
  ULONG CurrentMhz;
  ULONG MhzLimit;
  ULONG MaxIdleState;
  ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

typedef BOOL (WINAPI *GetProcessMemoryInfoType) (HANDLE Process, PROCESS_MEMORY_COUNTERS *ppsmemCounters, DWORD cb);
typedef BOOL (WINAPI *GlobalMemoryStatusExType) (LPMEMORYSTATUSEX lpBuffer);
typedef long (__stdcall *CallNtPowerInformationType) (POWER_INFORMATION_LEVEL information_level, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength);

static int initialize = false;
static HMODULE psapi_dll = 0;
static HMODULE kernel32_dll = 0;
static HMODULE power_dll = 0;
static GetProcessMemoryInfoType GetProcessMemoryInfoFunction = 0;
static GlobalMemoryStatusExType GlobalMemoryStatusExFunction = 0;
static CallNtPowerInformationType CallNtPowerInformationFunction = 0;

void get_memory_information (DisplayInformation *display_information) {
  if (initialize == false) {
    psapi_dll = LoadLibrary("psapi.dll");
    if (psapi_dll) {
      GetProcessMemoryInfoFunction = (GetProcessMemoryInfoType) GetProcAddress(psapi_dll, "GetProcessMemoryInfo");
    }

    kernel32_dll = LoadLibrary("kernel32.dll");
    if (kernel32_dll) {
      GlobalMemoryStatusExFunction = (GlobalMemoryStatusExType) GetProcAddress(kernel32_dll, "GlobalMemoryStatusEx");
    }

    initialize = true;
  }

  if (GlobalMemoryStatusExFunction) {
    MEMORYSTATUSEX memory_status;

    memory_status.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusExFunction(&memory_status)) {
      display_information->_physical_memory = memory_status.ullTotalPhys;
      display_information->_available_physical_memory = memory_status.ullAvailPhys;
      display_information->_page_file_size = memory_status.ullTotalPageFile;
      display_information->_available_page_file_size = memory_status.ullAvailPageFile;
      display_information->_process_virtual_memory = memory_status.ullTotalVirtual;
      display_information->_available_process_virtual_memory = memory_status.ullAvailVirtual;
      display_information->_memory_load = memory_status.dwMemoryLoad;
    }
  } else {
    MEMORYSTATUS memory_status;

    memory_status.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus (&memory_status);

    display_information->_physical_memory = memory_status.dwTotalPhys;
    display_information->_available_physical_memory = memory_status.dwAvailPhys;
    display_information->_page_file_size = memory_status.dwTotalPageFile;
    display_information->_available_page_file_size = memory_status.dwAvailPageFile;
    display_information->_process_virtual_memory = memory_status.dwTotalVirtual;
    display_information->_available_process_virtual_memory = memory_status.dwAvailVirtual;
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

typedef union {
  PN_uint64 long_integer;
}
LONG_INTEGER;

PN_uint64 cpu_time_function (void) {
#ifdef _WIN64
  return __rdtsc();
#else
  LONG_INTEGER long_integer;
  LONG_INTEGER *long_integer_pointer;

  long_integer_pointer = &long_integer;

  __asm {
      mov   ebx,[long_integer_pointer]
      rdtsc
      mov   [ebx + 0], eax
      mov   [ebx + 4], edx
  }

  return long_integer.long_integer;
#endif
}

typedef union {
  struct {
    union {
      struct {
        unsigned char al;
        unsigned char ah;
      };
      unsigned int eax;
    };
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
  };
} CPU_ID_REGISTERS;

typedef struct {
  union {
    struct {
      int maximum_cpu_id_input;
      char cpu_vendor [16];
    };

    CPU_ID_REGISTERS cpu_id_registers_0;
  };

  union {
    CPU_ID_REGISTERS cpu_id_registers_1;

    struct {
      // eax
      union {
        unsigned int eax;
        unsigned int version_information;
        struct {
          unsigned int stepping_id : 4;
          unsigned int model : 4;
          unsigned int family : 4;
          unsigned int processor_type : 2;
          unsigned int reserved_0 : 2;
          unsigned int extended_model_id : 4;
          unsigned int extended_family_id : 8;
          unsigned int reserved_1 : 4;
        };
      };

      // ebx
      union {
        unsigned int ebx;
        struct {
          unsigned int brand_index : 8;
          unsigned int clflush : 8;
          unsigned int maximum_logical_processors : 8;
          unsigned int initial_apic_id : 8;
        };
      };

      // ecx
      union {
        unsigned int ecx;
        struct {
          unsigned int sse3 : 1;
          unsigned int reserved_1_to_2 : 2;
          unsigned int monitor : 1;
          unsigned int ds_cpl : 1;
          unsigned int vmx : 1;
          unsigned int reserved_6 : 1;
          unsigned int est : 1;
          unsigned int tm2 : 1;
          unsigned int reserved_9 : 1;
          unsigned int cnxt_id : 1;
          unsigned int reserved_11_to_12 : 2;
          unsigned int cmpxchg16b : 1;
          unsigned int xtpr_disable : 1;
          unsigned int reserved_15_to_31 : 17;
        };
      };

      // edx
      union {
        unsigned int edx;
        struct {
          unsigned int fpu : 1;
          unsigned int vme : 1;
          unsigned int de : 1;
          unsigned int pse : 1;
          unsigned int tsc : 1;
          unsigned int msr : 1;
          unsigned int pae : 1;
          unsigned int mce : 1;
          unsigned int cx8 : 1;
          unsigned int apic : 1;
          unsigned int reserved_10 : 1;
          unsigned int sep : 1;
          unsigned int mtrr : 1;
          unsigned int pge : 1;
          unsigned int mca : 1;
          unsigned int cmov : 1;
          unsigned int pat : 1;
          unsigned int pse_36 : 1;
          unsigned int psn : 1;
          unsigned int cflush : 1;
          unsigned int reserved_20 : 1;
          unsigned int ds : 1;
          unsigned int acpi : 1;
          unsigned int mmx : 1;
          unsigned int fxsr : 1;
          unsigned int sse : 1;
          unsigned int sse2 : 1;
          unsigned int ss : 1;
          unsigned int htt : 1;
          unsigned int tm : 1;
          unsigned int reserved_30 : 1;
          unsigned int pbe : 1;
        };
      };
    };
  };

  #define MAXIMUM_2 8
  #define MAXIMUM_CHARACTERS (MAXIMUM_2 * sizeof(CPU_ID_REGISTERS))

  union {
    CPU_ID_REGISTERS cpu_id_registers_2;
    unsigned char character_array_2 [MAXIMUM_CHARACTERS];
    CPU_ID_REGISTERS cpu_id_registers_2_array [MAXIMUM_2];
  };

  union {
    CPU_ID_REGISTERS cpu_id_registers_0x80000000;
  };

  union {
    CPU_ID_REGISTERS cpu_id_registers_0x80000001;
  };

  union {
    char cpu_brand_string [sizeof(CPU_ID_REGISTERS) * 3];
    struct {
      CPU_ID_REGISTERS cpu_id_registers_0x80000002;
      CPU_ID_REGISTERS cpu_id_registers_0x80000003;
      CPU_ID_REGISTERS cpu_id_registers_0x80000004;
    };
  };

  union {
    struct {
      unsigned int eax;
      unsigned int ebx;
      union {
        unsigned int ecx;
        struct {
          unsigned int l1_data_cache_line_size : 8;
          unsigned int l1_data_reserved_8_to_15 : 8;
          unsigned int l1_data_associativity : 8;
          unsigned int l1_data_cache_size : 8;
        };
      };
      union {
        unsigned int edx;
        struct {
          unsigned int l1_code_cache_line_size : 8;
          unsigned int l1_code_reserved_8_to_15 : 8;
          unsigned int l1_code_associativity : 8;
          unsigned int l1_code_cache_size : 8;
        };
      };
    };
    CPU_ID_REGISTERS cpu_id_registers_0x80000005;
  };

  union {
    struct {
      unsigned int eax;
      unsigned int ebx;
      union {
        unsigned int ecx;
        struct {
          unsigned int l2_cache_line_size : 8;
          unsigned int l2_reserved_8_to_11 : 4;
          unsigned int l2_associativity : 4;
          unsigned int l2_cache_size : 16;
        };
      };
      unsigned int edx;
    };
    CPU_ID_REGISTERS cpu_id_registers_0x80000006;
  };

  CPU_ID_REGISTERS cpu_id_registers_0x80000008;

  unsigned int cache_line_size;
  unsigned int log_base_2_cache_line_size;
} CPU_ID;

typedef struct {
  CPU_ID_REGISTERS cpu_id_registers_0;
  CPU_ID_REGISTERS cpu_id_registers_1;

  CPU_ID_REGISTERS cpu_id_registers_0x80000000;
  CPU_ID_REGISTERS cpu_id_registers_0x80000001;
  CPU_ID_REGISTERS cpu_id_registers_0x80000002;
  CPU_ID_REGISTERS cpu_id_registers_0x80000003;
  CPU_ID_REGISTERS cpu_id_registers_0x80000004;

  CPU_ID_REGISTERS cpu_id_registers_0x80000006;

  CPU_ID_REGISTERS cpu_id_registers_0x80000008;
} CPU_ID_BINARY_DATA;

void cpu_id_to_cpu_id_binary_data (CPU_ID *cpu_id, CPU_ID_BINARY_DATA *cpu_id_binary_data) {
  cpu_id_binary_data->cpu_id_registers_0 = cpu_id->cpu_id_registers_0;
  cpu_id_binary_data->cpu_id_registers_1 = cpu_id->cpu_id_registers_1;
  cpu_id_binary_data->cpu_id_registers_0x80000000 = cpu_id->cpu_id_registers_0x80000000;
  cpu_id_binary_data->cpu_id_registers_0x80000001 = cpu_id->cpu_id_registers_0x80000001;
  cpu_id_binary_data->cpu_id_registers_0x80000002 = cpu_id->cpu_id_registers_0x80000002;
  cpu_id_binary_data->cpu_id_registers_0x80000003 = cpu_id->cpu_id_registers_0x80000003;
  cpu_id_binary_data->cpu_id_registers_0x80000004 = cpu_id->cpu_id_registers_0x80000004;
  cpu_id_binary_data->cpu_id_registers_0x80000006 = cpu_id->cpu_id_registers_0x80000006;
  cpu_id_binary_data->cpu_id_registers_0x80000008 = cpu_id->cpu_id_registers_0x80000008;
}

void cpu_id_binary_data_to_cpu_id (CPU_ID_BINARY_DATA *cpu_id_binary_data, CPU_ID *cpu_id) {
  memset (cpu_id, 0, sizeof(CPU_ID));

  cpu_id->cpu_id_registers_0 = cpu_id_binary_data->cpu_id_registers_0;
  cpu_id->cpu_id_registers_1 = cpu_id_binary_data->cpu_id_registers_1;
  cpu_id->cpu_id_registers_0x80000000 = cpu_id_binary_data->cpu_id_registers_0x80000000;
  cpu_id->cpu_id_registers_0x80000001 = cpu_id_binary_data->cpu_id_registers_0x80000001;
  cpu_id->cpu_id_registers_0x80000002 = cpu_id_binary_data->cpu_id_registers_0x80000002;
  cpu_id->cpu_id_registers_0x80000003 = cpu_id_binary_data->cpu_id_registers_0x80000003;
  cpu_id->cpu_id_registers_0x80000004 = cpu_id_binary_data->cpu_id_registers_0x80000004;
  cpu_id->cpu_id_registers_0x80000006 = cpu_id_binary_data->cpu_id_registers_0x80000006;
  cpu_id->cpu_id_registers_0x80000008 = cpu_id_binary_data->cpu_id_registers_0x80000008;
}

int cpuid(int input_eax, CPU_ID_REGISTERS *cpu_id_registers) {
  int state;

  state = false;
  __try {
    if (input_eax == 0) {
      // the order of ecx and edx is swapped when saved to make a proper vendor string
#ifdef _WIN64
      __cpuid((int*)cpu_id_registers, input_eax);
      unsigned int tmp = cpu_id_registers->edx;
      cpu_id_registers->edx = cpu_id_registers->ecx;
      cpu_id_registers->ecx = tmp;
#else
      __asm {
          mov   eax, [input_eax]
          mov   edi, [cpu_id_registers]

          cpuid

          mov   [edi + 0], eax
          mov   [edi + 4], ebx
          mov   [edi + 8], edx
          mov   [edi + 12], ecx
      }
#endif
    } else {
#ifdef _WIN64
      __cpuid((int*)cpu_id_registers, input_eax);
#else
      __asm {
          mov   eax, [input_eax]
          mov   edi, [cpu_id_registers]

          cpuid

          mov   [edi + 0], eax
          mov   [edi + 4], ebx
          mov   [edi + 8], ecx
          mov   [edi + 12], edx
      }
#endif
    }

    state = true;
  }
  __except (1) {
    state = false;
  }

  return state;
}

void parse_cpu_id(CPU_ID *cpu_id) {
  printf("CPUID\n");
  printf("  vendor = %s\n", cpu_id->cpu_vendor);
  printf("  brand string %s\n", cpu_id->cpu_brand_string);
  printf("  maximum_cpu_id_input = %u\n", cpu_id->maximum_cpu_id_input);
  printf("  maximum extended information = 0x%X\n", cpu_id->cpu_id_registers_0x80000000.eax);

  printf("  MMX  = %u\n", cpu_id->mmx);
  printf("  SSE  = %u\n", cpu_id->sse);
  printf("  SSE2 = %u\n", cpu_id->sse2);
  printf("  SSE3 = %u\n", cpu_id->sse3);

  printf("  EST  = %u\n", cpu_id->est);

  if (cpu_id->maximum_cpu_id_input >= 1) {
    printf("  version_information\n");
    printf("    stepping_id %u\n", cpu_id->stepping_id);
    printf("    model %u\n", cpu_id->model);
    printf("    family %u\n", cpu_id->family);
    printf("    processor_type %u\n", cpu_id->processor_type);
    printf("    extended_model_id %u\n", cpu_id->extended_model_id);
    printf("    extended_family_id %u\n", cpu_id->extended_family_id);

    printf("    brand_index %u\n", cpu_id->brand_index);
    printf("    clflush %u\n", cpu_id->clflush);
    printf("    maximum_logical_processors %u\n", cpu_id->maximum_logical_processors);
    printf("    initial_apic_id %u\n", cpu_id->initial_apic_id);

//    printf("  cache_line_size %u\n", cpu_id->cache_line_size);
//    printf("  log_base_2_cache_line_size %u\n", cpu_id->log_base_2_cache_line_size);
  }

  if (cpu_id->cpu_id_registers_0x80000000.eax >= 0x80000005) {
    printf("  l1_data_cache_line_size %d\n", cpu_id->l1_data_cache_line_size);
    printf("  l1_data_associativity %d\n", cpu_id->l1_data_associativity);
    printf("  l1_data_cache_size %dK\n", cpu_id->l1_data_cache_size);

    printf("  l1_code_cache_line_size %d\n", cpu_id->l1_code_cache_line_size);
    printf("  l1_code_associativity %d\n", cpu_id->l1_code_associativity);
    printf("  l1_code_cache_size %dK\n", cpu_id->l1_code_cache_size);
  }

  if (cpu_id->cpu_id_registers_0x80000000.eax >= 0x80000006) {
    printf("  l2_cache_line_size %d\n", cpu_id->l2_cache_line_size);
    printf("  l2_associativity %d\n", cpu_id->l2_associativity);
    printf("  l2_cache_size %dK\n", cpu_id->l2_cache_size);
  }
}

int initialize_cpu_id(CPU_ID *cpu_id) {
  int debug = false;
  memset(cpu_id, 0, sizeof(CPU_ID));

  if (cpuid(0, &cpu_id->cpu_id_registers_0)) {
    if (cpu_id->maximum_cpu_id_input >= 1) {
      cpuid(1, &cpu_id->cpu_id_registers_1);
    }
    if (cpu_id->maximum_cpu_id_input >= 2) {
      unsigned int index;

      cpuid(2, &cpu_id->cpu_id_registers_2);
      if (debug) {
        printf("  al = %u\n", cpu_id->cpu_id_registers_2.al);
      }

      for (index = 1; index < cpu_id->cpu_id_registers_2.al && index < MAXIMUM_2; index++) {
        cpuid(2, &cpu_id->cpu_id_registers_2_array [index]);
      }

      for (index = 1; index < MAXIMUM_CHARACTERS; index++) {
        if (cpu_id->character_array_2 [index]) {
          if (debug) {
            printf("  cache/TLB byte = %X\n", cpu_id->character_array_2 [index]);
          }
          switch (cpu_id->character_array_2 [index]) {
          case 0x0A:
          case 0x0C:
            cpu_id->cache_line_size = 32;
            cpu_id->log_base_2_cache_line_size = 5;
            break;

          case 0x2C:
          case 0x60:
          case 0x66:
          case 0x67:
          case 0x68:
            cpu_id->cache_line_size = 64;
            cpu_id->log_base_2_cache_line_size = 6;
            break;
          }
        }
      }
    }

    cpuid(0x80000000, &cpu_id->cpu_id_registers_0x80000000);

    if (cpu_id->cpu_id_registers_0x80000000.eax >= 0x80000001) {
      cpuid(0x80000001, &cpu_id->cpu_id_registers_0x80000001);
    }

    if (cpu_id->cpu_id_registers_0x80000000.eax >= 0x80000004) {
      cpuid(0x80000002, &cpu_id->cpu_id_registers_0x80000002);
      cpuid(0x80000003, &cpu_id->cpu_id_registers_0x80000003);
      cpuid(0x80000004, &cpu_id->cpu_id_registers_0x80000004);
    }

    if (cpu_id->cpu_id_registers_0x80000000.eax >= 0x80000005) {
      cpuid(0x80000005, &cpu_id->cpu_id_registers_0x80000005);
    }

    if (cpu_id->cpu_id_registers_0x80000000.eax >= 0x80000006) {
      cpuid(0x80000006, &cpu_id->cpu_id_registers_0x80000006);
    }

    if (cpu_id->cpu_id_registers_0x80000000.eax >= 0x80000008) {
      cpuid(0x80000008, &cpu_id->cpu_id_registers_0x80000008);
    }

    return true;
  }

  return false;
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
    input_buffer = NULL;
    output_buffer = processor_power_information_array;
    input_buffer_size = 0;
    output_buffer_size = sizeof(PROCESSOR_POWER_INFORMATION) * MAXIMUM_PROCESSORS;
    if (CallNtPowerInformationFunction(information_level, input_buffer, input_buffer_size, output_buffer, output_buffer_size) == 0) {
      processor_power_information = processor_power_information_array;
      for (i = 0; i < MAXIMUM_PROCESSORS; i++) {
        if (processor_power_information->Number == processor_number) {
          PN_uint64 value;

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
  if (glpi == NULL) {
    windisplay_cat.info()
      << "GetLogicalProcessorInformation is not supported.\n";
    return;
  }

  // Allocate a buffer to hold the result of the
  // GetLogicalProcessorInformation call.
  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
  DWORD buffer_length = 0;
  DWORD rc = glpi(buffer, &buffer_length);
  while (!rc) {
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
      if (buffer != NULL) {
        PANDA_FREE_ARRAY(buffer);
      }

      buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)PANDA_MALLOC_ARRAY(buffer_length);
      nassertv(buffer != NULL);
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
      num_logical_cpus += count_bits_in_word((PN_uint64)(ptr->ProcessorMask));
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


////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsPipe::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WinGraphicsPipe::
WinGraphicsPipe() {
  char string [512];

  _supported_types = OT_window | OT_fullscreen_window;

  // these fns arent defined on win95, so get dynamic ptrs to them
  // to avoid ugly DLL loader failures on w95
  _pfnTrackMouseEvent = NULL;

  _hUser32 = (HINSTANCE)LoadLibrary("user32.dll");
  if (_hUser32 != NULL) {
    _pfnTrackMouseEvent =
      (PFN_TRACKMOUSEEVENT)GetProcAddress(_hUser32, "TrackMouseEvent");
  }

#ifdef HAVE_DX9
  // Use D3D to get display info.  This is disabled by default as it is slow.
  if (request_dxdisplay_information) {
    DisplaySearchParameters display_search_parameters_dx9;
    int dx9_display_information (DisplaySearchParameters &display_search_parameters_dx9, DisplayInformation *display_information);
    dx9_display_information(display_search_parameters_dx9, _display_information);
  } else
#endif
  {
    // Use the Win32 API to query the available display modes.
    pvector<DisplayMode> display_modes;
    DEVMODE dm = {0};
    dm.dmSize = sizeof(dm);
    for (int i = 0; EnumDisplaySettings(NULL, i, &dm) != 0; ++i) {
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
      sprintf(string, "OS version: %d.%d.%d.%d\n", version_info.dwMajorVersion, version_info.dwMinorVersion, version_info.dwPlatformId, version_info.dwBuildNumber);
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

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsPipe::lookup_cpu_data
//       Access: Public, Virtual
//  Description: Looks up the detailed CPU information and stores it
//               in _display_information, if supported by the OS.
//               This may take a second or two.
////////////////////////////////////////////////////////////////////
void WinGraphicsPipe::
lookup_cpu_data() {
  char string [512];

  // set callback for memory function
  _display_information->_get_memory_information_function = get_memory_information;

  // set callback for cpu time function
  _display_information->_cpu_time_function = cpu_time_function;

  // determine CPU frequency
  PN_uint64 time;
  PN_uint64 end_time;
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
        time = cpu_time_function();
        end.QuadPart = counter.QuadPart + frequency.QuadPart;
        while (QueryPerformanceCounter (&counter) && counter.QuadPart < end.QuadPart) {

        }
        end_time = cpu_time_function();

        _display_information->_cpu_frequency = end_time - time;
      }
    }
  }

  SetThreadPriority(thread, priority);
  sprintf(string, "QueryPerformanceFrequency: %I64d\n", frequency.QuadPart);
  windisplay_cat.info() << string;
  sprintf(string, "CPU frequency: %I64d\n", _display_information->_cpu_frequency);
  windisplay_cat.info() << string;


  // CPUID
  CPU_ID cpu_id;

  windisplay_cat.info() << "start CPU ID\n";

  if (initialize_cpu_id(&cpu_id)) {
    CPU_ID_BINARY_DATA *cpu_id_binary_data;

    cpu_id_binary_data = new (CPU_ID_BINARY_DATA);
    if (cpu_id_binary_data) {
      cpu_id_to_cpu_id_binary_data(&cpu_id, cpu_id_binary_data);
      _display_information->_cpu_id_size = sizeof(CPU_ID_BINARY_DATA) / sizeof(unsigned int);
      _display_information->_cpu_id_data = (unsigned int *) cpu_id_binary_data;

      _display_information->_cpu_vendor_string = strdup(cpu_id.cpu_vendor);
      _display_information->_cpu_brand_string = strdup(cpu_id.cpu_brand_string);
      _display_information->_cpu_version_information = cpu_id.version_information;
      _display_information->_cpu_brand_index = cpu_id.brand_index;

      if (windisplay_cat.is_debug()) {
        windisplay_cat.debug()
          << hex << _display_information->_cpu_id_version << dec << "|";

        int index;
        for (index = 0; index < _display_information->_cpu_id_size; ++index) {
          unsigned int data;
          data = _display_information->_cpu_id_data[index];

          windisplay_cat.debug(false)
            << hex << data << dec;
          if (index < _display_information->_cpu_id_size - 1) {
            windisplay_cat.debug(false)
              << "|";
          }
        }
        windisplay_cat.debug(false)
          << "\n";
      }
    }

    if (windisplay_cat.is_debug()) {
      parse_cpu_id(&cpu_id);
    }
  }

  windisplay_cat.info() << "end CPU ID\n";

  // Number of CPU's
  count_number_of_cpus(_display_information);
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
WinGraphicsPipe::
~WinGraphicsPipe() {
  if (_hUser32 != NULL) {
    FreeLibrary(_hUser32);
    _hUser32 = NULL;
  }
}

bool MyGetProcAddr(HINSTANCE hDLL, FARPROC *pFn, const char *szExportedFnName) {
  *pFn = (FARPROC) GetProcAddress(hDLL, szExportedFnName);
  if (*pFn == NULL) {
    windisplay_cat.error() << "GetProcAddr failed for " << szExportedFnName << ", error=" << GetLastError() <<endl;
    return false;
  }
  return true;
}

bool MyLoadLib(HINSTANCE &hDLL, const char *DLLname) {
  hDLL = LoadLibrary(DLLname);
  if(hDLL == NULL) {
    windisplay_cat.error() << "LoadLibrary failed for " << DLLname << ", error=" << GetLastError() <<endl;
    return false;
  }
  return true;
}
