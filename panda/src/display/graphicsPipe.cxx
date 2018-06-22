/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsPipe.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "graphicsPipe.h"
#include "graphicsWindow.h"
#include "graphicsBuffer.h"
#include "config_display.h"
#include "mutexHolder.h"
#include "displayInformation.h"

#ifdef IS_LINUX
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

#if defined(IS_OSX) || defined(IS_FREEBSD)
#include <sys/sysctl.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

// CPUID is only available on i386 and x86-64 architectures.
#if defined(__i386) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)

#if defined(__GNUC__) && !defined(__APPLE__)
// GCC and Clang offer a useful cpuid.h header.
#include <cpuid.h>
#endif

#ifdef _MSC_VER
// MSVC has a __cpuid intrinsic.
#include <intrin.h>
#endif

union cpuid_info {
  char str[16];
  struct {
    uint32_t eax, ebx, ecx, edx;
  };
};

/**
 * Returns the highest cpuid leaf that is supported by the CPU.
 */
static inline uint32_t get_cpuid_max(uint32_t leaf) {
#if defined(__GNUC__) && !defined(__APPLE__)
  return __get_cpuid_max(leaf, nullptr);
#elif defined(_MSC_VER)
  uint32_t p[4] = {0};
  __cpuid((int *)p, leaf);
  return p[0];
#else
  unsigned int eax = 0;
  __asm__ ("cpuid\n\t"
           : "=a" (eax)
           : "0" (leaf));
  return eax;
#endif
}

/**
 * Gets cpuid info for the given leaf.
 */
static inline void get_cpuid(uint32_t leaf, cpuid_info &info) {
#if defined(__GNUC__) && !defined(__APPLE__)
  __cpuid(leaf, info.eax, info.ebx, info.ecx, info.edx);
#elif defined(_MSC_VER)
  __cpuid((int *)info.str, leaf);
#else
  __asm__ ("cpuid\n\t"
           : "=a" (info.eax), "=b" (info.ebx), "=c" (info.ecx), "=d" (info.edx)
           : "0" (leaf));
#endif
}
#endif

#ifdef IS_LINUX
/**
 * Updates the current memory usage statistics in the DisplayInformation.
 */
static void update_memory_info(DisplayInformation *info) {
  struct sysinfo meminfo;
  if (sysinfo(&meminfo) == 0) {
    info->_physical_memory = meminfo.totalram;
    info->_available_physical_memory = meminfo.freeram;
    info->_page_file_size = meminfo.totalswap;
    info->_available_page_file_size = meminfo.freeswap;
  }
}
#endif

TypeHandle GraphicsPipe::_type_handle;

/**
 *
 */
GraphicsPipe::
GraphicsPipe() :
  _lock("GraphicsPipe")
{
  // Initially, we assume the GraphicsPipe is valid.  A derived class should
  // set this to false if it determines otherwise.
  _is_valid = true;

  // A derived class must indicate the kinds of GraphicsOutput objects it can
  // create.
  _supported_types = 0;

  _display_width = 0;
  _display_height = 0;

  _display_information = new DisplayInformation();

#if defined(__i386) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
  cpuid_info info;
  const uint32_t max_cpuid = get_cpuid_max(0);
  const uint32_t max_extended = get_cpuid_max(0x80000000);

  if (max_cpuid >= 1) {
    get_cpuid(0, info);
    std::swap(info.ecx, info.edx);
    _display_information->_cpu_vendor_string = std::string(info.str + 4, 12);

    get_cpuid(1, info);
    _display_information->_cpu_version_information = info.eax;
    _display_information->_cpu_brand_index = info.ebx & 0xff;
  }

  if (max_extended >= 0x80000004) {
    char brand[49];
    get_cpuid(0x80000002, info);
    memcpy(brand, info.str, 16);
    get_cpuid(0x80000003, info);
    memcpy(brand + 16, info.str, 16);
    get_cpuid(0x80000004, info);
    memcpy(brand + 32, info.str, 16);
    brand[48] = 0;
    _display_information->_cpu_brand_string = brand;
  }
#endif

#if defined(IS_OSX)
  // macOS exposes a lot of useful information through sysctl.
  size_t len = sizeof(uint64_t);
  sysctlbyname("hw.memsize", &_display_information->_physical_memory, &len, nullptr, 0);
  len = sizeof(uint64_t);
  sysctlbyname("hw.cpufrequency", &_display_information->_cpu_frequency, &len, nullptr, 0);
  len = sizeof(uint64_t);
  sysctlbyname("hw.cpufrequency", &_display_information->_current_cpu_frequency, &len, nullptr, 0);
  len = sizeof(uint64_t);
  sysctlbyname("hw.cpufrequency_max", &_display_information->_maximum_cpu_frequency, &len, nullptr, 0);
  len = sizeof(int);
  sysctlbyname("hw.physicalcpu", &_display_information->_num_cpu_cores, &len, nullptr, 0);
  len = sizeof(int);
  sysctlbyname("hw.logicalcpu", &_display_information->_num_logical_cpus, &len, nullptr, 0);

#elif defined(IS_LINUX)
  _display_information->_get_memory_information_function = &update_memory_info;
  update_memory_info(_display_information);

#elif defined(IS_FREEBSD)
  size_t len = sizeof(uint64_t);
  sysctlbyname("hw.physmem", &_display_information->_physical_memory, &len, nullptr, 0);
  len = sizeof(uint64_t);
  sysctlbyname("vm.swap_total", &_display_information->_page_file_size, &len, nullptr, 0);

#elif defined(_WIN32)
  MEMORYSTATUSEX status;
  status.dwLength = sizeof(MEMORYSTATUSEX);
  if (GlobalMemoryStatusEx(&status)) {
    _display_information->_physical_memory = status.ullTotalPhys;
    _display_information->_available_physical_memory = status.ullAvailPhys;
    _display_information->_page_file_size = status.ullTotalPageFile;
    _display_information->_available_page_file_size = status.ullAvailPageFile;
    _display_information->_process_virtual_memory = status.ullTotalVirtual;
    _display_information->_available_process_virtual_memory = status.ullAvailVirtual;
    _display_information->_memory_load = status.dwMemoryLoad;
  }
#endif

#if defined(IS_LINUX) || defined(IS_FREEBSD)
  long nproc = sysconf(_SC_NPROCESSORS_CONF);
  if (nproc > 0) {
    _display_information->_num_logical_cpus = nproc;
  }
#endif
}

/**
 *
 */
GraphicsPipe::
~GraphicsPipe() {
  delete _display_information;
}

/**
 * Returns an indication of the thread in which this GraphicsPipe requires its
 * window processing to be performed: typically either the app thread (e.g.
 * X) or the draw thread (Windows).
 */
GraphicsPipe::PreferredWindowThread
GraphicsPipe::get_preferred_window_thread() const {
  return PWT_draw;
}

/**
 * This is called when make_output() is used to create a
 * CallbackGraphicsWindow.  If the GraphicsPipe can construct a GSG that's not
 * associated with any particular window object, do so now, assuming the
 * correct graphics context has been set up externally.
 */
PT(GraphicsStateGuardian) GraphicsPipe::
make_callback_gsg(GraphicsEngine *engine) {
  return nullptr;
}

/**
 * Creates a new device for the pipe.  Only DirectX uses this device, for
 * other api's it is NULL.
 */
PT(GraphicsDevice) GraphicsPipe::
make_device(void *scrn) {
  display_cat.error()
    << "make_device() unimplemented by " << get_type() << "\n";
  return nullptr;
}

/**
 * This will be called in the draw thread (the same thread in which the GSG
 * was created via make_gsg, above) to close the indicated GSG and free its
 * associated graphics objects just before it is destructed.  This method
 * exists to provide a hook for the graphics pipe to do any necessary cleanup,
 * if any.
 */
void GraphicsPipe::
close_gsg(GraphicsStateGuardian *gsg) {
  if (gsg != nullptr) {
    gsg->close_gsg();
  }
}

/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) GraphicsPipe::
make_output(const std::string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {
  display_cat.error()
    << get_type() << " cannot create buffers or windows.\n";
  return nullptr;
}

/**
 * Gets the pipe's DisplayInformation.
 */
DisplayInformation *GraphicsPipe::
get_display_information() {
  return _display_information;
}

/**
 * Looks up the detailed CPU information and stores it in
 * _display_information, if supported by the OS. This may take a second or
 * two.
 */
void GraphicsPipe::
lookup_cpu_data() {
}
