//
// Config.pp
//
// This file defines certain configuration variables that are written
// into the various make scripts.  It is processed by ppremake (along
// with the Sources.pp files in each of the various directories) to
// generate build scripts appropriate to each environment.
//
// ppremake is capable of generating generic Unix autoconf/automake
// style build scripts, as well as makefiles customized for SGI's
// MipsPRO compiler or for Microsoft's Visual C++.  It can also
// generate Microsoft Developer's Studio project files directly.  In
// principle, it can be extended to generate suitable build script
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
//
// In general, #defer is used in this file, to allow the user to
// redefine critical variables in his or her own Config.pp file.




// What kind of build scripts are we generating?  This selects a
// suitable template file from the ppremake system files.  The
// allowable choices, at present, are:
//
//  autoconf  - Generate configure.in and a series of Makefile.am files,
//              suitable for using with autoconf/automake.  Do not use
//              this mode yet; it's not finished.
//  stopgap   - Generate original Frang-style Makefile/Makefile.install/etc.
//              files, to ease transition to the new system.
//  unix      - Generate makefiles suitable for most Unix platforms,
//              without using autoconf.
//  msvc      - Generate makefiles suitable for building on Windows platforms
//              (e.g. Windows NT, Windows 2000) using the Microsoft Visual C++
//              command-line compiler and Microsoft nmake.
//
#define BUILD_TYPE stopgap


// What is the default install directory for all trees in the Panda
// suite?  You may also override this for a particular tree by
// defining a variable name like DTOOL_INSTALL or PANDA_INSTALL.  This
// variable will have no effect when you are using the cttools to
// control your attachment to the trees; in this case, the install
// directory for each tree will by default be the root of the tree
// itself (although this may be overridden).
#define INSTALL_DIR /usr/local/panda


// What level of compiler optimization/debug symbols should we build?
// The various optimize levels are defined as follows:
//
//   1 - No compiler optimizations, full debug symbols
//   2 - Full compiler optimizations, full debug symbols
//         (if the compiler supports this)
//   3 - Full compiler optimizations, no debug symbols
//   4 - Full optimizations, no debug symbols, and asserts removed
//
// Setting this has no effect when BUILD_TYPE is "stopgap".  In this
// case, the compiler optimizations are selected by setting the
// environment variable OPTIMIZE accordingly at compile time.
#define OPTIMIZE 2



////////////////////////////////////////////////////////////////////
// The remaining variables are considered only if BUILD_TYPE is not
// "autoconf".  (Autoconf can determine these directly.)
////////////////////////////////////////////////////////////////////

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

// What additional options should be passed to interrogate when
// generating either of the above two interfaces?  Generally, you
// probably don't want to mess with this.
#define INTERROGATE_OPTIONS -fnames -string -refcount -assert -promiscuous

// Is Python installed, and should Python interfaces be generated?  If
// Python is installed, which directory is it in?  (If the directory
// is someplace standard like /usr/include, you may leave it blank.)
#define PYTHON_IPATH /usr/local/include/python1.6
#define PYTHON_LPATH
#defer HAVE_PYTHON $[isdir $[PYTHON_IPATH]]

// Is NSPR installed, and where?
#define NSPR_IPATH /usr/local/include/nspr
#define NSPR_LPATH /usr/local/lib
#define NSPR_LIBS nspr4
#defer HAVE_NSPR $[libtest $[NSPR_LPATH],$[NSPR_LIBS]]

// Is VRPN installed, and where?
#define VRPN_IPATH
#define VRPN_LPATH
#defer HAVE_VRPN $[isdir $[VRPN_IPATH]]

// Is ZLIB installed, and where?
#define ZLIB_IPATH
#define ZLIB_LPATH
#define ZLIB_LIBS z
#defer HAVE_ZLIB $[libtest $[ZLIB_LPATH],$[ZLIB_LIBS]]

// Is the sox libst library installed, and where?
#define SOXST_IPATH
#define SOXST_LPATH
#define SOXST_LIBS st
#defer HAVE_SOXST $[libtest $[SOXST_LPATH],$[SOXST_LIBS]]

// Is OpenGL installed, and where?  This should include libGL as well
// as libGLU, if they are in different places.
#define GL_IPATH
#define GL_LPATH /usr/X11R6/lib
#if $[eq $[PLATFORM],Win32]
  #define GL_LIBS \
     opengl32.lib glu32.lib winmm.lib kernel32.lib \
     oldnames.lib mswsock.lib ws2_32.lib \
     advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib
#else
  #define GL_LIBS GL GLU
#endif
#defer HAVE_GL $[libtest $[GL_LPATH],$[GL_LIBS]]

// How about GLX?
#define GLX_IPATH
#define GLX_LPATH
#if $[eq $[PLATFORM],Win32]
  #defer HAVE_GLX
#else
  #defer HAVE_GLX $[HAVE_GL]
#endif

// Glut?
#define GLUT_IPATH
#define GLUT_LPATH
#define GLUT_LIBS glut
#defer HAVE_GLUT $[libtest $[GLUT_LPATH],$[GLUT_LIBS]]

// Should we try to build the WGL interface?
#define HAVE_WGL

// Should we try to build the SGI-specific glxdisplay?
#define HAVE_SGIGL

// Should we try to build the DirectX interface?  What additional
// libraries do we need?
#define DX_IPATH
#define DX_LPATH
#define DX_LIBS \
  dxguid.lib winmm.lib kernel32.lib gdi32.lib comdlg32.lib winspool.lib \
  user32.lib advapi32.lib ddraw.lib d3dim.lib
#defer HAVE_DX $[libtest $[DX_LPATH],$[DX_LIBS]]

// Do you want to build the Renderman interface?
#define HAVE_RIB

// Is Mikmod installed?  How should we run the libmikmod-config program?
#define MIKMOD_CONFIG libmikmod-config
#defer HAVE_MIKMOD $[bintest $[MIKMOD_CONFIG]]

// Do you want to build the network interface?  What additional libraries
// are required?
#define NET_IPATH
#define NET_LPATH
#if $[eq $[PLATFORM],Win32]
  #define NET_LIBS ws2_32.lib
#else
  #define NET_LIBS
#endif
#define HAVE_NET 1

// Do you want to build the audio interface?  What additional
// libraries are required?
#define AUDIO_IPATH
#define AUDIO_LPATH
#if $[eq $[PLATFORM],Win32]
  #define AUDIO_LIBS winmm.lib dsound.lib user32.lib ole32.lib dxguid.lib
#else
  #define AUDIO_LIBS
#endif
#define HAVE_AUDIO 1


// Is Gtk-- installed?  How should we run the gtkmm-config program?
// This matters only to programs in PANDATOOL.
#define GTKMM_CONFIG gtkmm-config
#defer HAVE_GTKMM $[bintest $[GTKMM_CONFIG]]

// Is Maya installed?  This matters only to programs in PANDATOOL.
#define MAYA_LOCATION /usr/aw/maya2.5
#defer HAVE_MAYA $[isdir $[MAYA_LOCATION]]


///////////////////////////////////////////////////////////////////////
// The following variables are meaningful when BUILD_TYPE is "unix" or
// "msvc".  They define a few environmental things.
//////////////////////////////////////////////////////////////////////

// How to invoke bison and flex.  Panda takes advantage of some
// bison/flex features, and therefore specifically requires bison and
// flex, not some other versions of yacc and lex.  You can build Panda
// without having bison or flex, but only if you obtained Panda from a
// tarball or zip archive that included the source files generated by
// bison and flex, and only if you do not modify any bison or flex
// sources.
#defer BISON bison
#defer FLEX flex 

// How to invoke sed.  A handful of make rules use this.  Since some
// platforms (specifically, non-Unix platforms like Windows) don't
// have any kind of sed, ppremake performs some limited sed-like
// functions.  The default is to use ppremake in this capacity.  In
// this variable, $[source] is the name of the file to read, $[target]
// is the name of the file to generate, and $[script] is the one-line
// sed script to run.
#defer SED ppremake -s '$[script]' <$[source] >$[target]

// What directory name (within each source directory) should the .o
// (or .obj) files be written to, for both shared and static sources?
// In general, it is safe to define these to be the same.  However,
// don't define these to be '.', or you will be very sad the next time
// you run 'make clean'.
#defer ODIR Opt$[OPTIMIZE]-$[PLATFORM]
#defer ODIR_SHARED $[ODIR]
#defer ODIR_STATIC $[ODIR]



///////////////////////////////////////////////////////////////////////
// The following variables are only meaningful when BUILD_TYPE is
// "unix".  These define the commands to invoke the compiler, linker,
// etc.
//////////////////////////////////////////////////////////////////////

// How to invoke the C and C++ compilers.
#defer CC gcc
#defer CXX g++

// How to compile a C or C++ file into a .o file.  $[target] is the
// name of the .o file, $[source] is the name of the source file,
// $[ipath] is a space-separated list of directories to search for
// include files, and $[flags] is a list of additional flags to pass
// to the compiler.
#defer COMPILE_C $[CC] -c -o $[target] $[ipath:%=-I%] $[flags] $[source]
#defer COMPILE_C++ $[CXX] -c -o $[target] $[ipath:%=-I%] $[flags] $[source]

// What flags should be passed to both C and C++ compilers to enable
// compiler optimizations?  This will be supplied when OPTIMIZE
// (above) is set to 2, 3, or 4.
#defer OPTFLAGS -O2

// What define variables should be passed to the compilers for each
// value of OPTIMIZE?  We separate this so we can pass these same
// options to interrogate, guaranteeing that the correct interfaces
// are generated.  Do not include -D here; that will be supplied
// automatically.
#defer CDEFINES_OPT1
#defer CDEFINES_OPT2
#defer CDEFINES_OPT3
#defer CDEFINES_OPT4 NDEBUG

// What additional flags should be passed for each value of OPTIMIZE
// (above)?  We separate out the compiler-optimization flags, above,
// so we can compile certain files that give optimizers trouble (like
// the output of lex and yacc) without them, but with all the other
// relevant flags.
#defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=-D%] -Wall -g
#defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=-D%] -Wall -g
#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=-D%]
#defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=-D%]

// What additional flags should be passed to both compilers when
// building shared (relocatable) sources?  Some architectures require
// special support for this.
#defer CFLAGS_SHARED -fPIC

// How to generate a C or C++ executable from a collection of .o
// files.  $[target] is the name of the binary to generate, and
// $[sources] is the list of .o files.  $[libs] is a space-separated
// list of dependent libraries, and $[lpath] is a space-separated list
// of directories in which those libraries can be found.
#defer LINK_BIN_C $[CC] -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]
#defer LINK_BIN_C++ $[CXX] -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]

// How to generate a static C or C++ library.  $[target] is the
// name of the library to generate, and $[sources] is the list of .o
// files that will go into the library.
#defer STATIC_LIB_C ar cru $[target] $[sources]
#defer STATIC_LIB_C++ ar cru $[target] $[sources]

// How to run ranlib, if necessary, after generating a static library.
// $[target] is the name of the library.  Set this to the empty string
// if ranlib is not necessary on your platform.
#defer RANLIB ranlib $[target]

// How to generate a shared C or C++ library.  $[source] and $[target]
// as above, and $[libs] is a space-separated list of dependent
// libraries, and $[lpath] is a space-separated list of directories in
// which those libraries can be found.
#defer SHARED_LIB_C $[CC] -shared -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]
#defer SHARED_LIB_C++ $[CXX] -shared -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]

// How to install a data file or executable file.  $[local] is the
// local name of the file to install, and $[dest] is the name of the
// directory to put it in.
#defer INSTALL install -m 644 $[local] $[dest]
#defer INSTALL_PROG install -m 755 $[local] $[dest]

// When building under Irix, we assume you want to use the MIPSPro
// compiler.  Comment this bit out (or redefine the variables
// yourself) if you'd rather use gcc or some other compiler.
#if $[eq $[PLATFORM],Irix]
  #defer CC cc -n32 -mips3
  #defer CXX CC -n32 -mips3
    
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
  #defer SHARED_LIB_C $[CC] -shared $[SHARED_FLAGS] -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]
  #defer SHARED_LIB_C++ $[CXX] -shared $[SHARED_FLAGS] -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]
#endif


//////////////////////////////////////////////////////////////////////
// There are also some additional variables that control specific
// compiler/platform features or characteristics, defined in the
// platform specific file Config.platform.pp.  Be sure to inspect
// these variables for correctness too.  As above, these are
// unnecessary when BUILD_TYPE is "autoconf".
//////////////////////////////////////////////////////////////////////
