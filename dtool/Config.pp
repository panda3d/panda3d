//
// dtool/Config.pp
//
// This file defines certain configuration variables that are written
// into the various make scripts.  It is processed by ppremake (along
// with the Sources.pp files in each of the various directories) to
// generate build scripts appropriate to each environment.
//
// ppremake is capable of generating makefiles for Unix compilers such
// as gcc or SGI's MipsPRO compiler, as well as for Windows
// environments like Microsoft's Visual C++.  It can also,
// potentially, generate Microsoft Developer's Studio project files
// directly, although we haven't written the scripts to do this yet.
// In principle, it can be extended to generate suitable build script
// files for any number of different build environments.
//
// All of these build scripts can be tuned for a particular
// environment via this file.  This is the place for the user to
// specify which external packages are installed and where, or to
// enable or disable certain optional features.  However, it is
// suggested that rather than modify this file directly, you create a
// custom file in your home directory and there redefine whatever
// variables are appropriate, and set the environment variable
// PPREMAKE_CONFIG to refer to it.  In this way, you can easily get an
// updated source tree (including a new Config.pp) without risking
// accidentally losing your customizations.  This also avoids having
// to redefine the same variables in different packages (for instance,
// in dtool and in panda).
//
// The syntax in this file resembles some hybrid between C++
// preprocessor declarations and GNU make variables.  This is the same
// syntax used in the various ppremake system configure files; it's
// designed to be easy to use as a macro language to generate
// makefiles and their ilk.
//

// Some of the variables below are defined using the #define command,
// and others are defined using #defer.  The two are very similar in
// their purpose; the difference is that, if the variable definition
// includes references to other variables (e.g. $[varname]), then
// #define will evaluate all of the other variable references
// immediately and store the resulting expansion, while #defer will
// store only the variable references themselves, and expand them when
// the variable is later referenced.  It is very similar to the
// relationship between := and = in GNU Make.
// dtool/Config.pp

// In general, #defer is used in this file, to allow the user to
// redefine critical variables in his or her own Config.pp file.



// What kind of build scripts are we generating?  This selects a
// suitable template file from the ppremake system files.  The
// allowable choices, at present, are:
//
//  unix      - Generate makefiles suitable for most Unix platforms.
//  msvc      - Generate Visual C++ project files (still a work in progress)
//  gmsvc     - Generate makefiles similar to the above, using Microsoft
//              Visual C++, but uses the Cygwin-supplied GNU make
//              instead of Microsoft nmake.  This is potentially
//              faster if you have multiple CPU's, since it supports
//              distributed make.  It's a tiny bit slower if you're
//              not taking advantage of distributed make, because of
//              the overhead associated with Cygwin fork() calls.

#if $[eq $[PLATFORM], Win32]
//  #define BUILD_TYPE msvc  // not ready yet.
  #define BUILD_TYPE gmsvc
#elif $[eq $[PLATFORM], Cygwin]
  #define BUILD_TYPE gmsvc
#elif $[eq $[PLATFORM], osx]
  #define BUILD_TYPE osx
#else
  #define BUILD_TYPE unix
#endif

// What is the default install directory for all trees in the Panda
// suite?  The default value for this variable is provided by
// ppremake; on Unix machines it is the value of --prefix passed in to
// the configure script, and on Windows machines the default is
// hardcoded in config_msvc.h to C:\Panda3d.

// You may also override this for a particular tree by defining a
// variable name like DTOOL_INSTALL or PANDA_INSTALL.  This variable
// will have no effect when you are using the cttools to control your
// attachment to the trees; in this case, the install directory for
// each tree will by default be the root of the tree itself (although
// this may be overridden).

// #define INSTALL_DIR /usr/local/panda

// If you intend to use Panda only as a Python module, you may find
// the following define useful (but you should put in the correct path
// to site-packages within your own installed Python).  This will
// install the Panda libraries into the standard Python search space
// so that they can be accessed as Python modules.  (Also see the
// PYTHON_IPATH variable, below.)

// If you don't do this, you can still use Panda as a Python module,
// but you must put /usr/local/panda/lib (or $INSTALL_DIR/lib) on
// your PYTHONPATH.

// #define INSTALL_LIB_DIR /usr/lib/python2.2/site-packages


// The PRC files are used by Panda for runtime configuration.  Panda
// will load up all files named *.prc in the directory specified by
// the PRC_DIR environment variable, or in the directory named here if
// that environment variable is undefined.  Config files are loaded up
// in alphabetical order (sorted by ASCII value), and the
// alphabetically last files have precedence.

// By default, we specify the install/etc dir, which is where the
// system-provided PRC files get copied to.
#defer DEFAULT_PRC_DIR $[INSTALL_DIR]/etc


// What level of compiler optimization/debug symbols should we build?
// The various optimize levels are defined as follows:
//
//   1 - No compiler optimizations, full debug symbols
//   2 - Full compiler optimizations, full debug symbols
//         (if the compiler supports this)
//   3 - Full compiler optimizations, no debug symbols
//   4 - Full optimizations, no debug symbols, and asserts removed
//
#define OPTIMIZE 3

// NOTE: In the following, to indicate "yes" to a yes/no question,
// define the variable to be a nonempty string.  To indicate "no",
// define the variable to be an empty string.

// Many of the HAVE_* variables are defined in terms of expressions
// based on the paths and library names, etc., defined above.  These
// are defined using the "defer" command, so that they are not
// evaluated right away, giving the user an opportunity to redefine
// the variables they depend on, or to redefine the HAVE_* variables
// themselves (you can explicitly define a HAVE_* variable to some
// nonempty string to force the package to be marked as installed).


// Do you want to generate a Python-callable interrogate interface?
// This is only necessary if you plan to make calls into Panda from a
// program written in Python.  This is done only if HAVE_PYTHON,
// below, is also true.
#define INTERROGATE_PYTHON_INTERFACE 1

// Do you want to generate a C-callable interrogate interface?  This
// generates an interface similar to the Python interface above, with
// a C calling convention.  It should be useful for most other kinds
// of scripting language; the VR Studio used to use this to make calls
// into Panda from Squeak.  This is not presently used by any VR
// Studio code.
#define INTERROGATE_C_INTERFACE

// Do you even want to build interrogate at all?  This is the program
// that reads our C++ source files and generates one of the above
// interfaces.  If you won't be building the interfaces, you don't
// need the program.
#defer HAVE_INTERROGATE $[or $[INTERROGATE_PYTHON_INTERFACE],$[INTERROGATE_C_INTERFACE]]

// What additional options should be passed to interrogate when
// generating either of the above two interfaces?  Generally, you
// probably don't want to mess with this.
#define INTERROGATE_OPTIONS -fnames -string -refcount -assert

// What's the name of the interrogate binary to run?  The default
// specified is the one that is built as part of DTOOL.  If you have a
// prebuilt binary standing by (for instance, one built opt4), specify
// its name instead.
#define INTERROGATE interrogate
#define INTERROGATE_MODULE interrogate_module

// Is Python installed, and should Python interfaces be generated?  If
// Python is installed, which directory is it in?
#define PYTHON_IPATH /usr/include/python2.2
#define PYTHON_LPATH
#define PYTHON_FPATH
#define PYTHON_FRAMEWORK
#defer HAVE_PYTHON $[isdir $[PYTHON_IPATH]]

// Define the default set of libraries to be instrumented by
// genPyCode.  You may wish to add to this list to add your own
// libraries, or if you want to use some of the more obscure
// interfaces like libpandaegg and libpandafx.
#define GENPYCODE_LIBS libpandaexpress libpanda libpandaphysics libdirect

// Normally, Python source files are copied into the INSTALL_LIB_DIR
// defined above, along with the compiled C++ library objects, when
// you make install.  If you prefer not to copy these Python source
// files, but would rather run them directly out of the source
// directory (presumably so you can develop them and make changes
// without having to reinstall), comment out this definition and put
// your source directory on your PYTHONPATH.
#define INSTALL_PYTHON_SOURCE 1

// Do you want to enable the "in_interpreter" global variable?  This
// will enable some callbacks, particularly the MemoryUsage object, to
// know whether they were called from Python code (or other high-level
// show code) and react accordingly, generally for debugging
// purporses.  It adds a bit of runtime overhead, and isn't usually
// useful unless we're building a debug tree anyway.  The default is
// to enable it only for optimize levels 1 and 2.
#defer TRACK_IN_INTERPRETER $[<= $[OPTIMIZE], 2]

// Do you want to compile in support for tracking memory usage?  This
// enables you to define the variable "track-memory-usage" at runtime
// to help track memory leaks, and also report total memory usage on
// PStats.  There is some small overhead for having this ability
// available, even if it is unused.
#defer DO_MEMORY_USAGE $[<= $[OPTIMIZE], 3]

// Do you want to compile in support for pipelining?  This enables
// setting and accessing multiple different copies of frame-specific
// data stored in nodes, etc.  At the moment, Panda cannot actually
// take advantage of this support to do anything useful, but
// eventually this will enable multi-stage pipelining of the render
// process, as well as potentially remote rendering using a
// distributed scene graph.  For now, we enable this when building
// optimize 1 only, since turning this on does perform some additional
// sanity checks, but doesn't do anything else useful other than
// increase run-time overhead.
#defer DO_PIPELINING $[<= $[OPTIMIZE], 1]

// Is NSPR installed, and where?  This is the Netscape Portable
// Runtime library, downloadable as part of the Mozilla package from
// mozilla.org.  It provides portable threading and networking
// services to Panda.  Panda should compile without it, although
// without any threading or networking capabilities; eventually,
// native support for these capabilities may be added for certain
// platforms.  See also HAVE_IPC and HAVE_NET.
#define NSPR_IPATH /usr/include/nspr
#define NSPR_LPATH
#define NSPR_LIBS nspr4
#defer HAVE_NSPR $[libtest $[NSPR_LPATH],$[NSPR_LIBS]]

// Is a third-party STL library installed, and where?  This is only
// necessary if the default include and link lines that come with the
// compiler don't provide adequate STL support.  At least some form of
// STL is absolutely required in order to build Panda.
#define STL_IPATH
#define STL_LPATH
#define STL_CFLAGS
#define STL_LIBS

// Is OpenSSL installed, and where?
#define SSL_IPATH /usr/local/ssl/include
#define SSL_LPATH /usr/local/ssl/lib
#define SSL_LIBS ssl crypto
#defer HAVE_SSL $[libtest $[SSL_LPATH],$[SSL_LIBS]]
// Define this nonempty if your version of OpenSSL is 0.9.7 or better.
#define SSL_097

// Define this true to include the OpenSSL code to report verbose
// error messages when they occur.
#defer REPORT_OPENSSL_ERRORS $[< $[OPTIMIZE], 4]

// Is libjpeg installed, and where?
#define JPEG_IPATH
#define JPEG_LPATH
#define JPEG_LIBS jpeg
#defer HAVE_JPEG $[libtest $[JPEG_LPATH],$[JPEG_LIBS]]

// Is libpng installed, and where?
#define PNG_IPATH
#define PNG_LPATH
#define PNG_LIBS png
#defer HAVE_PNG $[libtest $[PNG_LPATH],$[PNG_LIBS]]

// Is libtiff installed, and where?
#define TIFF_IPATH
#define TIFF_LPATH
#define TIFF_LIBS tiff z
#defer HAVE_TIFF $[libtest $[TIFF_LPATH],$[TIFF_LIBS]]


// Is libfftw installed, and where?
#define FFTW_IPATH /usr/local/include
#define FFTW_LPATH /usr/local/lib
#define FFTW_LIBS rfftw fftw
#defer HAVE_FFTW $[libtest $[FFTW_LPATH],$[FFTW_LIBS]]


// Is NURBS++ installed, and where?
#define NURBSPP_IPATH /usr/local/include/nurbs++
#define NURBSPP_LPATH /usr/local/lib
#define NURBSPP_LIBS nurbsf matrixN matrixI matrix
#defer HAVE_NURBSPP $[libtest $[NURBSPP_LPATH],$[NURBSPP_LIBS]]


// Is VRPN installed, and where?
#define VRPN_IPATH
#define VRPN_LPATH
#define VRPN_LIBS
#defer HAVE_VRPN $[libtest $[VRPN_LPATH],$[VRPN_LIBS]]

// Is ZLIB installed, and where?
#define ZLIB_IPATH
#define ZLIB_LPATH
#define ZLIB_LIBS z
#defer HAVE_ZLIB $[libtest $[ZLIB_LPATH],$[ZLIB_LIBS]]

// Is OpenGL installed, and where?  This should include libGL as well
// as libGLU, if they are in different places.
#if $[WINDOWS_PLATFORM]
  #defer GL_IPATH
  #defer GL_LPATH
  #define GL_LIBS opengl32.lib glu32.lib
#else
  #defer GL_IPATH
  #defer GL_LPATH /usr/X11R6/lib
  #defer GL_LIBS GL GLU
#endif
#defer HAVE_GL $[libtest $[GL_LPATH],$[GL_LIBS]]

// Is Mesa installed separately from OpenGL?  Mesa is an open-source
// software-only OpenGL renderer.  Panda can link with it
// independently from OpenGL (and if Mesa is built statically, and/or
// with -DUSE_MGL_NAMESPACE declared to rename gl* to mgl*, it can
// switch between the system OpenGL implementation and the Mesa
// implementation at runtime).

// Also, Mesa includes some core libraries (in libOSMesa.so) that
// allow totally headless rendering, handy if you want to run a
// renderer as a batch service, and you don't want to insist that a
// user be logged on to the desktop or otherwise deal with X11 or
// Windows.

// If you define HAVE_MESA here, and the appropriate paths to headers
// and libraries, then Panda will build libmesadisplay, which can be
// used in lieu of libpandagl or libpandadx to do rendering.  However,
// for most applications, you don't need to do this, since (a) if you
// have hardware rendering capability, you probably don't want to use
// Mesa, since it's software-only, and (b) if you don't have hardware
// rendering, you can install Mesa as the system's OpenGL
// implementation, so you can just use the normal libpandagl.  You
// only need to define HAVE_MESA if you want to run totally headless,
// or if you want to be able to easily switch between Mesa and the
// system OpenGL implementation at runtime.  If you compiled Mesa with
// USE_MGL_NAMESPACE defined, define MESA_MGL here.
#define MESA_IPATH
#define MESA_LPATH
#define MESA_LIBS
#define MESA_MGL
#defer HAVE_MESA $[libtest $[MESA_LPATH],$[MESA_LIBS]]


// Is the Chromium remote-rendering library installed, and where?
// This should include libcr_opengl32.
#defer CHROMIUM_IPATH
#defer CHROMIUM_LPATH
#defer CHROMIUM_LIBS
#defer HAVE_CHROMIUM $[libtest $[CHROMIUM_LPATH],$[CHROMIUM_LIBS]]

// How about GLX?
#define GLX_IPATH
#define GLX_LPATH
#defer HAVE_GLX $[and $[HAVE_GL],$[UNIX_PLATFORM]]

// Should we try to build the WGL interface?
#defer HAVE_WGL $[and $[HAVE_GL],$[WINDOWS_PLATFORM]]

// Should we try to build the SGI-specific glxdisplay?
#define HAVE_SGIGL $[eq $[PLATFORM],Irix]

// Is DirectX available, and should we try to build with it?
#define DX_IPATH
#define DX_LPATH
#define DX_LIBS d3d8.lib d3dx8.lib dxerr8.lib
#defer HAVE_DX $[libtest $[DX_LPATH],$[DX_LIBS]]

// Do you want to build the DirectD tools for starting Panda clients
// remotely?  This only affects the direct tree.  Enabling this may
// cause libdirect.dll to fail to load on Win98 clients.
#define HAVE_DIRECTD

// Do you want to build in support for threading (multiprocessing)?
// Building in support for threading will enable Panda to take
// advantage of multiple CPU's if you have them (and if the OS
// supports kernel threads running on different CPU's), but it will
// slightly slow down Panda for the single CPU case, so this is not
// enabled by default.

// Currently, threading support requires NSPR, so you should not
// define this true unless you have NSPR installed.
#define HAVE_THREADS

// Do you want to build the network interface?  What additional libraries
// are required?  Currently, this requires NSPR.
#define NET_IPATH
#define NET_LPATH
#if $[WINDOWS_PLATFORM]
  #define NET_LIBS wsock32.lib
#else
  #define NET_LIBS
#endif
#defer HAVE_NET $[HAVE_NSPR]

// Do you want to build the PStats interface, for graphical run-time
// performance statistics?  This requires NET to be available.  By
// default, we don't build PStats when OPTIMIZE = 4, although this is
// possible.
#defer DO_PSTATS $[or $[and $[HAVE_NET],$[< $[OPTIMIZE], 4]], $[DO_PSTATS]]

// Do you want to build the debugging tools for recording and
// visualizing intersection tests by the collision system?  Enabling
// this increases runtime collision overhead just a tiny bit.
#defer DO_COLLISION_RECORDING $[< $[OPTIMIZE], 4]

// Do you want to include the "debug" and "spam" Notify messages?
// Normally, these are stripped out when we build with OPTIMIZE = 4, but
// sometimes it's useful to keep them around.  Redefine this in your
// own Config.pp to achieve that.
#defer NOTIFY_DEBUG $[< $[OPTIMIZE], 4]

// Do you want to build the audio interface?
#define HAVE_AUDIO 1

// Info for the RAD game tools, Miles Sound System
// note this may be overwritten in wintools Config.pp
#define RAD_MSS_IPATH /usr/include/Miles6/include
#define RAD_MSS_LPATH /usr/lib/Miles6/lib/win
#define RAD_MSS_LIBS Mss32
#defer HAVE_RAD_MSS $[libtest $[RAD_MSS_LPATH],$[RAD_MSS_LIBS]]

// Info for the Fmod audio engine
// note this may be overwritten in wintools Config.pp
#define FMOD_IPATH
#define FMOD_LPATH
#define FMOD_LIBS fmod
#defer HAVE_FMOD $[libtest $[FMOD_LPATH],$[FMOD_LIBS]]

// Info for http://www.sourceforge.net/projects/chromium
// note this may be overwritten in wintools Config.pp
#define CHROMIUM_IPATH /usr/include/chromium/include
#define CHROMIUM_LPATH /usr/lib/chromium/bin/WINT_NT
#define CHROMIUM_LIBS spuload
#defer HAVE_CHROMIUM $[libtest $[CHROMIUM_LPATH],$[CHROMIUM_LIBS]]

// Is Gtk-- installed?  How should we run the gtkmm-config program?
// This matters only to programs in PANDATOOL.
#define GTKMM_CONFIG gtkmm-config
#defer HAVE_GTKMM $[bintest $[GTKMM_CONFIG]]

// Do we have Freetype 2.0 (or better)?  If available, this package is
// used to generate dynamic in-the-world text from font files.

// On Unix, freetype comes with the freetype-config executable, which
// tells us where to look for the various files.  On Windows, we need to
// supply this information explicitly.
#define FREETYPE_CONFIG freetype-config
#defer HAVE_FREETYPE $[or $[libtest $[FREETYPE_LPATH],$[FREETYPE_LIBS]],$[bintest $[FREETYPE_CONFIG]]]
#define FREETYPE_CFLAGS
#define FREETYPE_IPATH
#define FREETYPE_LPATH
#define FREETYPE_LIBS

// Define this true to compile in a default font, so every TextNode
// will always have a font available without requiring the user to
// specify one.  Define it empty not to do this, saving a few
// kilobytes on the generated library.  Sorry, you can't pick a
// particular font to be the default; it's hardcoded in the source
// (although you can use the text-default-font prc variable to specify
// a particular font file to load as the default, overriding the
// compiled-in font).
#define COMPILE_IN_DEFAULT_FONT 1

// Is Maya installed?  This matters only to programs in PANDATOOL.

// Also, as of Maya 5.0 it seems the Maya library will not compile
// properly with optimize level 4 set (we get link errors with ostream).

#define MAYA_LOCATION /usr/aw/maya
#defer MAYA_LIBS $[if $[WINDOWS_PLATFORM],Foundation.lib OpenMaya.lib OpenMayaAnim.lib,Foundation OpenMaya OpenMayaAnim]
// Optionally define this to the value of LM_LICENSE_FILE that should
// be set before invoking Maya.
#define MAYA_LICENSE_FILE
#defer HAVE_MAYA $[and $[<= $[OPTIMIZE], 3],$[isdir $[MAYA_LOCATION]/include/maya]]
// Define this if your version of Maya is earlier than 5.0 (e.g. Maya 4.5).
#define MAYA_PRE_5_0

// In the same fashion as mayaegg converter above, set softimage to egg converter as well
#define SOFTIMAGE_LOCATION /c/Softimage/sdk_18sp2/SDK_1.8SP2/SAAPHIRE
#defer SOFTIMAGE_LIBS SAA.lib
#defer HAVE_SOFTIMAGE $[isdir $[SOFTIMAGE_LOCATION]/h]


// Define this to generate static libraries and executables, rather than
// dynamic libraries.
//#define LINK_ALL_STATIC yes

// Define this to export the templates from the DLL.  This is only
// meaningful if LINK_ALL_STATIC is not defined, and we are building
// on Windows.  Some Windows compilers may not support this syntax.
#defer EXPORT_TEMPLATES yes

// Define this to explicitly link in the various external drivers, which
// are normally separate, as part of the Panda library.
//#define LINK_IN_GL yes
//#define LINK_IN_DX yes
//#define LINK_IN_EGG yes
//#define LINK_IN_PHYSICS yes

// Define USE_COMPILER to switch the particular compiler that should
// be used.  A handful of tokens are recognized, depending on BUILD_TYPE.
// This may also be further customized within Global.$[BUILD_TYPE].pp.

// If BUILD_TYPE is "unix", this may be one of:
//    GCC    (gcc/g++)
//    MIPS   (Irix MIPSPro compiler)
//
// If BUILD_TYPE is "msvc" or "gmsvc", this may be one of:
//    MSVC   (Microsoft Visual C++ 6.0)
//    MSVC7  (Microsoft Visual C++ 7.0)
//    BOUNDS (BoundsChecker)
//    INTEL  (Intel C/C++ compiler)

#if $[WINDOWS_PLATFORM]
  #if $[eq $[USE_COMPILER],]
    #define USE_COMPILER MSVC7
  #endif
#elif $[eq $[PLATFORM], Irix]
  #define USE_COMPILER MIPS
#elif $[eq $[PLATFORM], Linux]
  #define USE_COMPILER GCC
#elif $[eq $[PLATFORM], osx]
  #define USE_COMPILER GCC
#endif

// Permission masks to install data and executable files,
// respectively.  This is only meaningful for Unix systems.
#define INSTALL_UMASK_DATA 644
#define INSTALL_UMASK_PROG 755

// How to invoke bison and flex.  Panda takes advantage of some
// bison/flex features, and therefore specifically requires bison and
// flex, not some other versions of yacc and lex.  However, you only
// need to have these programs if you need to make changes to the
// bison or flex sources (see the next point, below).
#defer BISON bison
#defer FLEX flex

// You may not even have bison and flex installed.  If you don't, no
// sweat; Panda ships with the pre-generated output of these programs,
// so you don't need them unless you want to make changes to the
// grammars themselves (files named *.yxx or *.lxx).
#defer HAVE_BISON $[bintest $[BISON]]

// How to invoke sed.  A handful of make rules use this.  Since some
// platforms (specifically, non-Unix platforms like Windows) don't
// have any kind of sed, ppremake performs some limited sed-like
// functions.  The default is to use ppremake in this capacity.  In
// this variable, $[source] is the name of the file to read, $[target]
// is the name of the file to generate, and $[script] is the one-line
// sed script to run.
#defer SED ppremake -s "$[script]" <$[source] >$[target]

// What directory name (within each source directory) should the .o
// (or .obj) files be written to?  This can be any name, and it can be
// used to differentiate different builds within the same tree.
// However, don't define this to be '.', or you will be very sad the
// next time you run 'make clean'.
//#defer ODIR Opt$[OPTIMIZE]-$[PLATFORM]$[USE_COMPILER]
// ODIR_SUFFIX is optional, usually empty
#defer ODIR Opt$[OPTIMIZE]-$[PLATFORM]$[ODIR_SUFFIX]


// What is the normal extension of a compiled object file?
#if $[WINDOWS_PLATFORM]
  #define OBJ .obj
#else
  #define OBJ .o
#endif


///////////////////////////////////////////////////////////////////////
// The following variables are only meaningful when BUILD_TYPE is
// "unix".  These define the commands to invoke the compiler, linker,
// etc.
//////////////////////////////////////////////////////////////////////

// How to invoke the C and C++ compilers.
#if $[eq $[USE_COMPILER], GCC]
  #define CC gcc
  #define CXX g++

  // gcc might run into template limits on some parts of Panda.
  // I upped this from 25 to build on OS X (GCC 3.3) -- skyler.
  #define C++FLAGS_GEN -ftemplate-depth-30
#else
  #define CC cc
  #define CXX CC
#endif

// How to compile a C or C++ file into a .o file.  $[target] is the
// name of the .o file, $[source] is the name of the source file,
// $[ipath] is a space-separated list of directories to search for
// include files, and $[flags] is a list of additional flags to pass
// to the compiler.
#defer COMPILE_C $[CC] $[CFLAGS_GEN] -c -o $[target] $[ipath:%=-I%] $[flags] $[source]
#defer COMPILE_C++ $[CXX] $[C++FLAGS_GEN] -c -o $[target] $[ipath:%=-I%] $[flags] $[source]

// What flags should be passed to both C and C++ compilers to enable
// compiler optimizations?  This will be supplied when OPTIMIZE
// (above) is set to 2, 3, or 4.
#defer OPTFLAGS -O2

// What define variables should be passed to the compilers for each
// value of OPTIMIZE?  We separate this so we can pass these same
// options to interrogate, guaranteeing that the correct interfaces
// are generated.  Do not include -D here; that will be supplied
// automatically.
#defer CDEFINES_OPT1 _DEBUG $[EXTRA_CDEFS]
#defer CDEFINES_OPT2 _DEBUG $[EXTRA_CDEFS]
#defer CDEFINES_OPT3 $[EXTRA_CDEFS]
#defer CDEFINES_OPT4 NDEBUG $[EXTRA_CDEFS]

// What additional flags should be passed for each value of OPTIMIZE
// (above)?  We separate out the compiler-optimization flags, above,
// so we can compile certain files that give optimizers trouble (like
// the output of lex and yacc) without them, but with all the other
// relevant flags.
#defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=-D%] -Wall -g
#defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=-D%] -Wall -g $[OPTFLAGS]
#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=-D%] $[OPTFLAGS]
#defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=-D%] $[OPTFLAGS]

// What additional flags should be passed to both compilers when
// building shared (relocatable) sources?  Some architectures require
// special support for this.
#defer CFLAGS_SHARED -fPIC

// How to generate a C or C++ executable from a collection of .o
// files.  $[target] is the name of the binary to generate, and
// $[sources] is the list of .o files.  $[libs] is a space-separated
// list of dependent libraries, and $[lpath] is a space-separated list
// of directories in which those libraries can be found.
#defer LINK_BIN_C $[cc_ld] -o $[target] $[sources] $[flags] $[lpath:%=-L%] $[libs:%=-l%]\
 $[fpath:%=-Wl,-F%] $[patsubst %,-framework %, $[frameworks]]
#defer LINK_BIN_C++ $[cxx_ld]\
 -o $[target] $[sources]\
 $[flags]\
 $[lpath:%=-L%] $[libs:%=-l%]\
 $[fpath:%=-Wl,-F%] $[patsubst %,-framework %, $[frameworks]]

// How to generate a static C or C++ library.  $[target] is the
// name of the library to generate, and $[sources] is the list of .o
// files that will go into the library.
#if $[eq $[PLATFORM], osx]	
  #defer STATIC_LIB_C libtool -static -o $[target] $[sources]
  #defer STATIC_LIB_C++ libtool -static -o $[target] $[sources]
#else
  #defer STATIC_LIB_C ar cru $[target] $[sources]
  #defer STATIC_LIB_C++ ar cru $[target] $[sources]
#endif

// How to run ranlib, if necessary, after generating a static library.
// $[target] is the name of the library.  Set this to the empty string
// if ranlib is not necessary on your platform.
#defer RANLIB ranlib $[target]

// How to generate a shared C or C++ library.  $[source] and $[target]
// as above, and $[libs] is a space-separated list of dependent
// libraries, and $[lpath] is a space-separated list of directories in
// which those libraries can be found.
#if $[eq $[PLATFORM], osx]
  #defer SHARED_LIB_C $[cc_ld] -o $[target] -install_name $[notdir $[target]] $[sources] $[lpath:%=-L%] $[libs:%=-l%] $[patsubst %,-framework %, $[frameworks]]
  #defer SHARED_LIB_C++ $[cxx_ld] -dynamic -dynamiclib -o $[target] -install_name $[notdir $[target]] $[sources] $[lpath:%=-L%] $[libs:%=-l%] $[patsubst %,-framework %, $[frameworks]]
#else
  #defer SHARED_LIB_C $[cc_ld] -shared -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]
  #defer SHARED_LIB_C++ $[cxx_ld] -shared -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]
#endif

// How to install a data file or executable file.  $[local] is the
// local name of the file to install, and $[dest] is the name of the
// directory to put it in.

// On Unix systems, we strongly prefer using the install program to
// install files.  This has nice features like automatically setting
// the permissions bits, and also is usually clever enough to install
// a running program without crashing the running instance.  However,
// it doesn't understanding installing a program from a subdirectory,
// so we have to cd into the source directory first.
#defer INSTALL $[if $[ne $[dir $[local]], ./],cd ./$[dir $[local]] &&] install -m $[INSTALL_UMASK_DATA] $[notdir $[local]] $[dest]/
#defer INSTALL_PROG $[if $[ne $[dir $[local]], ./],cd ./$[dir $[local]] &&] install -m $[INSTALL_UMASK_PROG] $[notdir $[local]] $[dest]/

// Variable definitions for building with the Irix MIPSPro compiler.
#if $[eq $[USE_COMPILER], MIPS]
  #define CC cc -n32 -mips3
  #define CXX CC -n32 -mips3

  // Turn off a few annoying warning messages.
  // 1174 - function 'blah' was declared but never used
  // 1201 - trailing comma is nonstandard.
  // 1209 - controlling expression is constant, e.g. if (0) { ... }
  // 1234 - access control not specified, 'public' by default
  // 1355 - extra ";" ignored
  // 1375 - destructor for base class is not virtual.
  //    this one actually is bad.  But we got alot of them from the classes
  //    that we've derived from STL collections.  Beware of this.
  // 3322 - omission of explicit type is nonstandard ("int" assumed)
  #define WOFF_LIST -woff 1174,1201,1209,1234,1355,1375,3322

  // Linker warnings
  // 85 - definition of SOMESYMBOL in SOMELIB preempts that of definition in
  //      SOMEOTHERLIB.
  #define WOFF_LIST $[WOFF_LIST] -Wl,-LD_MSG:off=85

  #defer OPTFLAGS -O2 -OPT:Olimit=2500

  #defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=-D%] $[WOFF_LIST] -g
  #defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=-D%] $[WOFF_LIST]
  #defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=-D%] $[WOFF_LIST]
  #defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=-D%] $[WOFF_LIST]

  #defer CFLAGS_SHARED

  #defer STATIC_LIB_C $[CC] -ar -o $[target] $[sources]
  #defer STATIC_LIB_C++ $[CXX] -ar -o $[target] $[sources]
  #defer RANLIB

  #define SHARED_FLAGS -Wl,-none -Wl,-update_registry,$[TOPDIR]/so_locations
  #defer SHARED_LIB_C $[cc_ld] -shared $[SHARED_FLAGS] -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]
  #defer SHARED_LIB_C++ $[cxx_ld] -shared $[SHARED_FLAGS] -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]
#endif


//////////////////////////////////////////////////////////////////////
// There are also some additional variables that control specific
// compiler/platform features or characteristics, defined in the
// platform specific file Config.platform.pp.  Be sure to inspect
// these variables for correctness too.
//////////////////////////////////////////////////////////////////////
