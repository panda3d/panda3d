//
// dtool/Config.pp
//
// This file defines certain configuration variables that are written
// into the various make scripts.  It is processed by ppremake (along
// with the Sources.pp files in each of the various directories) to
// generate build scripts appropriate to each environment.
// 

// *******************************************************************
// NOTE: you should not attempt to copy this file verbatim as your own
// personal Config.pp file.  Instead, you should start with an empty
// Config.pp file, and add lines to it when you wish to override
// settings given in here.  In the normal ppremake system, this file
// will always be read first, and then your personal Config.pp file
// will be read later, which gives you a chance to override the
// default settings found in this file.  However, if you start by
// copying the entire file, it will be difficult to tell which
// settings you have customized, and it will be difficult to upgrade
// to a subsequent version of Panda.
// *******************************************************************

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
//  nmake     - Generate makefiles for Microsoft Visual C++, using 
//              Microsoft's nmake utility.
//  gmsvc     - Generate makefiles similar to the above, using Microsoft
//              Visual C++, but uses the Cygwin-supplied GNU make
//              instead of Microsoft nmake.  This is potentially
//              faster if you have multiple CPU's, since it supports
//              distributed make.  It's a tiny bit slower if you're
//              not taking advantage of distributed make, because of
//              the overhead associated with Cygwin fork() calls.

#if $[eq $[PLATFORM], Win32]
  #define BUILD_TYPE nmake
#elif $[eq $[PLATFORM], Cygwin]
  #define BUILD_TYPE gmsvc
#elif $[OSX_PLATFORM]
  #define BUILD_TYPE unix
#else
  #define BUILD_TYPE unix
#endif

// What is the default install directory for all trees in the Panda
// suite?  The default value for this variable is provided by
// ppremake; on Unix machines it is the value of --prefix passed in to
// the configure script, and on Windows machines the default is
// hardcoded in config_msvc.h to C:\Panda3d.

// You may also override this for a particular tree by defining a
// variable name like DTOOL_INSTALL or PANDA_INSTALL.  (The
// INSTALL_DIR variable will have no effect if you are using the
// ctattach tools to control your attachment to the trees; but this
// will be the case only if you are a member of the VR Studio.)

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


// The character used to separate components of an OS-specific
// directory name depends on the platform (it is '/' on Unix, '\' on
// Windows).  That character selection is hardcoded into Panda and
// cannot be changed here.  (Note that an internal Panda filename
// always uses the forward slash, '/', to separate the components of a
// directory name.)

// There's a different character used to separate the complete
// directory names in a search path specification.  On Unix, the
// normal convention is ':', on Windows, it has to be ';', because the
// colon is already used to mark the drive letter.  This character is
// selectable here.  Most users won't want to change this.  If
// multiple characters are placed in this string, any one of them may
// be used as a separator character.
#define DEFAULT_PATHSEP $[if $[WINDOWS_PLATFORM],;,:]

// What level of compiler optimization/debug symbols should we build?
// The various optimize levels are defined as follows:
//
//   1 - No compiler optimizations, debug symbols, debug heap, lots of checks
//   2 - Full compiler optimizations, debug symbols, debug heap, lots of checks
//   3 - Full compiler optimizations, full debug symbols, fewer checks
//   4 - Full optimizations, no debug symbols, and asserts removed
//
#define OPTIMIZE 3

// On OSX, you may or may not want to compile universal binaries.

// Turning this option on allows your compiled version of Panda to run
// on any version of OSX (PPC or Intel-based), but it will also
// increase the compilation time, as well as the resulting binary
// size.  I believe you have to be building on an Intel-based platform
// to generate universal binaries using this technique.  This option
// has no effect on non-OSX platforms.
#define UNIVERSAL_BINARIES

// Panda uses prc files for runtime configuration.  There are many
// compiled-in options to customize the behavior of the prc config
// system; most users won't need to change any of them.  Feel free to
// skip over all of the PRC_* variables defined here.

// The default behavior is to search for files names *.prc in the
// directory specified by the PRC_DIR environment variable, and then
// to search along all of the directories named by the PRC_PATH
// environment variable.  Either of these variables might be
// undefined; if both of them are undefined, the default is to search
// in the directory named here by DEFAULT_PRC_DIR.

// By default, we specify the install/etc dir, which is where the
// system-provided PRC files get copied to.
#defer DEFAULT_PRC_DIR $[INSTALL_DIR]/etc

// You can specify the names of the environment variables that are
// used to specify the search location(s) for prc files at runtime.
// These are space-separated lists of environment variable names.
// Specify empty string for either one of these to disable the
// feature.  For instance, redefining PRC_DIR_ENVVARS here to
// PANDA_PRC_DIR would cause the environment variable $PANDA_PRC_DIR
// to be consulted at startup instead of the default value of
// $PRC_DIR.
#define PRC_DIR_ENVVARS PRC_DIR
#define PRC_PATH_ENVVARS PRC_PATH

// You can specify the name of the file(s) to search for in the above
// paths to be considered a config file.  This should be a
// space-separated list of filename patterns.  This is *.prc by
// default; normally there's no reason to change this.
#define PRC_PATTERNS *.prc

// You can optionally encrypt your prc file(s) to help protect them
// from curious eyes.  You have to specify the encryption key, which
// gets hard-coded into the executable.  (This feature provides mere
// obfuscation, not real security, since the encryption key can
// potentially be extracted by a hacker.)  This requires building with
// OpenSSL (see below).
#define PRC_ENCRYPTED_PATTERNS *.prc.pe
#define PRC_ENCRYPTION_KEY ""

// One unusual feature of config is the ability to execute one or more
// of the files it discovers as if it were a program, and then treat
// the output of this program as a prc file.  If you want to use this
// feature, define this variable to the filename pattern or patterns
// for such executable-style config programs (e.g. *prc.exe).  This
// can be the same as the above if you like this sort of ambiguity; in
// that case, config will execute the file if it appears to be
// executable; otherwise, it will simply read it.
#define PRC_EXECUTABLE_PATTERNS

// If you do use the above feature, you'll need another environment
// variable that specifies additional arguments to pass to the
// executable programs.  The default definition, given here, makes
// that variable be $PRC_EXECUTABLE_ARGS.  Sorry, the same arguments
// must be supplied to all executables in a given runtime session.
#define PRC_EXECUTABLE_ARGS_ENVVAR PRC_EXECUTABLE_ARGS

// You can implement signed prc files, if you require this advanced
// feature.  This allows certain config variables to be set only by a
// prc file that has been provided by a trusted source.  To do this,
// first install and compile Dtool with OpenSSL (below) and run the
// program make-prc-key, and then specify here the output filename
// generated by that program, and then recompile Dtool (ppremake; make
// install).
#define PRC_PUBLIC_KEYS_FILENAME

// By default, the signed-prc feature, above, is enabled only for a
// release build (OPTIMIZE = 4).  In a normal development environment
// (OPTIMIZE < 4), any prc file can set any config variable, whether
// or not it is signed.  Set this variable true (nonempty) or false
// (empty) to explicitly enable or disable this feature.
#defer PRC_RESPECT_TRUST_LEVEL $[= $[OPTIMIZE],4]

// If trust level is in effect, this specifies the default trust level
// for any legacy (Dconfig) config variables (that is, variables
// created using the config.GetBool(), etc. interface, rather than the
// newer ConfigVariableBool interface).
#defer PRC_DCONFIG_TRUST_LEVEL 0

// If trust level is in effect, you may globally increment the
// (mis)trust level of all variables by the specified amount.
// Incrementing this value by 1 will cause all variables to require at
// least a level 1 signature.
#define PRC_INC_TRUST_LEVEL 0

// Similarly, the descriptions are normally saved only in a
// development build, not in a release build.  Set this value true to
// explicitly save them anyway.
#defer PRC_SAVE_DESCRIPTIONS $[< $[OPTIMIZE],4]

// This is the end of the PRC variable customization section.  The
// remaining variables are of general interest to everyone.



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

// Define this true to use the new interrogate feature to generate
// Python-native objects directly, rather than requiring a separate
// FFI step.  This loads and runs much more quickly than the original
// mechanism.  Define this false (that is, empty) to use the original
// interfaces.
#define PYTHON_NATIVE 1

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
#define PYTHON_IPATH /usr/include/python2.4
#define PYTHON_LPATH
#define PYTHON_FPATH
#define PYTHON_COMMAND python
#defer PYTHON_DEBUG_COMMAND $[PYTHON_COMMAND]$[if $[WINDOWS_PLATFORM],_d]
#define PYTHON_FRAMEWORK
#defer HAVE_PYTHON $[or $[PYTHON_FRAMEWORK],$[isdir $[PYTHON_IPATH]]]

// By default, we'll assume the user only wants to run with Debug
// python if he has to--that is, on Windows when building a debug build.
#defer USE_DEBUG_PYTHON $[and $[< $[OPTIMIZE],3],$[WINDOWS_PLATFORM]]

// Define the default set of libraries to be instrumented by
// genPyCode.  You may wish to add to this list to add your own
// libraries, or if you want to use some of the more obscure
// interfaces like libpandaegg and libpandafx.
#defer GENPYCODE_LIBS libpandaexpress libpanda libpandaphysics libdirect libpandafx $[if $[HAVE_ODE],libpandaode]

// Normally, Python source files are copied into the INSTALL_LIB_DIR
// defined above, along with the compiled C++ library objects, when
// you make install.  If you prefer not to copy these Python source
// files, but would rather run them directly out of the source
// directory (presumably so you can develop them and make changes
// without having to reinstall), comment out this definition and put
// your source directory on your PYTHONPATH.
#define INSTALL_PYTHON_SOURCE 1

// Do you want to compile in support for tracking memory usage?  This
// enables you to define the variable "track-memory-usage" at runtime
// to help track memory leaks, and also report total memory usage on
// PStats.  There is some small overhead for having this ability
// available, even if it is unused.
#defer DO_MEMORY_USAGE $[<= $[OPTIMIZE], 3]

// This option compiles in support for simulating network delay via
// the min-lag and max-lag prc variables.  It adds a tiny bit of
// overhead even when it is not activated, so it is typically enabled
// only in a development build.
#defer SIMULATE_NETWORK_DELAY $[<= $[OPTIMIZE], 3]

// This option compiles in support for immediate-mode OpenGL
// rendering.  Since this is normally useful only for researching
// buggy drivers, and since there is a tiny bit of per-primitive
// overhead to have this option available even if it is unused, it is
// by default enabled only in a development build.  This has no effect
// on DirectX rendering.
#define SUPPORT_IMMEDIATE_MODE $[<= $[OPTIMIZE], 3]

// Do you want to compile in support for pipelining?  This enables
// setting and accessing multiple different copies of frame-specific
// data stored in nodes, etc.  This is necessary, in conjunction with
// HAVE_THREADS, to implement threaded multistage rendering in Panda.
// However, compiling this option in does add some additional runtime
// overhead even if it is not used.  By default, we enable pipelining
// whenever threads are enabled, assuming that if you have threads,
// you also want to use pipelining.  We also enable it at OPTIMIZE
// level 1, since that enables additional runtime checks.
//#defer DO_PIPELINING $[or $[<= $[OPTIMIZE], 1],$[HAVE_THREADS]]

// Actually, let's *not* assume that threading implies pipelining, at
// least not until pipelining is less of a performance hit.
//#defer DO_PIPELINING $[<= $[OPTIMIZE], 1]

// Pipelining is a little broken right now.  Turn it off altogether
// for now.
#defer DO_PIPELINING

// Panda contains some experimental code to compile for IPhone.  This
// requires the Apple IPhone SDK, which is currently only available
// for OS X platforms.  Set this to either "iPhoneSimulator" or
// "iPhoneOS".  Note that this is still *experimental* and incomplete!
// Don't enable this unless you know what you're doing!
#define BUILD_IPHONE

// Do you want to use one of the alternative malloc implementations?
// This is almost always a good idea on Windows, where the standard
// malloc implementation appears to be pretty poor, but probably
// doesn't matter much on Linux (which is likely to implement
// ptmalloc2 anyway).  We always define this by default on Windows; on
// Linux, we define it by default only when DO_MEMORY_USAGE is enabled
// (since in that case, we'll be paying the overhead for the extra
// call anyway) or when HAVE_THREADS is not defined (since the
// non-thread-safe dlmalloc is a tiny bit faster than the system
// library).

// In hindsight, let's not enable this at all.  It just causes
// problems.
//#defer ALTERNATIVE_MALLOC $[or $[WINDOWS_PLATFORM],$[DO_MEMORY_USAGE],$[not $[HAVE_THREADS]]]
#define ALTERNATIVE_MALLOC

// Define this true to use the DELETED_CHAIN macros, which support
// fast re-use of existing allocated blocks, minimizing the low-level
// calls to malloc() and free() for frequently-created and -deleted
// objects.  There's usually no reason to set this false, unless you
// suspect a bug in Panda's memory management code.
#define USE_DELETED_CHAIN 1

// Define this true to build the low-level native network
// implementation.  Normally this should be set true.
#define WANT_NATIVE_NET 1
#define NATIVE_NET_IPATH
#define NATIVE_NET_LPATH
#define NATIVE_NET_LIBS $[if $[WINDOWS_PLATFORM],wsock32.lib]

// Do you want to build the high-level network interface?  This layers
// on top of the low-level native_net interface, specified above.
// Normally, if you build NATIVE_NET, you will also build NET.
#defer HAVE_NET $[WANT_NATIVE_NET]

// Do you want to build the egg loader?  Usually there's no reason to
// avoid building this, unless you really want to make a low-footprint
// build (such as, for instance, for the iPhone).
#define HAVE_EGG 1

// Is a third-party STL library installed, and where?  This is only
// necessary if the default include and link lines that come with the
// compiler don't provide adequate STL support.  At least some form of
// STL is absolutely required in order to build Panda.
#define STL_IPATH
#define STL_LPATH
#define STL_CFLAGS
#define STL_LIBS

// Does your STL library provide hashed associative containers like
// hash_map and hash_set?  Define this true if you have a nonstandard
// STL library that provides these, like Visual Studio .NET's.  (These
// hashtable containers are not part of the C++ standard yet, but the
// Dinkum STL library that VC7 ships with includes a preliminary
// implementation that Panda can optionally use.)  For now, we assume
// you have this by default only on a Windows platform.

// On second thought, it turns out that this API is still too
// volatile.  The interface seems to have changed with the next
// version of .NET, and it didn't present any measureable performance
// gain anyway.  Never mind.
#define HAVE_STL_HASH

// Is OpenSSL installed, and where?
#define OPENSSL_IPATH /usr/local/ssl/include
#define OPENSSL_LPATH /usr/local/ssl/lib
#define OPENSSL_LIBS ssl crypto
#defer HAVE_OPENSSL $[libtest $[OPENSSL_LPATH],$[OPENSSL_LIBS]]
// Redefine this to empty if your version of OpenSSL is prior to 0.9.7.
#define OPENSSL_097 1

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

// These image file formats don't require the assistance of a
// third-party library to read and write, so there's normally no
// reason to disable them in the build, unless you are looking to
// reduce the memory footprint.
#define HAVE_SGI_RGB 1
#define HAVE_TGA 1
#define HAVE_IMG 1
#define HAVE_SOFTIMAGE_PIC 1
#define HAVE_BMP 1
#define HAVE_PNM 1

// Is libtar installed, and where?  This is used to optimize patch
// generation against tar files.
#define TAR_IPATH
#define TAR_LPATH
#define TAR_LIBS tar
#defer HAVE_TAR $[libtest $[TAR_LPATH],$[TAR_LIBS]]


// Is libfftw installed, and where?
#define FFTW_IPATH /opt/local/include
#define FFTW_LPATH /opt/local/lib
#define FFTW_LIBS rfftw fftw
#defer HAVE_FFTW $[libtest $[FFTW_LPATH],$[FFTW_LIBS]]
// This is because darwinport's version of the fftw lib is called
// drfftw instead of rfftw.
#defer HAVE_DRFFTW_H $[libtest $[FFTW_LPATH],drfftw]

// Is libsquish installed, and where?
#define SQUISH_IPATH /usr/local/include
#define SQUISH_LPATH /usr/local/lib
#define SQUISH_LIBS squish
#defer HAVE_SQUISH $[libtest $[SQUISH_LPATH],$[SQUISH_LIBS]]


// Is Berkeley DB installed, and where?  Presently, this is only used
// for some applications (egg-optchar in particular) in Pandatool, and
// it is completely optional there.  If available, egg-optchar takes
// advantage of it to allow the optimization of very large numbers of
// models in one pass, that might otherwise exceed available memory.

// Actually, this isn't even true anymore.  At the time of this writing,
// no system in Panda makes use of Berkeley DB.  So don't bother to
// define this.
#define BDB_IPATH
#define BDB_LPATH
#define BDB_LIBS db db_cxx
#defer HAVE_BDB $[libtest $[BDB_LPATH],$[BDB_LIBS]]

// Is Cg installed, and where?
#if $[WINDOWS_PLATFORM]
  #define CG_IPATH 
  #define CG_LPATH
  #define CG_LIBS cg.lib
#else
  #define CG_IPATH
  #define CG_LPATH
  #define CG_LIBS Cg
#endif
#define CG_FRAMEWORK
#defer HAVE_CG $[or $[CG_FRAMEWORK],$[libtest $[CG_LPATH],$[CG_LIBS]]]

// Is CgGL installed, and where?
#defer CGGL_IPATH $[CG_IPATH]
#defer CGGL_LPATH $[CG_LPATH]
#define CGGL_LIBS $[if $[WINDOWS_PLATFORM],cgGL.lib,CgGL]
#defer HAVE_CGGL $[or $[CGGL_FRAMEWORK],$[and $[HAVE_CG],$[libtest $[CGGL_LPATH],$[CGGL_LIBS]]]]

// Is CgDX8 installed, and where?
#defer CGDX8_IPATH $[CG_IPATH]
#defer CGDX8_LPATH $[CG_LPATH]
#define CGDX8_LIBS $[if $[WINDOWS_PLATFORM],cgD3D8.lib,CgDX8]
#defer HAVE_CGDX8 $[and $[HAVE_CG],$[libtest $[CGDX8_LPATH],$[CGDX8_LIBS]]]

// Is CgDX9 installed, and where?
#defer CGDX9_IPATH $[CG_IPATH]
#defer CGDX9_LPATH $[CG_LPATH]
#define CGDX9_LIBS $[if $[WINDOWS_PLATFORM],cgD3D9.lib,CgDX9]
#defer HAVE_CGDX9 $[and $[HAVE_CG],$[libtest $[CGDX9_LPATH],$[CGDX9_LIBS]]]

// Is CgDX10 installed, and where?
#defer CGDX10_IPATH $[CG_IPATH]
#defer CGDX10_LPATH $[CG_LPATH]
#define CGDX10_LIBS $[if $[WINDOWS_PLATFORM],cgD3D10.lib,CgDX10]
#defer HAVE_CGDX10 $[and $[HAVE_CG],$[libtest $[CGDX10_LPATH],$[CGDX10_LIBS]]]

// Is VRPN installed, and where?
#define VRPN_IPATH
#define VRPN_LPATH
#define VRPN_LIBS
#defer HAVE_VRPN $[libtest $[VRPN_LPATH],$[VRPN_LIBS]]

// Is HELIX installed, and where?
#define HELIX_IPATH
#define HELIX_LPATH
#define HELIX_LIBS
#defer HAVE_HELIX $[libtest $[HELIX_LPATH],$[HELIX_LIBS]]

// Is ZLIB installed, and where?
#define ZLIB_IPATH
#define ZLIB_LPATH
#define ZLIB_LIBS z
#defer HAVE_ZLIB $[libtest $[ZLIB_LPATH],$[ZLIB_LIBS]]

// Is OpenGL installed, and where?  This should include libGL as well
// as libGLU, if they are in different places.
#defer GL_IPATH /usr/include
#defer GL_LPATH
#defer GL_LIBS
#defer GLU_LIBS
#if $[WINDOWS_PLATFORM]
  #define GL_LIBS opengl32.lib
  #define GLU_LIBS glu32.lib
#elif $[OSX_PLATFORM]
  #defer GL_FRAMEWORK OpenGL
#else
  #defer GL_LPATH /usr/X11R6/lib
  #defer GL_LIBS GL
  #defer GLU_LIBS GLU
#endif
#defer HAVE_GL $[libtest $[GL_LPATH],$[GL_LIBS]]

// GLU is an auxiliary library that is usually provided with OpenGL,
// but is sometimes missing (e.g. the default FC5 installation).
#defer HAVE_GLU $[libtest $[GL_LPATH],$[GLU_LIBS]]

// If you are having trouble linking in OpenGL extension functions at
// runtime for some reason, you can set this variable.  This defines
// the minimum runtime version of OpenGL that Panda will require.
// Setting it to a higher version will compile in hard references to
// the extension functions provided by that OpenGL version and below,
// which may reduce runtime portability to other systems, but it will
// avoid issues with getting extension function pointers.  It also, of
// course, requires you to install the OpenGL header files and
// compile-time libraries appropriate to the version you want to
// compile against.

// The variable is the major, minor version of OpenGL, separated by a
// space (instead of a dot).  Thus, "1 1" means OpenGL version 1.1.
#define MIN_GL_VERSION 1 1

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

// Similar to MIN_GL_VERSION, above.
#define MIN_MESA_VERSION 1 1

// Do you want to build tinydisplay, a light and fast software
// renderer built into Panda, based on TinyGL?  This isn't as
// full-featured as Mesa, but it is many times faster, and in fact
// competes favorably with hardware-accelerated integrated graphics
// cards for raw speed (though the hardware-accelerated output looks
// better).
#define HAVE_TINYDISPLAY 1

// The SDL library is useful only for tinydisplay, and is not even
// required for that, as tinydisplay is also supported natively on
// each supported platform.
#define SDL_IPATH
#define SDL_LPATH
#define SDL_LIBS
#defer HAVE_SDL $[libtest $[SDL_LPATH],$[SDL_LIBS]]

// X11 may need to be linked against for tinydisplay, but probably
// only on a Linux platform.
#define X11_IPATH
#define X11_LPATH /usr/X11R6/lib
#define X11_LIBS X11
#defer HAVE_X11 $[and $[IS_LINUX],$[libtest $[X11_LPATH],$[X11_LIBS]]]

// How about GLX?
#define GLX_IPATH
#define GLX_LPATH
#defer HAVE_GLX $[and $[HAVE_GL],$[UNIX_PLATFORM]]

// glXGetProcAddress() is the function used to query OpenGL extensions
// under X.  However, this function is itself an extension function,
// leading to a chicken-and-egg problem.  One approach is to compile
// in a hard reference to the function, another is to pull the
// function address from the dynamic runtime.  Each has its share of
// problems.  Panda's default behavior is to pull it from the dynamic
// runtime; define this to compile in a reference to the function.
// This is only relevant from platforms using OpenGL under X (for
// instance, Linux).
#define LINK_IN_GLXGETPROCADDRESS

// Should we try to build the WGL interface?
#defer HAVE_WGL $[and $[HAVE_GL],$[WINDOWS_PLATFORM]]

// Is DirectX8 available, and should we try to build with it?
#define DX8_IPATH
#define DX8_LPATH
#define DX8_LIBS d3d8.lib d3dx8.lib dxerr8.lib
#defer HAVE_DX8 $[libtest $[DX8_LPATH],$[DX8_LIBS]]

// Is DirectX9 available, and should we try to build with it?
#define DX9_IPATH
#define DX9_LPATH
#define DX9_LIBS d3d9.lib d3dx9.lib dxerr9.lib
#defer HAVE_DX9 $[libtest $[DX9_LPATH],$[DX9_LIBS]]

// Is OpenCV installed, and where?
#define OPENCV_IPATH /usr/local/include/opencv
#define OPENCV_LPATH /usr/local/lib
#define OPENCV_LIBS $[if $[WINDOWS_PLATFORM],cv.lib highgui.lib cxcore.lib,cv highgui cxcore]
#defer HAVE_OPENCV $[libtest $[OPENCV_LPATH],$[OPENCV_LIBS]]

// Is FFMPEG installed, and where?
#define FFMPEG_IPATH /usr/include/ffmpeg
#define FFMPEG_LPATH
#define FFMPEG_LIBS $[if $[WINDOWS_PLATFORM],libavcodec.lib libavformat.lib libavutil.lib libgcc.lib,avcodec avformat avutil]
#defer HAVE_FFMPEG $[libtest $[FFMPEG_LPATH],$[FFMPEG_LIBS]]

// Is ODE installed, and where?
#define ODE_IPATH
#define ODE_LPATH
#define ODE_LIBS $[if $[WINDOWS_PLATFORM],ode.lib,ode]
#defer HAVE_ODE $[libtest $[ODE_LPATH],$[ODE_LIBS]]

// Do you want to build the DirectD tools for starting Panda clients
// remotely?  This only affects the direct tree.  Enabling this may
// cause libdirect.dll to fail to load on Win98 clients.
#define HAVE_DIRECTD

// If your system supports the Posix threads interface
// (pthread_create(), etc.), define this true.
#define HAVE_POSIX_THREADS $[and $[isfile /usr/include/pthread.h],$[not $[WINDOWS_PLATFORM]]]

// Do you want to build in support for threading (multiprocessing)?
// Building in support for threading will enable Panda to take
// advantage of multiple CPU's if you have them (and if the OS
// supports kernel threads running on different CPU's), but it will
// slightly slow down Panda for the single CPU case, so this is not
// enabled by default.
#define HAVE_THREADS
#define THREADS_LIBS $[if $[not $[WINDOWS_PLATFORM]],pthread]

// If you have enabled threading support with HAVE_THREADS, the
// default is to use OS-provided threading constructs, which usually
// allows for full multiprogramming support (i.e. the program can take
// advantage of multiple CPU's).  On the other hand, compiling in this
// full OS-provided support can impose some substantial runtime
// overhead, making the application run slower on a single-CPU
// machine.  To avoid this overhead, but still gain some of the basic
// functionality of threads (such as support for asynchronous model
// loads), define SIMPLE_THREADS true in addition to HAVE_THREADS.
// This will compile in a homespun cooperative threading
// implementation that runs strictly on one CPU, adding very little
// overhead over plain single-threaded code.
#define SIMPLE_THREADS

// Whether threading is defined or not, you might want to validate the
// thread and synchronization operations.  With threading enabled,
// defining this will also enable deadlock detection and logging.
// Without threading enabled, defining this will simply verify that a
// mutex is not recursively locked.  There is, of course, additional
// run-time overhead for these tests.
#defer DEBUG_THREADS $[<= $[OPTIMIZE], 2]

// Define this true to implement mutexes and condition variables via
// user-space spinlocks, instead of via OS-provided constructs.  This
// is almost never a good idea, except possibly in very specialized
// cases when you are building Panda for a particular application, on
// a particular platform, and you are sure you won't have more threads
// than CPU's.  Even then, OS-based locking is probably better.
#define MUTEX_SPINLOCK

// Define this to use the PandaFileStream interface for pifstream,
// pofstream, and pfstream.  This is a customized file buffer that may
// have slightly better newline handling, but its primary benefit is
// that it supports SIMPLE_THREADS better by blocking just the active
// "thread" when I/O is delayed, instead of blocking the entire
// process.  Normally, there's no reason to turn this off, unless you
// suspect a bug in Panda.
#define USE_PANDAFILESTREAM 1

// Do you want to build the PStats interface, for graphical run-time
// performance statistics?  This requires NET to be available.  By
// default, we don't build PStats when OPTIMIZE = 4, although this is
// possible.
#defer DO_PSTATS $[or $[and $[HAVE_NET],$[< $[OPTIMIZE], 4]], $[DO_PSTATS]]

// Do you want to type-check downcasts?  This is a good idea during
// development, but does impose some run-time overhead.
#defer DO_DCAST $[< $[OPTIMIZE], 3]

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

// The Tau profiler provides a multiplatform, thread-aware profiler.
// To use it, define USE_TAU to 1, and set TAU_MAKEFILE to the
// filename that contains the Tau-provided Makefile for your platform.
// Then rebuild the code with ppremake; make install.  Alternatively,
// instead of setting TAU_MAKEFILE, you can also define TAU_ROOT and
// PDT_ROOT, to point to the root directory of the tau and pdtoolkit
// installations, respectively; then the individual Tau components
// will be invoked directly.  This is especially useful on Windows,
// where there is no Tau Makefile.
#define TAU_MAKEFILE
#define TAU_ROOT
#define PDT_ROOT
#define TAU_OPTS -optKeepFiles
#define TAU_CFLAGS -D_GNU_SOURCE
#define USE_TAU

// Info for the RAD game tools, Miles Sound System
// note this may be overwritten in wintools Config.pp
#define RAD_MSS_IPATH /usr/include/Miles6/include
#define RAD_MSS_LPATH /usr/lib/Miles6/lib/win
#define RAD_MSS_LIBS Mss32
#defer HAVE_RAD_MSS $[libtest $[RAD_MSS_LPATH],$[RAD_MSS_LIBS]]

// Info for the Fmod audio engine
#define FMODEX_IPATH /usr/local/fmod/api/inc
#define FMODEX_LPATH /usr/local/fmod/api/lib
#define FMODEX_LIBS $[if $[libtest $[FMODEX_LPATH],fmodex64],fmodex64,fmodex]
#defer HAVE_FMODEX $[libtest $[FMODEX_LPATH],$[FMODEX_LIBS]]

// Info for the Ageia PhysX SDK
#define PHYSX_IPATH
#define PHYSX_LPATH
#define PHYSX_LIBS PhysXLoader.lib NxCharacter.lib NxCooking.lib NxExtensions.lib
#defer HAVE_PHYSX $[libtest $[PHYSX_LPATH],$[PHYSX_LIBS]]

// Info for TinyXML library
#define TINYXML_IPATH
#define TINYXML_LPATH
#define TINYXML_LIBS $[if $[WINDOWS_PLATFORM],tinyxml.lib,tinyxml]
#defer HAVE_TINYXML $[libtest $[TINYXML_LPATH],$[TINYXML_LIBS]]

// Info for http://www.sourceforge.net/projects/chromium
#define CHROMIUM_IPATH /usr/include/chromium/include
#define CHROMIUM_LPATH /usr/lib/chromium/bin/WINT_NT
#define CHROMIUM_LIBS spuload
#defer HAVE_CHROMIUM $[libtest $[CHROMIUM_LPATH],$[CHROMIUM_LIBS]]

// Is gtk+-2 installed?  This is only needed to build the pstats
// program on Unix (or non-Windows) platforms.
#define PKG_CONFIG pkg-config
#define HAVE_GTK

// Do we have Freetype 2.0 (or better)?  If available, this package is
// used to generate dynamic in-the-world text from font files.

// On Unix, freetype comes with the freetype-config executable, which
// tells us where to look for the various files.  On Windows, we need to
// supply this information explicitly.
#defer FREETYPE_CONFIG $[if $[not $[WINDOWS_PLATFORM]],freetype-config]
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
#defer MAYA_LIBS $[if $[WINDOWS_PLATFORM],Foundation.lib OpenMaya.lib OpenMayaAnim.lib OpenMayaUI.lib,Foundation OpenMaya OpenMayaAnim OpenMayaUI]
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

// Is FCollada installed? This is for the daeegg converter.
#define FCOLLADA_IPATH /usr/local/include/fcollada
#define FCOLLADA_LPATH /usr/local/lib
#define FCOLLADA_LIBS FColladaSD
#defer HAVE_FCOLLADA $[libtest $[FCOLLADA_LPATH],$[FCOLLADA_LIBS]]

// Also for the ARToolKit library, for augmented reality
#define ARTOOLKIT_IPATH
#define ARTOOLKIT_LPATH
#define ARTOOLKIT_LIBS $[if $[WINDOWS_PLATFORM],libAR.lib,AR]
#defer HAVE_ARTOOLKIT $[libtest $[ARTOOLKIT_LPATH],$[ARTOOLKIT_LIBS]]

// Define this to generate static libraries and executables, rather than
// dynamic libraries.
//#define LINK_ALL_STATIC yes

// Define this to export the templates from the DLL.  This is only
// meaningful if LINK_ALL_STATIC is not defined, and we are building
// on Windows.  Some Windows compilers may not support this syntax.
#defer EXPORT_TEMPLATES yes

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
#elif $[OSX_PLATFORM]
  #define USE_COMPILER GCC
#elif $[eq $[PLATFORM], FreeBSD]
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


//////////////////////////////////////////////////////////////////////
// There are also some additional variables that control specific
// compiler/platform features or characteristics, defined in the
// platform specific file Config.platform.pp.  Be sure to inspect
// these variables for correctness too.
//////////////////////////////////////////////////////////////////////
