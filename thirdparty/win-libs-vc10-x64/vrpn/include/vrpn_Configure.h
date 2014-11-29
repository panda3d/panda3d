// -*- c++ -*-

#ifndef VRPN_USING_CMAKE
// Only using this configuration file if not using CMake.
// An #else follows at the bottom of the file.

#ifndef	VRPN_CONFIGURE_H

//--------------------------------------------------------------
/* IMPORTANT NOTE: If you are using CMake to build VRPN, this
   file DOES NOT affect your build.  vrpn_Configure.h.cmake_in
   is processed automatically into a vrpn_Configure.h file
   placed in your build directory using the choices you make in
   CMake.  Until all modules are fully configured using CMake,
   you may have to edit the paths that are listed near the
   bottom of the first section of that file, then re-run CMake
   to regenerate vrpn_Configure.h. */
//--------------------------------------------------------------

//--------------------------------------------------------------
/* This file contains configuration options for VRPN.  The first
   section has definition lines that can be commented in or out
   at build time.  The second session has automaticly-generated
   directives and should not be edited. */
//--------------------------------------------------------------

//--------------------------------------------------------//
// EDIT BELOW THIS LINE FOR NORMAL CONFIGURATION SETTING. //
//--------------------------------------------------------//

//-----------------------
// Default port to listen on for a server.  It used to be 4500
// up through version 6.03, but then all sorts of VPNs started
// using this, as did Microsoft.  Port 3883 was assigned to VRPN
// by the Internet Assigned Numbers Authority (IANA) October, 2003.
// Change this to make a location-specific default if you like.
// The parentheses are to keep it from being expanded into something
// unexpected if the code has a dot after it.
#define	vrpn_DEFAULT_LISTEN_PORT_NO (3883)

//-----------------------
// Instructs VRPN to expose the vrpn_gettimeofday() function also
// as gettimeofday() so that external programs can use it.  This
// has no effect on any system that already has gettimeofday()
// defined, and is put here for Windows.  This function should
// not really be implemented within VRPN, but it was expedient to
// include it when porting applications to Windows.  Turn this
// off if you have another implementation, or if you want to call
// vrpn_gettimeofday() directly.
#define	VRPN_EXPORT_GETTIMEOFDAY

//-----------------------
// Tells VRPN to compile with support for the Message-Passing
// Interface (MPI) library.  There is a configuration section below
// that has a library path for the MPI library to link against.
// You will need to add the path to mpi.h and other needed files
// into your Visual Studio Tools/Options/Projects and Solutions/
// C++ Directories include path.  The original implementation is
// done with MPICH2, but an attempt has been made to use only
// MPI version 1 basic functions.
//#define	vrpn_USE_MPI

//-----------------------
// Instructs VRPN to use phantom library to construct a unified
// server, using phantom as a common device, and phantom
// configuration in .cfg file.
//#define	VRPN_USE_PHANTOM_SERVER

//------------------------
// Instructs vrpn to use SensAble's HDAPI rather than GHOST library.
// Only used in conjuntion with VRPN_USE_PHANTOM_SERVER.
// PLEASE SPECIFY PATH TO HDAPI IN NEXT SECTION IF YOU USE THIS.
// Also, you need to go to the vrpn_phantom and vrpn_server projects
// and remove the GHOST include directories from the include paths.
// Yes, HDAPI fails if it even has them in the path (as so many other
// things also fail).  At least we're rid of them now.  When you
// uncomment it (to use GHOST), add the following to the include
// directories for the vrpn_phantom project: $(SYSTEMDRIVE)\Program Files\SensAble\GHOST\v4.0\include,$(SYSTEMDRIVE)\Program Files\SensAble\GHOST\v4.0\external\stl,
// On SGI, you need to uncomment the GHOST lines in the Makefile in
// the server_src directory.
#define VRPN_USE_HDAPI

//------------------------
// Instructs vrpn to use Ghost 3.1 instead of Ghost 3.4.
// Only used in conjuntion with VRPN_USE_PHANTOM_SERVER.
// PLEASE SPECIFY PATH TO GHOSTLIB IN NEXT SECTION IF YOU USE THIS
// (This is expected to be used on systems where Ghost 4.0 is not
// available, such as the SGI platform.  If you are using this on
// a Windows PC with Visual Studio, you will need to alter
// server_src/vrpn_phantom.dsp to reference the Ghost 3.1 include
// paths.)
//#define VRPN_USE_GHOST_31

//-----------------------
// Instructs VRPN to use the high-performance timer code on
// Windows, rather than the default clock which has an infrequent
// update.  At one point in the past, an implementation of this
// would only work correctly on some flavors of Windows and with
// some types of CPUs.
// There are actually two implementations
// of the faster windows clock.  The original one, made by Hans
// Weber, checks the clock rate to see how fast the performance
// clock runs (it takes a second to do this when the program
// first calls vrpn_gettimeofday()).  The second version by Haris
// Fretzagias relies on the timing supplied by Windows.  To use
// the second version, also define VRPN_WINDOWS_CLOCK_V2.
#define	VRPN_UNSAFE_WINDOWS_CLOCK
#define	VRPN_WINDOWS_CLOCK_V2

//-----------------------
// Instructs VRPN library and server to include code that uses
// the DirectX SDK.  If you set this, you may to edit the
// system configuration section below to point at the correct version
// of DirectX.  WARNING: With the August 2006 DirectX SDK, you
// cannot link against the debug library in Visual Studio 6.0,
// only the release.  Hopefully, Visual Studio.NET doesn't have
// this problem.
// IMPORTANT!  If you define this, you need to edit the Tools/Options
// menu:
//    For Visual studio 6, use the Directories tab, and add the
// include and lib paths to the TOP of the lists for all configurations.
//    For Visual studio .NET, add to the top of the Projects and Solutions/
//  VC++ Directories entry.
//    This will let the code find the right version when it compiles.
//#define	VRPN_USE_DIRECTINPUT
//#define   VRPN_USE_WINDOWS_XINPUT

//-----------------------
// Instructs VRPN library and server to include code that uses
// the DirectShow SDK.  If you set this, you may to edit the
// system configuration section below to point at the correct version
// of the Platform SDK.  WARNING: With the August 2006 DirectX SDK, you
// cannot link against the debug library in Visual Studio 6.0,
// only the release.  Visual Studio.NET doesn't have this problem.
//#define	VRPN_USE_DIRECTSHOW

//-----------------------
// Instructs the VRPN server to create an entry for the Adrienne
// time-code generator.  This is a device that produces time values
// from an analog video stream so that events in the virtual world
// can be synchronized with events on a movie.  The Adrienne folder
// should be located at the same level as the VRPN folder for the
// code to find it.
//#define	VRPN_INCLUDE_TIMECODE_SERVER

//-----------------------
// Compiles the InterSense Tracker using the
// InterSense Interface Libraries SDK (tested for version
// 3.45) on windows.  This should work with all Intersense trackers,
// both the USB and the serial port versions.  The files isense.h,
// types.h and isense.c should be put in a directory called 'isense'
// at the same level as the vrpn directory.  The isense.dll should
// be put either in Windows/system32 or in the location where the
// executable lives or somewhere on the path.
//#define VRPN_INCLUDE_INTERSENSE


//-----------------------
// Instructs VRPN library and server to include code that uses
// the National Instruments Nidaq libary to control analog outputa.
// Later in this file, we also instruct the compiler to link with
// the National Instruments libraries if this is defined.  Either or
// both of these can be defined, depending on which library you
// need to use.
//#define	VRPN_USE_NATIONAL_INSTRUMENTS
//#define	VRPN_USE_NATIONAL_INSTRUMENTS_MX

//-----------------------
// Instructs VRPN library and server to include code that uses
// the US Digital SEI/A2 libary to control analog inputs from the
// A2 absolute encoder.
// Later in this file, we also instruct the compiler to link with
// the US Digital library if this is defined.  You also need to
// define VRPN_USE_NATIONAL_INSTRUMENTS_MX above if you want to
// use this.
//#define	VRPN_USE_USDIGITAL

//-----------------------
// Instructs VRPN to use the default room space transforms for
// the Desktop Phantom as used in the nanoManipulator application
// rather than the default world-origin with identity rotation.
// Please don't anyone new use the room space transforms built
// into VRPN -- they are a hack pulled forward from Trackerlib.
#define	DESKTOP_PHANTOM_DEFAULTS

//------------------------
// Instructs VRPN to use microscribe3D library to construct a unified
// server
//#define VRPN_USE_MICROSCRIBE

//------------------------
// Compiles the VRPN libary with the PhaseSpace Tracker using the
// PhaseSpace OWL API on Linux and Windows.
//
// In Linux:
// The PhaseSpace header files (owl.h, etc) and libraries (libowlsock)
// should be placed in the phasespace directory at the same level as
// the vrpn folder.  Also, PHASESPACE needs to be uncommented in the
// server_src/Makefile so that the libraries are properly linked.
// libowlsock.so will need to be present in the directory of the
// final executable or in the default library path such as /usr/lib
//
// In Windows:
// The PhaseSpace header files (owl.h, etc) should be placed in the
// phasespace directory at the same level as the vrpn folder.
// libowlsock.lib will need to be located there as well.
// libowlsock.dll will need to be in the path or with the executable
// at run time.  Edit the path below to say where the .lib file
// can be found.
//
//#define VRPN_INCLUDE_PHASESPACE

//-----------------------
// Instructs VRPN to use a DLL interface on Windows systems.
// When using this, link with VRPNDLL.LIB (and VRPN.DLL) rather
// than VRPN.LIB in user code.  This is experimental and is
// under development to enable C# and other languages to pull in
// VRPN.  This is only needed when trying to link VRPN with
// languages other than C++ (and not even for Java).  If you don't
// have a good reason to, don't define it.
// Not implemented for .so-based Unix systems.
//#define VRPN_USE_SHARED_LIBRARY

//------------------------
// Instructs VRPN to use GPM Linux interface mouse interface.
// WARNING: If you define this, then you must also edit the server_src
// Makefile to include "-lgpm" into the SYSLIBS definition line for the
// architecture you use this on.  We had to change this because not all
// Linux releases included this library.
//#define VRPN_USE_GPM_MOUSE

//------------------------
// Instructs VRPN to use the GLI Interactive LLC MotionNode library to
// interface VRPN to their tracker.  If you do this, you must edit
// the include paths in the vrpn and vrpndll libraries to point to the
// correct locations and the lib path in vrpn_server and any other
// applications you build (including custom ones) to point to the right
// location.  You also have to have Boost (www.boost.org) installed and
// have pointed the vrpn and vrpndll project include paths to it and
// the vrpndll and vrpn_server lib paths to it.
// WARNING: This code does not compile under visual studio 6.0.
//#define VRPN_USE_MOTIONNODE

//------------------------
// Instructs VRPN to compile code for the Nintendo Wii Remote controller,
// getting access to it through the Wiiuse library in Windows and Linux.
// Note that this requires installing a bunch of other stuff, including
// a Windows driver developer kit to access the HID devices, and that some bluetooth
// stacks (like the one in Windows XP) cause people trouble -- there is a
// specific one needed for Linux and some options for Windows.  See the
// README file in the WiiUse library for more info.  Also note that the
// WiiUse library is GPL, which is more restrictive than the VRPN public-
// domain license, so check out its license file before building this driver
// into your code.  The original WiiUse library was abandoned and a new
// fork by Ryan Pavlik is available at https://github.com/rpavlik/wiiuse.
// To get the WiiUse library to compile on Visual Studio 2005 (apparently
// not for VS 2008), you need to add the include path
// to the driver developer kit (C:\WINDDK\3790.1830\inc\wxp) and the
// library path to hid.lib (C:\WINDDK\3790.1830\lib\wxp\i386) to the
// include and library directories in Visual Studio.
// Also, edit the configuration below to point to the WiiUse include
// file and library.
// Note that the wiiuse.dll needs to be in the path when running a server
// that uses WiiUse in Windows.
//#define VRPN_USE_WIIUSE

// Instructs VRPN to compile code to handle Hillcrest Labs' Freespace
// devices such as the Loop, and FRCM.  You will also need the libfreespace
// library which is available at http://libfreespace.hillcrestlabs.com/content/download.
// There are prebuilt binaries for Windows, and source available that should work
// on Windows, Linux or OS X.  You will need to make sure the header files
// and library are accessible to the compiler.  libfreespace is released under
// the LGPL and we (Hillcrest Labs) view static and dynamic linking as the same.
// We (Hillcrest Labs) do not require code linked to libfreespace (statically or
// dynamically) to be released under any particular license.
//#define VRPN_USE_FREESPACE

//------------------------
// Instructs VRPN to include code for the Novint Falcon haptic device.
// Access is provided through the libnifalcon library library on Windows,
// MacOSX and Linux. This may require additional libraries for programming
// USB devices. Please consult the corresponding homepages.
//#define VRPN_USE_LIBNIFALCON

//------------------------
// Instructs VRPN to compile code to use Trivisio's Colibri inertial
// tracker.  You will also need the SDK, which is available at 
// http://www.trivisio.com/products/motiontracking/colibri#download
// (tested on Windows).  VRPN_TRIVISIOCOLIBRI_H and VRPN_TRIVISIOCOLIBRI_LIB_PATH
// below point to the default installation locations on Windows.  Edit them 
// if installed elsewhere.  Note that Trivisio.dll and pthreadVC2.dll need to be in 
// the path when running the server on Windows
//#define VRPN_USE_TRIVISIOCOLIBRI

//------------------------
// Instructs VRPN to attempt to use HID.  If you don't have libusb installed
// on Linux, you'll want to turn this off so that it doesn't fail to compile.
// This should work fine on Windows, so we define it by default there.
// For the Mac, let CMake configure this; the built-in Makefile doesn't know
// how to compile with local HIDAPI, which we now need.
// For Linux, you need to have HIDAPI (either local or otherwise) for this
// to work, so this definition is not in by default there.
#if defined(_WIN32)
#if !defined(__MINGW__)
#define VRPN_USE_HID
#endif
#endif

//------------------------
// Instructs VRPN to link in the source code to a local version of
// hidapi to access HID devices.  The source code for this project
// is included as a git submodule under submodule/hidapi.  To pull
// this down if it is not present, use the commands:
// 'git submodule init; git submodule update' from the vrpn directory.
// If you have a system hidapi and you prefer to use it, then do not
// define this here.  Otherwise, define it so that VRPN will be able
// to access HID devices.
// Note that on Linux you will also need to have the libusb-1.0-0-dev
// package installed so that we can compile the code.  You
// will also need to uncommment the SYSLIBS line for HID in the
// server_src/Makefile for this to link.
// On the Mac, this needs to be configured via CMake; the standard
// Makefile doesn't know how to handle HIDAPI.
#if !defined(__MINGW__) && !defined(__APPLE__)
#define VRPN_USE_LOCAL_HIDAPI
#endif

//------------------------
// Instructs VRPN to attempt to use LibUSB-1.0. This will compile and
// link servers that use USB directly (as opposed to those that use it
// through the HID interface).
// See http://libusb.sourceforge.net for more on LibUSB-1.0.
// Note that on Linux you will also need to have the libusb-1.0-0-dev
// package installed so that we can compile the code.  You
// will also need to uncommment the SYSLIBS line for HID in the
// server_src/Makefile for this to link.
// Note that to compile on Windows you will need to have downloaded and installed
// the libusb.h file and libusb-1.0.lib files; the default location for
// the library is C:Program Files\libusb-1.0 and for the include file
// is in C:Program Files\libusb-1.0\libusb.  To open a device on Windows, you
// will need to have installed a driver that lets LibUSB open the
// device.  Generic HID devices and devices that use a WinUSB driver
// should work without adding a driver.  If you need to add a driver,
// consider using the libUSB Zadig.exe program; do not do this for a
// HID device or a device that has another driver, as it can prevent the
// device from operating except through LibUSB.
// Note that on Linux you will also need to have the libusb-1.0-0-dev
// package installed so that we can compile the code.  
//#define VRPN_USE_LIBUSB_1_0

// Instructs VRPN to compile code to handle JSON network messages.
// This requires jsoncpp.
// JSON Network (UDP) mesages are used by the vrpn widgets for Android,
//#define VRPN_USE_JSONNET

//------------------------
// Instructs VRPN to compile code to use the Arrington Research 
// ViewPoint EyeTracker.  You will also need to set VRPN_VIEWPOINT_H
// and VRPN_VIEWPOINT_LIB_PATH below to point to the correct location 
// on your system.  Note that the VRPN server and ViewPoint calibration
// software must use the same copy of the VPX_InterApp.dll
//#define VRPN_USE_VIEWPOINT

//------------------------------------------------------------------//
// SYSTEM CONFIGURATION SECTION                                     //
// EDIT THESE DEFINITIONS TO POINT TO OPTIONAL LIBRARIES.  THEY ARE //
// USED BELOW TO LOCATE LIBRARIES AND INCLUDE FILES.                //
//------------------------------------------------------------------//

#define VRPN_SYSTEMDRIVE "C:"

#define VRPN_PHASESPACE_LIB_PATH "../../phasespace/"

#define VRPN_WIIUSE_H "E:/borland/lib/wiiuse_v0.12_win/wiiuse.h"
#define VRPN_WIIUSE_LIB_PATH "E:/borland/lib/wiiuse_v0.12_win"

#if defined(VRPNDLL_EXPORTS) && !defined(VRPN_USE_SHARED_LIBRARY)
  #define VRPN_FREESPACE_LIB_PATH "../libfreespace/lib"
#else
  #define VRPN_FREESPACE_LIB_PATH "../../libfreespace/lib"
#endif

#define VRPN_TRIVISIOCOLIBRI_H          "C:/Program Files/Trivisio/Colibri/include/TrivisioColibri.h"
#define VRPN_TRIVISIOCOLIBRI_LIB_PATH   "C:/Program Files/Trivisio/Colibri/lib/"

#define VRPN_VIEWPOINT_H				"E:/borland/lib/ViewPoint 2.8.6.21/SDK/vpx.h"
#define VRPN_VIEWPOINT_LIB_PATH			"E:/borland/lib/ViewPoint 2.8.6.21/SDK/"

#ifdef linux
#define VRPN_HDAPI_PATH         "/usr/lib64"
#else
#define VRPN_HDAPI_PATH         VRPN_SYSTEMDRIVE "/Program Files/SensAble/3DTouch/lib/"
#endif
#define VRPN_HDAPI_UTIL_PATH    VRPN_SYSTEMDRIVE "/Program Files/SensAble/3DTouch/utilities/lib/"
#define VRPN_GHOST_31_PATH      VRPN_SYSTEMDRIVE "/Program Files/SensAble/GHOST/v3.1/lib/"
#define VRPN_GHOST_40_PATH      VRPN_SYSTEMDRIVE "/Program Files/SensAble/GHOST/v4.0/lib/"

#define VRPN_NIDAQ_PATH         VRPN_SYSTEMDRIVE "/Program Files/National Instruments/NI-DAQ/Lib/"
#define VRPN_NIDAQ_MX_PATH      VRPN_SYSTEMDRIVE "/Program Files/National Instruments/NI-DAQ/DAQmx ANSI C Dev/lib/msvc/"
#define VRPN_USDIGITAL_PATH     VRPN_SYSTEMDRIVE "/Program Files/SEI Explorer/"

#ifdef  vrpn_USE_MPI
#pragma comment (lib, VRPN_SYSTEMDRIVE "/Program Files/MPICH2/lib/mpi.lib")
#endif

// Load Adrienne libraries if we are using the timecode generator.
// If this doesn't match where you have installed these libraries,
// edit the following lines to point at the correct libraries.  Do
// this here rather than in the project settings so that it can be
// turned on and off using the definition above.
#ifdef	VRPN_INCLUDE_TIMECODE_SERVER
#pragma comment (lib, "../../Adrienne/AEC_DLL/AEC_NTTC.lib")
#endif

#ifdef VRPN_USE_MOTIONNODE
#pragma comment(lib, "libMotionNodeSDK.lib")
#endif

#ifdef VRPN_USE_LIBUSB_1_0
#define VRPN_LIBUSB_PATH  VRPN_SYSTEMDRIVE "/Program Files/libusb-1.0/"
#endif

//---------------------------------------------------------------//
// DO NOT EDIT BELOW THIS LINE FOR NORMAL CONFIGURATION SETTING. //
//---------------------------------------------------------------//

// Load the library for WiiUse.
#ifdef  VRPN_USE_WIIUSE
  #ifdef	_DEBUG
    #pragma comment(lib, VRPN_WIIUSE_LIB_PATH "/msvc/Debug/wiiuse.lib")
  #else
    #pragma comment(lib, VRPN_WIIUSE_LIB_PATH "/msvc/Release/wiiuse.lib")
  #endif
#endif

#ifdef  VRPN_USE_FREESPACE
  #ifdef	_DEBUG
//    #pragma comment(lib, VRPN_FREESPACE_LIB_PATH "/Debug/libfreespaced.lib")
    #pragma comment(lib, VRPN_FREESPACE_LIB_PATH "/Release/libfreespace.lib")
  #else
    #pragma comment(lib, VRPN_FREESPACE_LIB_PATH "/Release/libfreespace.lib")
  #endif
#endif

// Load libowlsock.lib if we're using Phasespace.
#ifdef	VRPN_INCLUDE_PHASESPACE
#pragma comment (lib, VRPN_PHASESPACE_LIB_PATH "libowlsock.lib")
#endif

// Load VRPN Phantom library if we are using phantom server as unified server
// Load SensAble Technologies GHOST library to run the Phantom
// NOTE: The paths to these libraries are set in the Settings/Link tab of
// the various project files.  The paths to the include files are in the
// Settings/C++/preprocessor tab.
#ifdef VRPN_USE_PHANTOM_SERVER
  #ifdef VRPN_USE_HDAPI
    #pragma comment (lib, VRPN_HDAPI_PATH "hd.lib")
    #ifdef	_DEBUG
      #pragma comment (lib,VRPN_HDAPI_UTIL_PATH "hdud.lib")
    #else
      #pragma comment (lib,VRPN_HDAPI_UTIL_PATH "hdu.lib")
    #endif
    #pragma comment (lib, VRPN_HDAPI_PATH "hl.lib")
  #else
    #ifdef VRPN_USE_GHOST_31
      #pragma comment (lib,VRPN_GHOST_31_PATH "GHOST31.lib")
    #else
      #pragma comment (lib,VRPN_GHOST_40_PATH "GHOST40.lib")
    #endif
  #endif
#endif

// DirectInput include file and libraries.
// Load DirectX SDK libraries and tell which version we need if we are using it.
#ifdef	VRPN_USE_DIRECTINPUT
#define	DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#endif

#ifdef	VRPN_USE_DIRECTINPUT
#pragma comment (lib, "dxguid.lib")
// Newer versions of the SDK have renamed this dxerr.lib;
// dxerr9.lib has also been said to work.
#pragma comment (lib, "dxerr.lib")
#pragma comment (lib, "dinput8.lib")
#endif

// Load National Instruments libraries if we are using them.
// If this doesn't match where you have installed these libraries,
// edit the following lines to point at the correct libraries.  Do
// this here rather than in the project settings so that it can be
// turned on and off using the definition above.
// NOTE: The paths to these libraries are set in the Settings/Link tab of
// the various project files.  The paths to the include files are in the
// Settings/C++/preprocessor tab.
#ifdef	VRPN_USE_NATIONAL_INSTRUMENTS
#pragma comment (lib, VRPN_NIDAQ_PATH "nidaq32.lib")
#pragma comment (lib, VRPN_NIDAQ_PATH "nidex32.lib")
#endif
#ifdef	VRPN_USE_NATIONAL_INSTRUMENTS_MX
#pragma comment (lib, VRPN_NIDAQ_MX_PATH "NIDAQmx.lib")
#endif

// Load US Digital libraries if we are using them.
// If this doesn't match where you have installed these libraries,
// edit the following lines to point at the correct libraries.  Do
// this here rather than in the project settings so that it can be
// turned on and off using the definition above.
// NOTE: The paths to these libraries are set in the Settings/Link tab of
// the various project files.  The paths to the include files are in the
// Settings/C++/preprocessor tab.
#ifdef  VRPN_USE_USDIGITAL
#pragma comment (lib, VRPN_USDIGITAL_PATH "SEIDrv32.lib")
#endif

// Load Microscribe-3D SDK libraries
// If this doesn't match where you have installed these libraries,
// edit the following lines to point at the correct libraries.  Do
// this here rather than in the project settings so that it can be
// turned on and off using the definition above.
#ifdef        VRPN_USE_MICROSCRIBE
#pragma comment (lib, "armdll32.lib")
#endif

// Load Trivisio Colibri library
#ifdef  VRPN_USE_TRIVISIOCOLIBRI
#pragma comment (lib, VRPN_TRIVISIOCOLIBRI_LIB_PATH "Trivisio.lib")
#endif

// Load Arrington Research ViewPoint EyeTracker library
#ifdef  VRPN_USE_VIEWPOINT
#pragma comment (lib, VRPN_VIEWPOINT_LIB_PATH "VPX_InterApp.lib")
#endif

#ifdef VRPN_USE_LIBUSB_1_0
#pragma comment(lib, VRPN_LIBUSB_PATH "libusb-1.0.lib")
#endif

// This will be defined in the VRPN (non-DLL) project and nothing else
// Overrides USE_SHARED_LIBRARY to get rid of "inconsistent DLL linkage" warnings.
#ifdef VRPNDLL_NOEXPORTS
#undef VRPN_USE_SHARED_LIBRARY
#endif

// This will be defined in the VRPN (DLL) project and nothing else
// Forces "USE_SHARED_LIBRARY independent of definition above so that the
// DLL will build
#if defined(VRPNDLL_EXPORTS) && !defined(VRPN_USE_SHARED_LIBRARY)
#define VRPN_USE_SHARED_LIBRARY
#endif

// For client code, make sure we add the proper library dependency to the linker
#ifdef _WIN32
#pragma comment (lib, "wsock32.lib")  // VRPN requires the Windows Sockets library.
#ifdef VRPN_USE_SHARED_LIBRARY
#ifdef VRPNDLL_EXPORTS
#define  VRPN_API		 __declspec(dllexport)
#else
#define  VRPN_API		 __declspec(dllimport)
#endif
#else
#define  VRPN_API
#endif
#define	 VRPN_CALLBACK	 __stdcall
#else
// In the future, other architectures may need their own sections
#define  VRPN_API
#define  VRPN_CALLBACK
#endif

#define	VRPN_CONFIGURE_H
#endif

#else // VRPN_USING_CMAKE

// When using CMake, we need to use the vrpn_Configure.h generated in the
// build directory instead.

//#pragma message "NOTE: File included \"vrpn_Configure.h\" from the source dir even though this is a CMake build!"

#include <vrpn_Configure.h>
#endif
