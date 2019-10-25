""" This module contains code to freeze a number of Python modules
into a single (mostly) standalone DLL or EXE. """

import modulefinder
import sys
import os
import marshal
import imp
import platform
import struct
import io
import distutils.sysconfig as sysconf
import zipfile

from . import pefile

# Temporary (?) try..except to protect against unbuilt p3extend_frozen.
try:
    import p3extend_frozen
except ImportError:
    p3extend_frozen = None

from panda3d.core import *

# Check to see if we are running python_d, which implies we have a
# debug build, and we have to build the module with debug options.
# This is only relevant on Windows.

# I wonder if there's a better way to determine this?
python = os.path.splitext(os.path.split(sys.executable)[1])[0]
isDebugBuild = (python.lower().endswith('_d'))

# These are modules that Python always tries to import up-front.  They
# must be frozen in any main.exe.
# NB. if encodings are removed, be sure to remove them from the shortcut in
# deploy-stub.c.
startupModules = [
    'imp', 'encodings', 'encodings.*',
]
if sys.version_info >= (3, 0):
    # Modules specific to Python 3
    startupModules += ['io', 'marshal', 'importlib.machinery', 'importlib.util']
else:
    # Modules specific to Python 2
    startupModules += []

# These are some special init functions for some built-in Python modules that
# deviate from the standard naming convention.  A value of None means that a
# dummy entry should be written to the inittab.
builtinInitFuncs = {
    'builtins': None,
    '__builtin__': None,
    'sys': None,
    'exceptions': None,
    '_warnings': '_PyWarnings_Init',
    'marshal': 'PyMarshal_Init',
}
if sys.version_info < (3, 7):
    builtinInitFuncs['_imp'] = 'PyInit_imp'

# These are modules that are not found normally for these modules. Add them
# to an include list so users do not have to do this manually.
try:
    from pytest import freeze_includes as pytest_imports
except ImportError:
    def pytest_imports():
        return []

hiddenImports = {
    'pytest': pytest_imports(),
    'pkg_resources': [
        'pkg_resources.*.*',
    ],
    'xml.etree.cElementTree': ['xml.etree.ElementTree'],
    'datetime': ['_strptime'],
    'keyring.backends': ['keyring.backends.*'],
    'matplotlib.font_manager': ['encodings.mac_roman'],
    'direct.particles': ['direct.particles.ParticleManagerGlobal'],
    'numpy.core._multiarray_umath': [
        'numpy.core._internal',
        'numpy.core._dtype_ctypes',
        'numpy.core._methods',
    ],
}

if sys.version_info >= (3,):
    hiddenImports['matplotlib.backends._backend_tk'] = ['tkinter']
else:
    hiddenImports['matplotlib.backends._backend_tk'] = ['Tkinter']


# These are overrides for specific modules.
overrideModules = {
    # Used by the warnings module, among others, to get line numbers.  Since
    # we set __file__, this would cause it to try and extract Python code
    # lines from the main executable, which we don't want.
    'linecache': """__all__ = ["getline", "clearcache", "checkcache"]

cache = {}

def getline(filename, lineno, module_globals=None):
    return ''

def clearcache():
    global cache
    cache = {}

def getlines(filename, module_globals=None):
    return []

def checkcache(filename=None):
    pass

def updatecache(filename, module_globals=None):
    pass

def lazycache(filename, module_globals):
    pass
""",
}

# These are missing modules that we've reported already this session.
reportedMissing = {}


class CompilationEnvironment:
    """ Create an instance of this class to record the commands to
    invoke the compiler on a given platform.  If needed, the caller
    can create a custom instance of this class (or simply set the
    compile strings directly) to customize the build environment. """

    def __init__(self, platform):
        self.platform = platform

        # The command to compile a c to an object file.  Replace %(basename)s
        # with the basename of the source file, and an implicit .c extension.
        self.compileObj = 'error'

        # The command to link a single object file into an executable.  As
        # above, replace $(basename)s with the basename of the original source
        # file, and of the target executable.
        self.linkExe = 'error'

        # The command to link a single object file into a shared library.
        self.linkDll = 'error'

        # Paths to Python stuff.
        self.Python = None
        self.PythonIPath = sysconf.get_python_inc()
        self.PythonVersion = sysconf.get_config_var("LDVERSION") or sysconf.get_python_version()

        # The VC directory of Microsoft Visual Studio (if relevant)
        self.MSVC = None
        # Directory to Windows Platform SDK (if relevant)
        self.PSDK = None

        # The setting to control release vs. debug builds.  Only relevant on
        # Windows.
        self.MD = None

        # Added to the path to the MSVC bin and lib directories on 64-bits Windows.
        self.suffix64 = ''

        # The _d extension to add to dll filenames on Windows in debug builds.
        self.dllext = ''

        # Any architecture-specific string.
        self.arch = ''

        self.determineStandardSetup()

    def determineStandardSetup(self):
        if self.platform.startswith('win'):
            self.Python = sysconf.PREFIX

            if ('VCINSTALLDIR' in os.environ):
                self.MSVC = os.environ['VCINSTALLDIR']
            elif (Filename('/c/Program Files/Microsoft Visual Studio 9.0/VC').exists()):
                self.MSVC = Filename('/c/Program Files/Microsoft Visual Studio 9.0/VC').toOsSpecific()
            elif (Filename('/c/Program Files (x86)/Microsoft Visual Studio 9.0/VC').exists()):
                self.MSVC = Filename('/c/Program Files (x86)/Microsoft Visual Studio 9.0/VC').toOsSpecific()
            elif (Filename('/c/Program Files/Microsoft Visual Studio .NET 2003/Vc7').exists()):
                self.MSVC = Filename('/c/Program Files/Microsoft Visual Studio .NET 2003/Vc7').toOsSpecific()
            else:
                print('Could not locate Microsoft Visual C++ Compiler! Try running from the Visual Studio Command Prompt.')
                sys.exit(1)

            if ('WindowsSdkDir' in os.environ):
                self.PSDK = os.environ['WindowsSdkDir']
            elif (platform.architecture()[0] == '32bit' and Filename('/c/Program Files/Microsoft Platform SDK for Windows Server 2003 R2').exists()):
                self.PSDK = Filename('/c/Program Files/Microsoft Platform SDK for Windows Server 2003 R2').toOsSpecific()
            elif (os.path.exists(os.path.join(self.MSVC, 'PlatformSDK'))):
                self.PSDK = os.path.join(self.MSVC, 'PlatformSDK')
            else:
                print('Could not locate the Microsoft Windows Platform SDK! Try running from the Visual Studio Command Prompt.')
                sys.exit(1)

            # We need to use the correct compiler setting for debug vs. release builds.
            self.MD = '/MD'
            if isDebugBuild:
                self.MD = '/MDd'
                self.dllext = '_d'

            # MSVC/bin and /lib directories have a different location
            # for win64.
            if self.platform == 'win_amd64':
                self.suffix64 = '\\amd64'

            # If it is run by makepanda, it handles the MSVC and PlatformSDK paths itself.
            if ('MAKEPANDA' in os.environ):
                self.compileObjExe = 'cl /wd4996 /Fo%(basename)s.obj /nologo /c %(MD)s /Zi /O2 /Ob2 /EHsc /Zm300 /W3 /I"%(pythonIPath)s" %(filename)s'
                self.compileObjDll = self.compileObjExe
                self.linkExe = 'link /nologo /MAP:NUL /FIXED:NO /OPT:REF /STACK:4194304 /INCREMENTAL:NO /LIBPATH:"%(python)s\\libs"  /out:%(basename)s.exe %(basename)s.obj'
                self.linkDll = 'link /nologo /DLL /MAP:NUL /FIXED:NO /OPT:REF /INCREMENTAL:NO /LIBPATH:"%(python)s\\libs"  /out:%(basename)s%(dllext)s.pyd %(basename)s.obj'
            else:
                os.environ['PATH'] += ';' + self.MSVC + '\\bin' + self.suffix64 + ';' + self.MSVC + '\\Common7\\IDE;' + self.PSDK + '\\bin'

                self.compileObjExe = 'cl /wd4996 /Fo%(basename)s.obj /nologo /c %(MD)s /Zi /O2 /Ob2 /EHsc /Zm300 /W3 /I"%(pythonIPath)s" /I"%(PSDK)s\\include" /I"%(MSVC)s\\include" %(filename)s'
                self.compileObjDll = self.compileObjExe
                self.linkExe = 'link /nologo /MAP:NUL /FIXED:NO /OPT:REF /STACK:4194304 /INCREMENTAL:NO /LIBPATH:"%(PSDK)s\\lib" /LIBPATH:"%(MSVC)s\\lib%(suffix64)s" /LIBPATH:"%(python)s\\libs"  /out:%(basename)s.exe %(basename)s.obj'
                self.linkDll = 'link /nologo /DLL /MAP:NUL /FIXED:NO /OPT:REF /INCREMENTAL:NO /LIBPATH:"%(PSDK)s\\lib" /LIBPATH:"%(MSVC)s\\lib%(suffix64)s" /LIBPATH:"%(python)s\\libs"  /out:%(basename)s%(dllext)s.pyd %(basename)s.obj'

        elif self.platform.startswith('osx_'):
            # OSX
            proc = self.platform.split('_', 1)[1]
            if proc == 'i386':
                self.arch = '-arch i386'
            elif proc == 'ppc':
                self.arch = '-arch ppc'
            elif proc == 'amd64':
                self.arch = '-arch x86_64'
            self.compileObjExe = "gcc -c %(arch)s -o %(basename)s.o -O2 -I%(pythonIPath)s %(filename)s"
            self.compileObjDll = "gcc -fPIC -c %(arch)s -o %(basename)s.o -O2 -I%(pythonIPath)s %(filename)s"
            self.linkExe = "gcc %(arch)s -o %(basename)s %(basename)s.o -framework Python"
            self.linkDll = "gcc %(arch)s -undefined dynamic_lookup -bundle -o %(basename)s.so %(basename)s.o"

        else:
            # Unix
            lib_dir = sysconf.get_python_lib(plat_specific=1, standard_lib=1)
            #python_a = os.path.join(lib_dir, "config", "libpython%(pythonVersion)s.a")
            self.compileObjExe = "%(CC)s %(CFLAGS)s -c -o %(basename)s.o -pthread -O2 %(filename)s -I%(pythonIPath)s"
            self.compileObjDll = "%(CC)s %(CFLAGS)s %(CCSHARED)s -c -o %(basename)s.o -O2 %(filename)s -I%(pythonIPath)s"
            self.linkExe = "%(CC)s -o %(basename)s %(basename)s.o -L/usr/local/lib -lpython%(pythonVersion)s"
            self.linkDll = "%(LDSHARED)s -o %(basename)s.so %(basename)s.o -L/usr/local/lib -lpython%(pythonVersion)s"

            if (os.path.isdir("/usr/PCBSD/local/lib")):
                self.linkExe += " -L/usr/PCBSD/local/lib"
                self.linkDll += " -L/usr/PCBSD/local/lib"

    def compileExe(self, filename, basename, extraLink=[]):
        compile = self.compileObjExe % dict({
            'python' : self.Python,
            'MSVC' : self.MSVC,
            'PSDK' : self.PSDK,
            'suffix64' : self.suffix64,
            'MD' : self.MD,
            'pythonIPath' : self.PythonIPath,
            'pythonVersion' : self.PythonVersion,
            'arch' : self.arch,
            'filename' : filename,
            'basename' : basename,
            }, **sysconf.get_config_vars())
        sys.stderr.write(compile + '\n')
        if os.system(compile) != 0:
            raise Exception('failed to compile %s.' % basename)

        link = self.linkExe % dict({
            'python' : self.Python,
            'MSVC' : self.MSVC,
            'PSDK' : self.PSDK,
            'suffix64' : self.suffix64,
            'pythonIPath' : self.PythonIPath,
            'pythonVersion' : self.PythonVersion,
            'arch' : self.arch,
            'filename' : filename,
            'basename' : basename,
            }, **sysconf.get_config_vars())
        link += ' ' + ' '.join(extraLink)
        sys.stderr.write(link + '\n')
        if os.system(link) != 0:
            raise Exception('failed to link %s.' % basename)

    def compileDll(self, filename, basename, extraLink=[]):
        compile = self.compileObjDll % dict({
            'python' : self.Python,
            'MSVC' : self.MSVC,
            'PSDK' : self.PSDK,
            'suffix64' : self.suffix64,
            'MD' : self.MD,
            'pythonIPath' : self.PythonIPath,
            'pythonVersion' : self.PythonVersion,
            'arch' : self.arch,
            'filename' : filename,
            'basename' : basename,
            }, **sysconf.get_config_vars())
        sys.stderr.write(compile + '\n')
        if os.system(compile) != 0:
            raise Exception('failed to compile %s.' % basename)

        link = self.linkDll % dict({
            'python' : self.Python,
            'MSVC' : self.MSVC,
            'PSDK' : self.PSDK,
            'suffix64' : self.suffix64,
            'pythonIPath' : self.PythonIPath,
            'pythonVersion' : self.PythonVersion,
            'arch' : self.arch,
            'filename' : filename,
            'basename' : basename,
            'dllext' : self.dllext,
            }, **sysconf.get_config_vars())
        link += ' ' + ' '.join(extraLink)
        sys.stderr.write(link + '\n')
        if os.system(link) != 0:
            raise Exception('failed to link %s.' % basename)

# The code from frozenmain.c in the Python source repository.
frozenMainCode = """
/* Python interpreter main program for frozen scripts */

#include <Python.h>

#if PY_MAJOR_VERSION >= 3
#include <locale.h>

#if PY_MINOR_VERSION < 5
#define Py_DecodeLocale _Py_char2wchar
#endif
#endif

#ifdef MS_WINDOWS
extern void PyWinFreeze_ExeInit(void);
extern void PyWinFreeze_ExeTerm(void);

extern PyAPI_FUNC(int) PyImport_ExtendInittab(struct _inittab *newtab);
#endif

/* Main program */

int
Py_FrozenMain(int argc, char **argv)
{
    char *p;
    int n, sts = 1;
    int inspect = 0;
    int unbuffered = 0;

#if PY_MAJOR_VERSION >= 3
    int i;
    char *oldloc;
    wchar_t **argv_copy = NULL;
    /* We need a second copies, as Python might modify the first one. */
    wchar_t **argv_copy2 = NULL;

    if (argc > 0) {
        argv_copy = (wchar_t **)alloca(sizeof(wchar_t *) * argc);
        argv_copy2 = (wchar_t **)alloca(sizeof(wchar_t *) * argc);
    }
#endif

    Py_FrozenFlag = 1; /* Suppress errors from getpath.c */
    Py_NoSiteFlag = 1;
    Py_NoUserSiteDirectory = 1;

    if ((p = Py_GETENV("PYTHONINSPECT")) && *p != '\\0')
        inspect = 1;
    if ((p = Py_GETENV("PYTHONUNBUFFERED")) && *p != '\\0')
        unbuffered = 1;

    if (unbuffered) {
        setbuf(stdin, (char *)NULL);
        setbuf(stdout, (char *)NULL);
        setbuf(stderr, (char *)NULL);
    }

#if PY_MAJOR_VERSION >= 3
    oldloc = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, \"\");
    for (i = 0; i < argc; i++) {
        argv_copy[i] = Py_DecodeLocale(argv[i], NULL);
        argv_copy2[i] = argv_copy[i];
        if (!argv_copy[i]) {
            fprintf(stderr, \"Unable to decode the command line argument #%i\\n\",
                            i + 1);
            argc = i;
            goto error;
        }
    }
    setlocale(LC_ALL, oldloc);
#endif

#ifdef MS_WINDOWS
    PyImport_ExtendInittab(extensions);
#endif /* MS_WINDOWS */

    if (argc >= 1) {
#if PY_MAJOR_VERSION >= 3
        Py_SetProgramName(argv_copy[0]);
#else
        Py_SetProgramName(argv[0]);
#endif
    }

    Py_Initialize();
#ifdef MS_WINDOWS
    PyWinFreeze_ExeInit();
#endif

    if (Py_VerboseFlag)
        fprintf(stderr, "Python %s\\n%s\\n",
            Py_GetVersion(), Py_GetCopyright());

#if PY_MAJOR_VERSION >= 3
    PySys_SetArgv(argc, argv_copy);
#else
    PySys_SetArgv(argc, argv);
#endif

    n = PyImport_ImportFrozenModule("__main__");
    if (n == 0)
        Py_FatalError("__main__ not frozen");
    if (n < 0) {
        PyErr_Print();
        sts = 1;
    }
    else
        sts = 0;

    if (inspect && isatty((int)fileno(stdin)))
        sts = PyRun_AnyFile(stdin, "<stdin>") != 0;

#ifdef MS_WINDOWS
    PyWinFreeze_ExeTerm();
#endif
    Py_Finalize();

#if PY_MAJOR_VERSION >= 3
error:
    if (argv_copy2) {
        for (i = 0; i < argc; i++) {
#if PY_MINOR_VERSION >= 4
            PyMem_RawFree(argv_copy2[i]);
#else
            PyMem_Free(argv_copy2[i]);
#endif
        }
    }
#endif
    return sts;
}
"""

# The code from frozen_dllmain.c in the Python source repository.
# Windows only.
frozenDllMainCode = """
#include <windows.h>

static char *possibleModules[] = {
    "pywintypes",
    "pythoncom",
    "win32ui",
    NULL,
};

BOOL CallModuleDllMain(char *modName, DWORD dwReason);


/*
  Called by a frozen .EXE only, so that built-in extension
  modules are initialized correctly
*/
void PyWinFreeze_ExeInit(void)
{
    char **modName;
    for (modName = possibleModules;*modName;*modName++) {
/*      printf("Initialising '%s'\\n", *modName); */
        CallModuleDllMain(*modName, DLL_PROCESS_ATTACH);
    }
}

/*
  Called by a frozen .EXE only, so that built-in extension
  modules are cleaned up
*/
void PyWinFreeze_ExeTerm(void)
{
    // Must go backwards
    char **modName;
    for (modName = possibleModules+(sizeof(possibleModules) / sizeof(char *))-2;
         modName >= possibleModules;
         *modName--) {
/*      printf("Terminating '%s'\\n", *modName);*/
        CallModuleDllMain(*modName, DLL_PROCESS_DETACH);
    }
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    BOOL ret = TRUE;
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
        {
            char **modName;
            for (modName = possibleModules;*modName;*modName++) {
                BOOL ok = CallModuleDllMain(*modName, dwReason);
                if (!ok)
                    ret = FALSE;
            }
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            // Must go backwards
            char **modName;
            for (modName = possibleModules+(sizeof(possibleModules) / sizeof(char *))-2;
                 modName >= possibleModules;
                 *modName--)
                CallModuleDllMain(*modName, DLL_PROCESS_DETACH);
            break;
        }
    }
    return ret;
}

BOOL CallModuleDllMain(char *modName, DWORD dwReason)
{
    BOOL (WINAPI * pfndllmain)(HINSTANCE, DWORD, LPVOID);

    char funcName[255];
    HMODULE hmod = GetModuleHandle(NULL);
    strcpy(funcName, "_DllMain");
    strcat(funcName, modName);
    strcat(funcName, "@12"); // stdcall convention.
    pfndllmain = (BOOL (WINAPI *)(HINSTANCE, DWORD, LPVOID))GetProcAddress(hmod, funcName);
    if (pfndllmain==NULL) {
        /* No function by that name exported - then that module does
           not appear in our frozen program - return OK
                */
        return TRUE;
    }
    return (*pfndllmain)(hmod, dwReason, NULL);
}
"""

# Our own glue code to start up a Python executable.
mainInitCode = """
%(frozenMainCode)s

int
main(int argc, char *argv[]) {
  PyImport_FrozenModules = _PyImport_FrozenModules;
  return Py_FrozenMain(argc, argv);
}
"""

# Our own glue code to start up a Python shared library.
dllInitCode = """
/*
 * Call this function to extend the frozen modules array with a new
 * array of frozen modules, provided in a C-style array, at runtime.
 * Returns the total number of frozen modules.
 */
static int
extend_frozen_modules(const struct _frozen *new_modules, int new_count) {
  int orig_count;
  struct _frozen *realloc_FrozenModules;

  /* First, count the number of frozen modules we had originally. */
  orig_count = 0;
  while (PyImport_FrozenModules[orig_count].name != NULL) {
    ++orig_count;
  }

  if (new_count == 0) {
    /* Trivial no-op. */
    return orig_count;
  }

  /* Reallocate the PyImport_FrozenModules array bigger to make room
     for the additional frozen modules.  We just leak the original
     array; it's too risky to try to free it. */
  realloc_FrozenModules = (struct _frozen *)malloc((orig_count + new_count + 1) * sizeof(struct _frozen));

  /* The new frozen modules go at the front of the list. */
  memcpy(realloc_FrozenModules, new_modules, new_count * sizeof(struct _frozen));

  /* Then the original set of frozen modules. */
  memcpy(realloc_FrozenModules + new_count, PyImport_FrozenModules, orig_count * sizeof(struct _frozen));

  /* Finally, a single 0-valued entry marks the end of the array. */
  memset(realloc_FrozenModules + orig_count + new_count, 0, sizeof(struct _frozen));

  /* Assign the new pointer. */
  PyImport_FrozenModules = realloc_FrozenModules;

  return orig_count + new_count;
}

#if PY_MAJOR_VERSION >= 3
static PyModuleDef mdef = {
  PyModuleDef_HEAD_INIT,
  "%(moduleName)s",
  "",
  -1,
  NULL, NULL, NULL, NULL, NULL
};

%(dllexport)sPyObject *PyInit_%(moduleName)s(void) {
  extend_frozen_modules(_PyImport_FrozenModules, sizeof(_PyImport_FrozenModules) / sizeof(struct _frozen));
  return PyModule_Create(&mdef);
}
#else
static PyMethodDef nullMethods[] = {
  {NULL, NULL}
};

%(dllexport)svoid init%(moduleName)s(void) {
  extend_frozen_modules(_PyImport_FrozenModules, sizeof(_PyImport_FrozenModules) / sizeof(struct _frozen));
  Py_InitModule("%(moduleName)s", nullMethods);
}
#endif
"""

programFile = """
#include <Python.h>
#ifdef _WIN32
#include <malloc.h>
#endif

%(moduleDefs)s

struct _frozen _PyImport_FrozenModules[] = {
%(moduleList)s
  {NULL, NULL, 0}
};
"""


okMissing = [
    '__main__', '_dummy_threading', 'Carbon', 'Carbon.Files',
    'Carbon.Folder', 'Carbon.Folders', 'HouseGlobals', 'Carbon.File',
    'MacOS', '_emx_link', 'ce', 'mac', 'org.python.core', 'os.path',
    'os2', 'posix', 'pwd', 'readline', 'riscos', 'riscosenviron',
    'riscospath', 'dbm', 'fcntl', 'win32api', 'win32pipe', 'usercustomize',
    '_winreg', 'winreg', 'ctypes', 'ctypes.wintypes', 'nt','msvcrt',
    'EasyDialogs', 'SOCKS', 'ic', 'rourl2path', 'termios', 'vms_lib',
    'OverrideFrom23._Res', 'email', 'email.Utils', 'email.Generator',
    'email.Iterators', '_subprocess', 'gestalt', 'java.lang',
    'direct.extensions_native.extensions_darwin',
    ]

class Freezer:
    class ModuleDef:
        def __init__(self, moduleName, filename = None,
                     implicit = False, guess = False,
                     exclude = False, forbid = False,
                     allowChildren = False, fromSource = None,
                     text = None):
            # The Python module name.
            self.moduleName = moduleName

            # The file on disk it was loaded from, if any.
            self.filename = filename
            if filename is not None and not isinstance(filename, Filename):
                self.filename = Filename(filename)

            # True if the module was found via the modulefinder.
            self.implicit = implicit

            # True if the moduleName might refer to some Python object
            # other than a module, in which case the module should be
            # ignored.
            self.guess = guess

            # True if the module should *not* be included in the
            # generated output.
            self.exclude = exclude

            # True if the module should never be allowed, even if it
            # exists at runtime.
            self.forbid = forbid

            # True if excluding the module still allows its children
            # to be included.  This only makes sense if the module
            # will exist at runtime through some other means
            # (e.g. from another package).
            self.allowChildren = allowChildren

            # Additional black-box information about where this module
            # record came from, supplied by the caller.
            self.fromSource = fromSource

            # If this is set, it contains Python code of the module.
            self.text = text

            # Some sanity checks.
            if not self.exclude:
                self.allowChildren = True

            if self.forbid:
                self.exclude = True
                self.allowChildren = False

        def __repr__(self):
            args = [repr(self.moduleName), repr(self.filename)]
            if self.implicit:
                args.append('implicit = True')
            if self.guess:
                args.append('guess = True')
            if self.exclude:
                args.append('exclude = True')
            if self.forbid:
                args.append('forbid = True')
            if self.allowChildren:
                args.append('allowChildren = True')
            return 'ModuleDef(%s)' % (', '.join(args))

    def __init__(self, previous = None, debugLevel = 0,
                 platform = None, path=None):
        # Normally, we are freezing for our own platform.  Change this
        # if untrue.
        self.platform = platform or PandaSystem.getPlatform()

        # This is the compilation environment.  Fill in your own
        # object here if you have custom needs (for instance, for a
        # cross-compiler or something).  If this is None, then a
        # default object will be created when it is needed.
        self.cenv = None

        # This is the search path to use for Python modules.  Leave it
        # to the default value of None to use sys.path.
        self.path = path

        # The filename extension to append to the source file before
        # compiling.
        self.sourceExtension = '.c'

        # The filename extension to append to the object file.
        self.objectExtension = '.o'
        if self.platform.startswith('win'):
            self.objectExtension = '.obj'

        self.keepTemporaryFiles = False

        # Change any of these to change the generated startup and glue
        # code.
        self.frozenMainCode = frozenMainCode
        self.frozenDllMainCode = frozenDllMainCode
        self.mainInitCode = mainInitCode

        # Set this true to encode Python files in a Multifile as their
        # original source if possible, or false to encode them as
        # compiled pyc or pyo files.  This has no effect on frozen exe
        # or dll's; those are always stored with compiled code.
        self.storePythonSource = False

        # This list will be filled in by generateCode() or
        # addToMultifile().  It contains a list of all the extension
        # modules that were discovered, which have not been added to
        # the output.  The list is a list of tuples of the form
        # (moduleName, filename).  filename will be None for built-in
        # modules.
        self.extras = []

        # Set this to true if extension modules should be linked in to
        # the resulting executable.
        self.linkExtensionModules = False

        # End of public interface.  These remaining members should not
        # be directly manipulated by callers.
        self.previousModules = {}
        self.modules = {}

        if previous:
            self.previousModules = dict(previous.modules)
            self.modules = dict(previous.modules)

        # Exclude doctest by default; it is not very useful in production
        # builds.  It can be explicitly included if desired.
        self.modules['doctest'] = self.ModuleDef('doctest', exclude = True)

        self.mf = None

        # Actually, make sure we know how to find all of the
        # already-imported modules.  (Some of them might do their own
        # special path mangling.)
        for moduleName, module in list(sys.modules.items()):
            if module and hasattr(module, '__path__'):
                path = list(getattr(module, '__path__'))
                if path:
                    modulefinder.AddPackagePath(moduleName, path[0])

        # Suffix/extension for Python C extension modules
        if self.platform == PandaSystem.getPlatform():
            self.moduleSuffixes = imp.get_suffixes()

            # Set extension for Python files to binary mode
            for i, suffix in enumerate(self.moduleSuffixes):
                if suffix[2] == imp.PY_SOURCE:
                    self.moduleSuffixes[i] = (suffix[0], 'rb', imp.PY_SOURCE)
        else:
            self.moduleSuffixes = [('.py', 'rb', 1), ('.pyc', 'rb', 2)]

            abi_version = '{0}{1}'.format(*sys.version_info)
            abi_flags = ''
            if sys.version_info < (3, 8):
                abi_flags += 'm'

            if 'linux' in self.platform:
                self.moduleSuffixes += [
                    ('.cpython-{0}{1}-x86_64-linux-gnu.so'.format(abi_version, abi_flags), 'rb', 3),
                    ('.cpython-{0}{1}-i686-linux-gnu.so'.format(abi_version, abi_flags), 'rb', 3),
                    ('.abi{0}.so'.format(sys.version_info[0]), 'rb', 3),
                    ('.so', 'rb', 3),
                ]
            elif 'win' in self.platform:
                # ABI flags are not appended on Windows.
                self.moduleSuffixes += [
                    ('.cp{0}-win_amd64.pyd'.format(abi_version), 'rb', 3),
                    ('.cp{0}-win32.pyd'.format(abi_version), 'rb', 3),
                    ('.pyd', 'rb', 3),
                ]
            elif 'mac' in self.platform:
                self.moduleSuffixes += [
                    ('.cpython-{0}{1}-darwin.so'.format(abi_version, abi_flags), 'rb', 3),
                    ('.abi{0}.so'.format(sys.version_info[0]), 'rb', 3),
                    ('.so', 'rb', 3),
                ]
            else: # FreeBSD et al.
                self.moduleSuffixes += [
                    ('.cpython-{0}{1}.so'.format(abi_version, abi_flags), 'rb', 3),
                    ('.abi{0}.so'.format(sys.version_info[0]), 'rb', 3),
                    ('.so', 'rb', 3),
                ]

    def excludeFrom(self, freezer):
        """ Excludes all modules that have already been processed by
        the indicated FreezeTool.  This is equivalent to passing the
        indicated FreezeTool object as previous to this object's
        constructor, but it may be called at any point during
        processing. """

        for key, value in list(freezer.modules.items()):
            self.previousModules[key] = value
            self.modules[key] = value

    def excludeModule(self, moduleName, forbid = False, allowChildren = False,
                      fromSource = None):
        """ Adds a module to the list of modules not to be exported by
        this tool.  If forbid is true, the module is furthermore
        forbidden to be imported, even if it exists on disk.  If
        allowChildren is true, the children of the indicated module
        may still be included."""

        assert self.mf is None

        self.modules[moduleName] = self.ModuleDef(
            moduleName, exclude = True,
            forbid = forbid, allowChildren = allowChildren,
            fromSource = fromSource)

    def handleCustomPath(self, moduleName):
        """ Indicates a module that may perform runtime manipulation
        of its __path__ variable, and which must therefore be actually
        imported at runtime in order to determine the true value of
        __path__. """

        str = 'import %s' % (moduleName)
        exec(str)

        module = sys.modules[moduleName]
        for path in module.__path__:
            modulefinder.AddPackagePath(moduleName, path)

    def getModulePath(self, moduleName):
        """ Looks for the indicated directory module and returns the
        __path__ member: the list of directories in which its python
        files can be found.  If the module is a .py file and not a
        directory, returns None. """

        # First, try to import the module directly.  That's the most
        # reliable answer, if it works.
        try:
            module = __import__(moduleName)
        except:
            print("couldn't import %s" % (moduleName))
            module = None

        if module is not None:
            for symbol in moduleName.split('.')[1:]:
                module = getattr(module, symbol)
            if hasattr(module, '__path__'):
                return module.__path__

        # If it didn't work--maybe the module is unimportable because
        # it makes certain assumptions about the builtins, or
        # whatever--then just look for file on disk.  That's usually
        # good enough.
        path = None
        baseName = moduleName
        if '.' in baseName:
            parentName, baseName = moduleName.rsplit('.', 1)
            path = self.getModulePath(parentName)
            if path is None:
                return None

        try:
            file, pathname, description = imp.find_module(baseName, path)
        except ImportError:
            return None

        if not os.path.isdir(pathname):
            return None
        return [pathname]

    def getModuleStar(self, moduleName):
        """ Looks for the indicated directory module and returns the
        __all__ member: the list of symbols within the module. """

        # First, try to import the module directly.  That's the most
        # reliable answer, if it works.
        try:
            module = __import__(moduleName)
        except:
            print("couldn't import %s" % (moduleName))
            module = None

        if module is not None:
            for symbol in moduleName.split('.')[1:]:
                module = getattr(module, symbol)
            if hasattr(module, '__all__'):
                return module.__all__

        # If it didn't work, just open the directory and scan for *.py
        # files.
        path = None
        baseName = moduleName
        if '.' in baseName:
            parentName, baseName = moduleName.rsplit('.', 1)
            path = self.getModulePath(parentName)
            if path is None:
                return None

        try:
            file, pathname, description = imp.find_module(baseName, path)
        except ImportError:
            return None

        if not os.path.isdir(pathname):
            return None

        # Scan the directory, looking for .py files.
        modules = []
        for basename in os.listdir(pathname):
            if basename.endswith('.py') and basename != '__init__.py':
                modules.append(basename[:-3])

        return modules

    def _gatherSubmodules(self, moduleName, implicit = False, newName = None,
                          filename = None, guess = False, fromSource = None,
                          text = None):
        if not newName:
            newName = moduleName

        assert(moduleName.endswith('.*'))
        assert(newName.endswith('.*'))

        mdefs = {}

        # Find the parent module, so we can get its directory.
        parentName = moduleName[:-2]
        newParentName = newName[:-2]
        parentNames = [(parentName, newParentName)]

        if parentName.endswith('.*'):
            assert(newParentName.endswith('.*'))
            # Another special case.  The parent name "*" means to
            # return all possible directories within a particular
            # directory.

            topName = parentName[:-2]
            newTopName = newParentName[:-2]
            parentNames = []
            modulePath = self.getModulePath(topName)
            if modulePath:
                for dirname in modulePath:
                    for basename in os.listdir(dirname):
                        if os.path.exists(os.path.join(dirname, basename, '__init__.py')):
                            parentName = '%s.%s' % (topName, basename)
                            newParentName = '%s.%s' % (newTopName, basename)
                            if self.getModulePath(parentName):
                                parentNames.append((parentName, newParentName))

        for parentName, newParentName in parentNames:
            modules = self.getModuleStar(parentName)

            if modules is None:
                # It's actually a regular module.
                mdefs[newParentName] = self.ModuleDef(
                    parentName, implicit = implicit, guess = guess,
                    fromSource = fromSource, text = text)

            else:
                # Now get all the py files in the parent directory.
                for basename in modules:
                    moduleName = '%s.%s' % (parentName, basename)
                    newName = '%s.%s' % (newParentName, basename)
                    mdefs[newName] = self.ModuleDef(
                        moduleName, implicit = implicit, guess = True,
                        fromSource = fromSource)
        return mdefs

    def addModule(self, moduleName, implicit = False, newName = None,
                  filename = None, guess = False, fromSource = None,
                  text = None):
        """ Adds a module to the list of modules to be exported by
        this tool.  If implicit is true, it is OK if the module does
        not actually exist.

        newName is the name to call the module when it appears in the
        output.  The default is the same name it had in the original.
        Use caution when renaming a module; if another module imports
        this module by its original name, you will also need to
        explicitly add the module under its original name, duplicating
        the module twice in the output.

        The module name may end in ".*", which means to add all of the
        .py files (other than __init__.py) in a particular directory.
        It may also end in ".*.*", which means to cycle through all
        directories within a particular directory.
        """

        assert self.mf is None

        if not newName:
            newName = moduleName

        if moduleName.endswith('.*'):
            self.modules.update(self._gatherSubmodules(
                moduleName, implicit, newName, filename,
                guess, fromSource, text))
        else:
            # A normal, explicit module name.
            self.modules[newName] = self.ModuleDef(
                moduleName, filename = filename, implicit = implicit,
                guess = guess, fromSource = fromSource, text = text)

    def done(self, addStartupModules = False):
        """ Call this method after you have added all modules with
        addModule().  You may then call generateCode() or
        writeMultifile() to dump the resulting output.  After a call
        to done(), you may not add any more modules until you call
        reset(). """

        assert self.mf is None

        # If we are building an exe, we also need to implicitly
        # bring in Python's startup modules.
        if addStartupModules:
            self.modules['_frozen_importlib'] = self.ModuleDef('importlib._bootstrap', implicit = True)
            self.modules['_frozen_importlib_external'] = self.ModuleDef('importlib._bootstrap_external', implicit = True)

            for moduleName in startupModules:
                if moduleName not in self.modules:
                    self.addModule(moduleName, implicit = True)

        # Excluding a parent module also excludes all its
        # (non-explicit) children, unless the parent has allowChildren
        # set.

        # Walk through the list in sorted order, so we reach parents
        # before children.
        names = list(self.modules.items())
        names.sort()

        excludeDict = {}
        implicitParentDict = {}
        includes = []
        autoIncludes = []
        origToNewName = {}
        for newName, mdef in names:
            moduleName = mdef.moduleName
            origToNewName[moduleName] = newName
            if mdef.implicit and '.' in newName:
                # For implicit modules, check if the parent is excluded.
                parentName, baseName = newName.rsplit('.', 1)
                if parentName in excludeDict :
                    mdef = excludeDict[parentName]

            if mdef.exclude:
                if not mdef.allowChildren:
                    excludeDict[moduleName] = mdef
            elif mdef.implicit or mdef.guess:
                autoIncludes.append(mdef)
            else:
                includes.append(mdef)

        self.mf = PandaModuleFinder(excludes=list(excludeDict.keys()), suffixes=self.moduleSuffixes, path=self.path)

        # Attempt to import the explicit modules into the modulefinder.

        # First, ensure the includes are sorted in order so that
        # packages appear before the modules they contain.  This
        # resolves potential ordering issues, especially with modules
        # that are discovered by filename rather than through import
        # statements.
        includes.sort(key = self.__sortModuleKey)

        # Now walk through the list and import them all.
        for mdef in includes:
            try:
                self.__loadModule(mdef)
            except ImportError as ex:
                message = "Unknown module: %s" % (mdef.moduleName)
                if str(ex) != "No module named " + str(mdef.moduleName):
                    message += " (%s)" % (ex)
                print(message)

        # Also attempt to import any implicit modules.  If any of
        # these fail to import, we don't really care.
        for mdef in autoIncludes:
            try:
                self.__loadModule(mdef)
                # Since it successfully loaded, it's no longer a guess.
                mdef.guess = False
            except:
                # Something went wrong, guess it's not an importable
                # module.
                pass

        # Check if any new modules we found have "hidden" imports
        for origName in list(self.mf.modules.keys()):
            hidden = hiddenImports.get(origName, [])
            for modname in hidden:
                if modname.endswith('.*'):
                    mdefs = self._gatherSubmodules(modname, implicit = True)
                    for mdef in mdefs.values():
                        try:
                            self.__loadModule(mdef)
                        except ImportError:
                            pass
                else:
                    self.__loadModule(self.ModuleDef(modname, implicit = True))

        # Now, any new modules we found get added to the export list.
        for origName in list(self.mf.modules.keys()):
            if origName not in origToNewName:
                self.modules[origName] = self.ModuleDef(origName, implicit = True)

        missing = []
        for origName in self.mf.any_missing_maybe()[0]:
            if origName in startupModules:
                continue
            if origName in self.previousModules:
                continue
            if origName in self.modules:
                continue

            # This module is missing.  Let it be missing in the
            # runtime also.
            self.modules[origName] = self.ModuleDef(origName, exclude = True,
                                                    implicit = True)

            if origName in okMissing:
                # If it's listed in okMissing, don't even report it.
                continue

            prefix = origName.split('.')[0]
            if origName not in reportedMissing:
                missing.append(origName)
                reportedMissing[origName] = True

        if missing:
            missing.sort()
            print("There are some missing modules: %r" % missing)

    def __sortModuleKey(self, mdef):
        """ A sort key function to sort a list of mdef's into order,
        primarily to ensure that packages proceed their modules. """

        if mdef.moduleName:
            # If we have a moduleName, the key consists of the split
            # tuple of packages names.  That way, parents always sort
            # before children.
            return ('a', mdef.moduleName.split('.'))
        else:
            # If we don't have a moduleName, the key doesn't really
            # matter--we use filename--but we start with 'b' to ensure
            # that all of non-named modules appear following all of
            # the named modules.
            return ('b', mdef.filename)

    def __loadModule(self, mdef):
        """ Adds the indicated module to the modulefinder. """

        if mdef.filename:
            # If it has a filename, then we found it as a file on
            # disk.  In this case, the moduleName may not be accurate
            # and useful, so load it as a file instead.

            tempPath = None
            if '.' not in mdef.moduleName:
                # If we loaded a python file from the root, we need to
                # temporarily add its directory to the module search
                # path, so the modulefinder can find any sibling
                # python files it imports as well.
                tempPath = Filename(mdef.filename.getDirname()).toOsSpecific()
                self.mf.path.append(tempPath)

            pathname = mdef.filename.toOsSpecific()
            ext = mdef.filename.getExtension()
            if ext == 'pyc' or ext == 'pyo':
                fp = open(pathname, 'rb')
                stuff = ("", "rb", imp.PY_COMPILED)
                self.mf.load_module(mdef.moduleName, fp, pathname, stuff)
            else:
                stuff = ("", "rb", imp.PY_SOURCE)
                if mdef.text:
                    fp = io.StringIO(mdef.text)
                else:
                    fp = open(pathname, 'rb')
                self.mf.load_module(mdef.moduleName, fp, pathname, stuff)

            if tempPath:
                del self.mf.path[-1]

        else:
            # Otherwise, we can just import it normally.
            self.mf.import_hook(mdef.moduleName)

    def reset(self):
        """ After a previous call to done(), this resets the
        FreezeTool object for a new pass.  More modules may be added
        and dumped to a new target.  Previously-added modules are
        remembered and will not be dumped again. """

        self.mf = None
        self.previousModules = dict(self.modules)

    def mangleName(self, moduleName):
        return 'M_' + moduleName.replace('.', '__').replace('-', '_')

    def getAllModuleNames(self):
        """ Return a list of all module names that have been included
        or forbidden, either in this current pass or in a previous
        pass.  Module names that have been excluded are not included
        in this list. """

        moduleNames = []

        for newName, mdef in list(self.modules.items()):
            if mdef.guess:
                # Not really a module.
                pass
            elif mdef.exclude and not mdef.forbid:
                # An excluded (but not forbidden) file.
                pass
            else:
                moduleNames.append(newName)

        moduleNames.sort()
        return moduleNames

    def getModuleDefs(self):
        """ Return a list of all of the modules we will be explicitly
        or implicitly including.  The return value is actually a list
        of tuples: (moduleName, moduleDef)."""

        moduleDefs = []

        for newName, mdef in list(self.modules.items()):
            prev = self.previousModules.get(newName, None)
            if not mdef.exclude:
                # Include this module (even if a previous pass
                # excluded it).  But don't bother if we exported it
                # previously.
                if prev and not prev.exclude:
                    # Previously exported.
                    pass
                elif mdef.moduleName in self.mf.modules or \
                     mdef.moduleName in startupModules or \
                     mdef.filename:
                    moduleDefs.append((newName, mdef))
            elif mdef.forbid:
                if not prev or not prev.forbid:
                    moduleDefs.append((newName, mdef))

        moduleDefs.sort()
        return moduleDefs

    def __replacePaths(self):
        # Build up the replacement pathname table, so we can eliminate
        # the personal information in the frozen pathnames.  The
        # actual filename we put in there is meaningful only for stack
        # traces, so we'll just use the module name.
        replace_paths = []
        for moduleName, module in list(self.mf.modules.items()):
            if module.__code__:
                origPathname = module.__code__.co_filename
                replace_paths.append((origPathname, moduleName))
        self.mf.replace_paths = replace_paths

        # Now that we have built up the replacement mapping, go back
        # through and actually replace the paths.
        for moduleName, module in list(self.mf.modules.items()):
            if module.__code__:
                co = self.mf.replace_paths_in_code(module.__code__)
                module.__code__ = co;

    def __addPyc(self, multifile, filename, code, compressionLevel):
        if code:
            data = imp.get_magic() + b'\0\0\0\0'

            if sys.version_info >= (3, 0):
                data += b'\0\0\0\0'

            data += marshal.dumps(code)

            stream = StringStream(data)
            multifile.addSubfile(filename, stream, compressionLevel)
            multifile.flush()

    def __addPythonDirs(self, multifile, moduleDirs, dirnames, compressionLevel):
        """ Adds all of the names on dirnames as a module directory. """
        if not dirnames:
            return

        str = '.'.join(dirnames)
        if str not in moduleDirs:
            # Add an implicit __init__.py file (but only if there's
            # not already a legitimate __init__.py file).
            moduleName = '.'.join(dirnames)
            filename = '/'.join(dirnames) + '/__init__'

            if self.storePythonSource:
                filename += '.py'
                stream = StringStream(b'')
                if multifile.findSubfile(filename) < 0:
                    multifile.addSubfile(filename, stream, 0)
                    multifile.flush()
            else:
                if __debug__:
                    filename += '.pyc'
                else:
                    filename += '.pyo'
                if multifile.findSubfile(filename) < 0:
                    code = compile('', moduleName, 'exec')
                    self.__addPyc(multifile, filename, code, compressionLevel)

            moduleDirs[str] = True
            self.__addPythonDirs(multifile, moduleDirs, dirnames[:-1], compressionLevel)

    def __addPythonFile(self, multifile, moduleDirs, moduleName, mdef,
                        compressionLevel):
        """ Adds the named module to the multifile as a .pyc file. """

        # First, split the module into its subdirectory names.
        dirnames = moduleName.split('.')
        if len(dirnames) > 1 and dirnames[-1] == '__init__':
            # The "module" may end in __init__, but that really means
            # the parent directory.
            dirnames = dirnames[:-1]

        self.__addPythonDirs(multifile, moduleDirs, dirnames[:-1], compressionLevel)

        filename = '/'.join(dirnames)

        module = self.mf.modules.get(mdef.moduleName, None)
        if getattr(module, '__path__', None) is not None or \
          (getattr(module, '__file__', None) is not None and getattr(module, '__file__').endswith('/__init__.py')):
            # It's actually a package.  In this case, we really write
            # the file moduleName/__init__.py.
            filename += '/__init__'
            moduleDirs[moduleName] = True

            # Ensure we don't have an implicit filename from above.
            multifile.removeSubfile(filename + '.py')
            if __debug__:
                multifile.removeSubfile(filename + '.pyc')
            else:
                multifile.removeSubfile(filename + '.pyo')

        # Attempt to add the original source file if we can.
        sourceFilename = None
        if mdef.filename and mdef.filename.getExtension() == "py":
            sourceFilename = mdef.filename
        elif getattr(module, '__file__', None):
            sourceFilename = Filename.fromOsSpecific(module.__file__)
            sourceFilename.setExtension("py")
            sourceFilename.setText()

        if self.storePythonSource:
            if sourceFilename and sourceFilename.exists():
                filename += '.py'
                multifile.addSubfile(filename, sourceFilename, compressionLevel)
                return

        # If we can't find the source file, add the compiled pyc instead.
        if __debug__:
            filename += '.pyc'
        else:
            filename += '.pyo'

        code = None
        if module:
            # Get the compiled code directly from the module object.
            code = getattr(module, "__code__", None)
            if not code:
                # This is a module with no associated Python
                # code.  It must be an extension module.  Get the
                # filename.
                extensionFilename = getattr(module, '__file__', None)
                if extensionFilename:
                    self.extras.append((moduleName, extensionFilename))
                else:
                    # It doesn't even have a filename; it must
                    # be a built-in module.  No worries about
                    # this one, then.
                    pass

        else:
            # Read the code from the source file and compile it on-the-fly.
            if sourceFilename and sourceFilename.exists():
                source = open(sourceFilename.toOsSpecific(), 'r').read()
                if source and source[-1] != '\n':
                    source = source + '\n'
                code = compile(source, str(sourceFilename), 'exec')

        self.__addPyc(multifile, filename, code, compressionLevel)

    def addToMultifile(self, multifile, compressionLevel = 0):
        """ After a call to done(), this stores all of the accumulated
        python code into the indicated Multifile.  Additional
        extension modules are listed in self.extras.  """

        moduleDirs = {}
        for moduleName, mdef in self.getModuleDefs():
            if not mdef.exclude:
                self.__addPythonFile(multifile, moduleDirs, moduleName, mdef,
                                     compressionLevel)

    def writeMultifile(self, mfname):
        """ After a call to done(), this stores all of the accumulated
        python code into a Multifile with the indicated filename,
        including the extension.  Additional extension modules are
        listed in self.extras."""

        self.__replacePaths()

        Filename(mfname).unlink()
        multifile = Multifile()
        if not multifile.openReadWrite(mfname):
            raise Exception

        self.addToMultifile(multifile)

        multifile.flush()
        multifile.repack()

    def writeCode(self, filename, initCode = ""):
        """ After a call to done(), this freezes all of the accumulated
        Python code into a C source file. """

        self.__replacePaths()

        # Now generate the actual export table.
        moduleDefs = []
        moduleList = []

        for moduleName, mdef in self.getModuleDefs():
            origName = mdef.moduleName
            if mdef.forbid:
                # Explicitly disallow importing this module.
                moduleList.append(self.makeForbiddenModuleListEntry(moduleName))
                continue

            assert not mdef.exclude
            # Allow importing this module.
            module = self.mf.modules.get(origName, None)
            code = getattr(module, "__code__", None)
            if code:
                code = marshal.dumps(code)

                mangledName = self.mangleName(moduleName)
                moduleDefs.append(self.makeModuleDef(mangledName, code))
                moduleList.append(self.makeModuleListEntry(mangledName, code, moduleName, module))
                continue

            #if moduleName in startupModules:
            #    # Forbid the loading of this startup module.
            #    moduleList.append(self.makeForbiddenModuleListEntry(moduleName))
            #    continue

            # This is a module with no associated Python code.  It is either
            # an extension module or a builtin module.  Get the filename, if
            # it is the former.
            extensionFilename = getattr(module, '__file__', None)

            if extensionFilename or self.linkExtensionModules:
                self.extras.append((moduleName, extensionFilename))

            # If it is a submodule of a frozen module, Python will have
            # trouble importing it as a builtin module.  Synthesize a frozen
            # module that loads it as builtin.
            if '.' in moduleName and self.linkExtensionModules:
                code = compile('import sys;del sys.modules["%s"];import imp;imp.init_builtin("%s")' % (moduleName, moduleName), moduleName, 'exec')
                code = marshal.dumps(code)
                mangledName = self.mangleName(moduleName)
                moduleDefs.append(self.makeModuleDef(mangledName, code))
                moduleList.append(self.makeModuleListEntry(mangledName, code, moduleName, None))
            elif '.' in moduleName:
                # Nothing we can do about this case except warn the user they
                # are in for some trouble.
                print('WARNING: Python cannot import extension modules under '
                      'frozen Python packages; %s will be inaccessible.  '
                      'passing either -l to link in extension modules or use '
                      '-x %s to exclude the entire package.' % (moduleName, moduleName.split('.')[0]))

        text = programFile % {
            'moduleDefs': '\n'.join(moduleDefs),
            'moduleList': '\n'.join(moduleList),
            }

        if self.linkExtensionModules and self.extras:
            # Should we link in extension modules?  If so, we write out a new
            # built-in module table that directly hooks up with the init
            # functions.  On Linux, we completely override Python's own
            # built-in module table; on Windows, we can't do this, so we
            # instead use PyImport_ExtendInittab to add to it.

            # Python 3 case.
            text += '#if PY_MAJOR_VERSION >= 3\n'
            for module, fn in self.extras:
                if sys.platform != "win32" or fn:
                    libName = module.split('.')[-1]
                    initFunc = builtinInitFuncs.get(module, 'PyInit_' + libName)
                    if initFunc:
                        text += 'extern PyAPI_FUNC(PyObject) *%s(void);\n' % (initFunc)
            text += '\n'

            if sys.platform == "win32":
                text += 'static struct _inittab extensions[] = {\n'
            else:
                text += 'struct _inittab _PyImport_Inittab[] = {\n'

            for module, fn in self.extras:
                if sys.platform != "win32" or fn:
                    libName = module.split('.')[-1]
                    initFunc = builtinInitFuncs.get(module, 'PyInit_' + libName) or 'NULL'
                    text += '  {"%s", %s},\n' % (module, initFunc)
            text += '  {0, 0},\n'
            text += '};\n\n'

            # Python 2 case.
            text += '#else\n'
            for module, fn in self.extras:
                if sys.platform != "win32" or fn:
                    libName = module.split('.')[-1]
                    initFunc = builtinInitFuncs.get(module, 'init' + libName)
                    if initFunc:
                        text += 'extern PyAPI_FUNC(void) %s(void);\n' % (initFunc)
            text += '\n'

            if sys.platform == "win32":
                text += 'static struct _inittab extensions[] = {\n'
            else:
                text += 'struct _inittab _PyImport_Inittab[] = {\n'

            for module, fn in self.extras:
                if sys.platform != "win32" or fn:
                    libName = module.split('.')[-1]
                    initFunc = builtinInitFuncs.get(module, 'init' + libName) or 'NULL'
                    text += '  {"%s", %s},\n' % (module, initFunc)
            text += '  {0, 0},\n'
            text += '};\n'
            text += '#endif\n\n'

        elif sys.platform == "win32":
            text += 'static struct _inittab extensions[] = {\n'
            text += '  {0, 0},\n'
            text += '};\n\n'

        text += initCode

        if filename is not None:
            file = open(filename, 'w')
            file.write(text)
            file.close()

    def generateCode(self, basename, compileToExe = False):
        """ After a call to done(), this freezes all of the
        accumulated python code into either an executable program (if
        compileToExe is true) or a dynamic library (if compileToExe is
        false).  The basename is the name of the file to write,
        without the extension.

        The return value is the newly-generated filename, including
        the filename extension.  Additional extension modules are
        listed in self.extras. """

        if compileToExe:
            # We must have a __main__ module to make an exe file.
            if not self.__writingModule('__main__'):
                message = "Can't generate an executable without a __main__ module."
                raise Exception(message)

        filename = basename + self.sourceExtension

        dllexport = ''
        dllimport = ''
        if self.platform.startswith('win'):
            dllexport = '__declspec(dllexport) '
            dllimport = '__declspec(dllimport) '

        if not self.cenv:
            self.cenv = CompilationEnvironment(platform = self.platform)

        if compileToExe:
            code = self.frozenMainCode
            if self.platform.startswith('win'):
                code += self.frozenDllMainCode
            initCode = self.mainInitCode % {
                'frozenMainCode' : code,
                'programName' : os.path.basename(basename),
                'dllexport' : dllexport,
                'dllimport' : dllimport,
                }
            if self.platform.startswith('win'):
                target = basename + '.exe'
            else:
                target = basename

            compileFunc = self.cenv.compileExe

        else:
            if self.platform.startswith('win'):
                target = basename + self.cenv.dllext + '.pyd'
            else:
                target = basename + '.so'

            initCode = dllInitCode % {
                'moduleName' : os.path.basename(basename),
                'dllexport' : dllexport,
                'dllimport' : dllimport,
                }
            compileFunc = self.cenv.compileDll

        self.writeCode(filename, initCode=initCode)

        # Keep track of the files we should clean up after use.
        cleanFiles = [filename, basename + self.objectExtension]

        extraLink = []
        if self.linkExtensionModules:
            for mod, fn in self.extras:
                if not fn:
                    continue
                if sys.platform == 'win32':
                    # We can't link with a .pyd directly on Windows.  Check
                    # if there is a corresponding .lib file in the Python libs
                    # directory.
                    libsdir = os.path.join(sys.exec_prefix, 'libs')
                    libfile = os.path.join(libsdir, mod + '.lib')
                    if os.path.isfile(libfile):
                        extraLink.append(mod + '.lib')
                        continue

                    # No, so we have to generate a .lib file.  This is pretty
                    # easy given that we know the only symbol we need is a
                    # initmodule or PyInit_module function.
                    modname = mod.split('.')[-1]
                    libfile = modname + '.lib'
                    if sys.version_info >= (3, 0):
                        symbolName = 'PyInit_' + modname
                    else:
                        symbolName = 'init' + modname
                    os.system('lib /nologo /def /export:%s /name:%s.pyd /out:%s' % (symbolName, modname, libfile))
                    extraLink.append(libfile)
                    cleanFiles += [libfile, modname + '.exp']
                else:
                    extraLink.append(fn)

        try:
            compileFunc(filename, basename, extraLink=extraLink)
        finally:
            if not self.keepTemporaryFiles:
                for file in cleanFiles:
                    if os.path.exists(file):
                        os.unlink(file)

        return target

    def generateRuntimeFromStub(self, target, stub_file, use_console, fields={},
                                log_append=False):
        # We must have a __main__ module to make an exe file.
        if not self.__writingModule('__main__'):
            message = "Can't generate an executable without a __main__ module."
            raise Exception(message)

        if self.platform.startswith('win'):
            modext = '.pyd'
        else:
            modext = '.so'

        # First gather up the strings and code for all the module names, and
        # put those in a string pool.
        pool = b""
        strings = set()

        for moduleName, mdef in self.getModuleDefs():
            strings.add(moduleName.encode('ascii'))

        for value in fields.values():
            if value is not None:
                strings.add(value.encode('utf-8'))

        # Sort by length descending, allowing reuse of partial strings.
        strings = sorted(strings, key=lambda str:-len(str))
        string_offsets = {}

        # Now add the strings to the pool, and collect the offsets relative to
        # the beginning of the pool.
        for string in strings:
            # First check whether it's already in there; it could be part of
            # a longer string.
            offset = pool.find(string + b'\0')
            if offset < 0:
                offset = len(pool)
                pool += string + b'\0'
            string_offsets[string] = offset

        # Now go through the modules and add them to the pool as well.  These
        # are not 0-terminated, but we later record their sizes and names in
        # a table after the blob header.
        moduleList = []

        for moduleName, mdef in self.getModuleDefs():
            origName = mdef.moduleName
            if mdef.forbid:
                # Explicitly disallow importing this module.
                moduleList.append((moduleName, 0, 0))
                continue

            # For whatever it's worth, align the code blocks.
            if len(pool) & 3 != 0:
                pad = (4 - (len(pool) & 3))
                pool += b'\0' * pad

            assert not mdef.exclude
            # Allow importing this module.
            module = self.mf.modules.get(origName, None)
            code = getattr(module, "__code__", None)
            if code:
                code = marshal.dumps(code)
                size = len(code)
                if getattr(module, "__path__", None):
                    # Indicate package by negative size
                    size = -size
                moduleList.append((moduleName, len(pool), size))
                pool += code
                continue

            # This is a module with no associated Python code.  It is either
            # an extension module or a builtin module.  Get the filename, if
            # it is the former.
            extensionFilename = getattr(module, '__file__', None)

            if extensionFilename:
                self.extras.append((moduleName, extensionFilename))

            # If it is a submodule of a frozen module, Python will have
            # trouble importing it as a builtin module.  Synthesize a frozen
            # module that loads it dynamically.
            if '.' in moduleName:
                if self.platform.startswith("macosx") and not use_console:
                    # We write the Frameworks directory to sys.path[0].
                    code = 'import sys;del sys.modules["%s"];import sys,os,imp;imp.load_dynamic("%s",os.path.join(sys.path[0], "%s%s"))' % (moduleName, moduleName, moduleName, modext)
                else:
                    code = 'import sys;del sys.modules["%s"];import sys,os,imp;imp.load_dynamic("%s",os.path.join(os.path.dirname(sys.executable), "%s%s"))' % (moduleName, moduleName, moduleName, modext)
                if sys.version_info >= (3, 2):
                    code = compile(code, moduleName, 'exec', optimize=2)
                else:
                    code = compile(code, moduleName, 'exec')
                code = marshal.dumps(code)
                moduleList.append((moduleName, len(pool), len(code)))
                pool += code

        # Determine the format of the header and module list entries depending
        # on the platform.
        num_pointers = 12
        stub_data = bytearray(stub_file.read())
        bitnesses = self._get_executable_bitnesses(stub_data)

        header_layouts = {
            32: '<QQHHHH8x%dII' % num_pointers,
            64: '<QQHHHH8x%dQQ' % num_pointers,
        }
        entry_layouts = {
            32: '<IIi',
            64: '<QQixxxx',
        }

        # Calculate the size of the module tables, so that we can determine
        # the proper offset for the string pointers.  There can be more than
        # one module table for macOS executables.  Sort the bitnesses so that
        # the alignment is correct.
        bitnesses = sorted(bitnesses, reverse=True)

        pool_offset = 0
        for bitness in bitnesses:
            pool_offset += (len(moduleList) + 1) * struct.calcsize(entry_layouts[bitness])

        # Now we can determine the offset of the blob.
        if self.platform.startswith('win'):
            # We don't use mmap on Windows.  Align just for good measure.
            blob_align = 32
        else:
            # Align to page size, so that it can be mmapped.
            blob_align = 4096

        # Add padding before the blob if necessary.
        blob_offset = len(stub_data)
        if (blob_offset & (blob_align - 1)) != 0:
            pad = (blob_align - (blob_offset & (blob_align - 1)))
            stub_data += (b'\0' * pad)
            blob_offset += pad
        assert (blob_offset % blob_align) == 0
        assert blob_offset == len(stub_data)

        # Also determine the total blob size now.  Add padding to the end.
        blob_size = pool_offset + len(pool)
        if blob_size & 31 != 0:
            pad = (32 - (blob_size & 31))
            blob_size += pad

        # Calculate the offsets for the variables.  These are pointers,
        # relative to the beginning of the blob.
        field_offsets = {}
        for key, value in fields.items():
            if value is not None:
                encoded = value.encode('utf-8')
                field_offsets[key] = pool_offset + string_offsets[encoded]

        # OK, now go and write the blob.  This consists of the module table
        # (there may be two in the case of a macOS universal (fat) binary).
        blob = b""
        append_offset = False
        for bitness in bitnesses:
            entry_layout = entry_layouts[bitness]
            header_layout = header_layouts[bitness]

            table_offset = len(blob)
            for moduleName, offset, size in moduleList:
                encoded = moduleName.encode('ascii')
                string_offset = pool_offset + string_offsets[encoded]
                if size != 0:
                    offset += pool_offset
                blob += struct.pack(entry_layout, string_offset, offset, size)

            # A null entry marks the end of the module table.
            blob += struct.pack(entry_layout, 0, 0, 0)

            flags = 0
            if log_append:
                flags |= 1

            # Compose the header we will be writing to the stub, to tell it
            # where to find the module data blob, as well as other variables.
            header = struct.pack(header_layout,
                blob_offset,
                blob_size,
                1, # Version number
                num_pointers, # Number of pointers that follow
                0, # Codepage, not yet used
                flags,
                table_offset, # Module table pointer.
                # The following variables need to be set before static init
                # time.  See configPageManager.cxx, where they are read.
                field_offsets.get('prc_data', 0),
                field_offsets.get('default_prc_dir', 0),
                field_offsets.get('prc_dir_envvars', 0),
                field_offsets.get('prc_path_envvars', 0),
                field_offsets.get('prc_patterns', 0),
                field_offsets.get('prc_encrypted_patterns', 0),
                field_offsets.get('prc_encryption_key', 0),
                field_offsets.get('prc_executable_patterns', 0),
                field_offsets.get('prc_executable_args_envvar', 0),
                field_offsets.get('main_dir', 0),
                field_offsets.get('log_filename', 0),
                0)

            # Now, find the location of the 'blobinfo' symbol in the binary,
            # to which we will write our header.
            if not self._replace_symbol(stub_data, b'blobinfo', header, bitness=bitness):
                # This must be a legacy deploy-stub, which requires the offset to
                # be appended to the end.
                append_offset = True

        # Add the string/code pool.
        assert len(blob) == pool_offset
        blob += pool
        del pool

        # Now pad out the blob to the calculated blob size.
        if len(blob) < blob_size:
            blob += b'\0' * (blob_size - len(blob))
        assert len(blob) == blob_size

        if append_offset:
            # This is for legacy deploy-stub.
            print("WARNING: Could not find blob header. Is deploy-stub outdated?")
            blob += struct.pack('<Q', blob_offset)

        with open(target, 'wb') as f:
            f.write(stub_data)
            assert f.tell() == blob_offset
            f.write(blob)

        os.chmod(target, 0o755)
        return target

    def _get_executable_bitnesses(self, data):
        """Returns the bitnesses (32 or 64) of the given executable data.
        This will contain 1 element for non-fat executables."""

        if data.startswith(b'MZ'):
            # A Windows PE file.
            offset, = struct.unpack_from('<I', data, 0x3c)
            assert data[offset:offset+4] == b'PE\0\0'

            magic, = struct.unpack_from('<H', data, offset + 24)
            assert magic in (0x010b, 0x020b)
            if magic == 0x020b:
                return (64,)
            else:
                return (32,)

        elif data.startswith(b"\177ELF"):
            # A Linux/FreeBSD ELF executable.
            elfclass = ord(data[4:5])
            assert elfclass in (1, 2)
            return (elfclass * 32,)

        elif data[:4] in (b'\xFE\xED\xFA\xCE', b'\xCE\xFA\xED\xFE'):
            # 32-bit Mach-O file, as used on macOS.
            return (32,)

        elif data[:4] in (b'\xFE\xED\xFA\xCF', b'\xCF\xFA\xED\xFE'):
            # 64-bit Mach-O file, as used on macOS.
            return (64,)

        elif data[:4] in (b'\xCA\xFE\xBA\xBE', b'\xBE\xBA\xFE\xCA'):
            # Universal binary with 32-bit offsets.
            num_fat, = struct.unpack_from('>I', data, 4)
            bitnesses = set()
            ptr = 8
            for i in range(num_fat):
                cputype, cpusubtype, offset, size, align = \
                    struct.unpack_from('>IIIII', data, ptr)
                ptr += 20

                if (cputype & 0x1000000) != 0:
                    bitnesses.add(64)
                else:
                    bitnesses.add(32)
            return tuple(bitnesses)

        elif data[:4] in (b'\xCA\xFE\xBA\xBF', b'\xBF\xBA\xFE\xCA'):
            # Universal binary with 64-bit offsets.
            num_fat, = struct.unpack_from('>I', data, 4)
            bitnesses = set()
            ptr = 8
            for i in range(num_fat):
                cputype, cpusubtype, offset, size, align = \
                    struct.unpack_from('>QQQQQ', data, ptr)
                ptr += 40

                if (cputype & 0x1000000) != 0:
                    bitnesses.add(64)
                else:
                    bitnesses.add(32)
            return tuple(bitnesses)

    def _replace_symbol(self, data, symbol_name, replacement, bitness=None):
        """We store a custom section in the binary file containing a header
        containing offsets to the binary data.
        If bitness is set, and the binary in question is a macOS universal
        binary, it only replaces for binaries with the given bitness. """

        if data.startswith(b'MZ'):
            # A Windows PE file.
            pe = pefile.PEFile()
            pe.read(io.BytesIO(data))
            addr = pe.get_export_address(symbol_name)
            if addr is not None:
                # We found it, return its offset in the file.
                offset = pe.get_address_offset(addr)
                if offset is not None:
                    data[offset:offset+len(replacement)] = replacement
                    return True

        elif data.startswith(b"\177ELF"):
            return self._replace_symbol_elf(data, symbol_name, replacement)

        elif data[:4] in (b'\xFE\xED\xFA\xCE', b'\xCE\xFA\xED\xFE',
                          b'\xFE\xED\xFA\xCF', b'\xCF\xFA\xED\xFE'):
            off = self._find_symbol_macho(data, symbol_name)
            if off is not None:
                data[off:off+len(replacement)] = replacement
                return True
            return False

        elif data[:4] in (b'\xCA\xFE\xBA\xBE', b'\xBE\xBA\xFE\xCA'):
            # Universal binary with 32-bit offsets.
            num_fat, = struct.unpack_from('>I', data, 4)
            replaced = False
            ptr = 8
            for i in range(num_fat):
                cputype, cpusubtype, offset, size, align = \
                    struct.unpack_from('>IIIII', data, ptr)
                ptr += 20

                # Does this match the requested bitness?
                if bitness is not None and ((cputype & 0x1000000) != 0) != (bitness == 64):
                    continue

                macho_data = data[offset:offset+size]
                off = self._find_symbol_macho(macho_data, symbol_name)
                if off is not None:
                    off += offset
                    data[off:off+len(replacement)] = replacement
                    replaced = True

            return replaced

        elif data[:4] in (b'\xCA\xFE\xBA\xBF', b'\xBF\xBA\xFE\xCA'):
            # Universal binary with 64-bit offsets.
            num_fat, = struct.unpack_from('>I', data, 4)
            replaced = False
            ptr = 8
            for i in range(num_fat):
                cputype, cpusubtype, offset, size, align = \
                    struct.unpack_from('>QQQQQ', data, ptr)
                ptr += 40

                # Does this match the requested bitness?
                if bitness is not None and ((cputype & 0x1000000) != 0) != (bitness == 64):
                    continue

                macho_data = data[offset:offset+size]
                off = self._find_symbol_macho(macho_data, symbol_name)
                if off is not None:
                    off += offset
                    data[off:off+len(replacement)] = replacement
                    replaced = True

            return replaced

        # We don't know what kind of file this is.
        return False

    def _replace_symbol_elf(self, elf_data, symbol_name, replacement):
        """ The Linux/FreeBSD implementation of _replace_symbol. """

        replaced = False

        # Make sure we read in the correct endianness and integer size
        endian = "<>"[ord(elf_data[5:6]) - 1]
        is_64bit = ord(elf_data[4:5]) - 1 # 0 = 32-bits, 1 = 64-bits
        header_struct = endian + ("HHIIIIIHHHHHH", "HHIQQQIHHHHHH")[is_64bit]
        section_struct = endian + ("4xI4xIIII8xI", "4xI8xQQQI12xQ")[is_64bit]
        symbol_struct = endian + ("IIIBBH", "IBBHQQ")[is_64bit]

        header_size = struct.calcsize(header_struct)
        type, machine, version, entry, phoff, shoff, flags, ehsize, phentsize, phnum, shentsize, shnum, shstrndx \
          = struct.unpack_from(header_struct, elf_data, 16)
        section_offsets = []
        symbol_tables = []
        string_tables = {}

        # Seek to the section header table and find the symbol tables.
        ptr = shoff
        for i in range(shnum):
            type, addr, offset, size, link, entsize = struct.unpack_from(section_struct, elf_data[ptr:ptr+shentsize])
            ptr += shentsize
            section_offsets.append(offset - addr)
            if type == 0x0B and link != 0: # SHT_DYNSYM, links to string table
                symbol_tables.append((offset, size, link, entsize))
                string_tables[link] = None

        # Read the relevant string tables.
        for idx in list(string_tables.keys()):
            ptr = shoff + idx * shentsize
            type, addr, offset, size, link, entsize = struct.unpack_from(section_struct, elf_data[ptr:ptr+shentsize])
            if type == 3:
                string_tables[idx] = elf_data[offset:offset+size]

        # Loop through to find the offset of the "blobinfo" symbol.
        for offset, size, link, entsize in symbol_tables:
            entries = size // entsize
            for i in range(entries):
                ptr = offset + i * entsize
                fields = struct.unpack_from(symbol_struct, elf_data[ptr:ptr+entsize])
                if is_64bit:
                    name, info, other, shndx, value, size = fields
                else:
                    name, value, size, info, other, shndx = fields

                if not name:
                    continue

                name = string_tables[link][name : string_tables[link].find(b'\0', name)]
                if name == symbol_name:
                    if shndx == 0: # SHN_UNDEF
                        continue
                    elif shndx >= 0xff00 and shndx <= 0xffff:
                        assert False
                    else:
                        # Got it.  Make the replacement.
                        off = section_offsets[shndx] + value
                        elf_data[off:off+len(replacement)] = replacement
                        replaced = True

        return replaced

    def _find_symbol_macho(self, macho_data, symbol_name):
        """ Returns the offset of the given symbol in the binary file. """

        if macho_data[:4] in (b'\xCE\xFA\xED\xFE', b'\xCF\xFA\xED\xFE'):
            endian = '<'
        else:
            endian = '>'

        cputype, cpusubtype, filetype, ncmds, sizeofcmds, flags = \
            struct.unpack_from(endian + 'IIIIII', macho_data, 4)

        is_64bit = (cputype & 0x1000000) != 0
        segments = []

        cmd_ptr = 28
        nlist_struct = endian + 'IBBHI'
        if is_64bit:
            nlist_struct = endian + 'IBBHQ'
            cmd_ptr += 4
        nlist_size = struct.calcsize(nlist_struct)

        for i in range(ncmds):
            cmd, cmd_size = struct.unpack_from(endian + 'II', macho_data, cmd_ptr)
            cmd_data = macho_data[cmd_ptr+8:cmd_ptr+cmd_size]
            cmd_ptr += cmd_size

            cmd &= ~0x80000000

            if cmd == 0x01: # LC_SEGMENT
                segname, vmaddr, vmsize, fileoff, filesize, maxprot, initprot, nsects, flags = \
                    struct.unpack_from(endian + '16sIIIIIIII', cmd_data)
                segments.append((vmaddr, vmsize, fileoff))

            elif cmd == 0x19: # LC_SEGMENT_64
                segname, vmaddr, vmsize, fileoff, filesize, maxprot, initprot, nsects, flags = \
                    struct.unpack_from(endian + '16sQQQQIIII', cmd_data)
                segments.append((vmaddr, vmsize, fileoff))

            elif cmd == 0x2: # LC_SYMTAB
                symoff, nsyms, stroff, strsize = \
                    struct.unpack_from(endian + 'IIII', cmd_data)

                strings = macho_data[stroff:stroff+strsize]

                for i in range(nsyms):
                    strx, type, sect, desc, value = struct.unpack_from(nlist_struct, macho_data, symoff)
                    symoff += nlist_size
                    name = strings[strx : strings.find(b'\0', strx)]

                    if name == b'_' + symbol_name:
                        # Find out in which segment this is.
                        for vmaddr, vmsize, fileoff in segments:
                            # Is it defined in this segment?
                            rel = value - vmaddr
                            if rel >= 0 and rel < vmsize:
                                # Yes, so return the symbol offset.
                                return fileoff + rel
                        print("Could not find memory address for symbol %s" % (symbol_name))

    def makeModuleDef(self, mangledName, code):
        result = ''
        result += 'static unsigned char %s[] = {' % (mangledName)
        for i in range(0, len(code), 16):
            result += '\n  '
            for c in code[i:i+16]:
                if isinstance(c, int): # Python 3
                    result += ('%d,' % c)
                else: # Python 2
                    result += ('%d,' % ord(c))
        result += '\n};\n'
        return result

    def makeModuleListEntry(self, mangledName, code, moduleName, module):
        size = len(code)
        if getattr(module, "__path__", None):
            # Indicate package by negative size
            size = -size
        return '  {"%s", %s, %s},' % (moduleName, mangledName, size)

    def makeForbiddenModuleListEntry(self, moduleName):
        return '  {"%s", NULL, 0},' % (moduleName)


    def __writingModule(self, moduleName):
        """ Returns true if we are outputting the named module in this
        pass, false if we have already output in a previous pass, or
        if it is not yet on the output table. """

        mdef = self.modules.get(moduleName, (None, None))
        if mdef.exclude:
            return False

        if moduleName in self.previousModules:
            return False

        return True

class PandaModuleFinder(modulefinder.ModuleFinder):

    def __init__(self, *args, **kw):
        """
        :param path: search path to look on, defaults to sys.path
        :param suffixes: defaults to imp.get_suffixes()
        :param excludes: a list of modules to exclude
        :param debug: an integer indicating the level of verbosity
        """

        self.suffixes = kw.pop('suffixes', imp.get_suffixes())

        modulefinder.ModuleFinder.__init__(self, *args, **kw)

        # Make sure we don't open a .whl/.zip file more than once.
        self._zip_files = {}

    def _open_file(self, path, mode):
        """ Opens a module at the given path, which may contain a zip file.
        Returns None if the module could not be found. """

        if os.path.isfile(path):
            if 'b' not in mode:
                return io.open(path, mode, encoding='utf8')
            else:
                return open(path, mode)

        # Is there a zip file along the path?
        dir, dirname = os.path.split(path)
        fn = dirname
        while dirname:
            if os.path.isfile(dir):
                # Okay, this is actually a file.  Is it a zip file?
                if dir in self._zip_files:
                    # Yes, and we've previously opened this.
                    zip = self._zip_files[dir]
                elif zipfile.is_zipfile(dir):
                    zip = zipfile.ZipFile(dir)
                    self._zip_files[dir] = zip
                else:
                    # It's a different kind of file.  Stop looking.
                    return None

                try:
                    fp = zip.open(fn.replace(os.path.sep, '/'), 'r')
                except KeyError:
                    return None

                if sys.version_info >= (3, 0) and 'b' not in mode:
                    return io.TextIOWrapper(fp, encoding='utf8')
                return fp

            # Look at the parent directory.
            dir, dirname = os.path.split(dir)
            fn = os.path.join(dirname, fn)

        return None

    def load_module(self, fqname, fp, pathname, file_info):
        """Copied from ModuleFinder.load_module with fixes to handle sending bytes
        to compile() for PY_SOURCE types. Sending bytes to compile allows it to
        handle file encodings."""

        suffix, mode, type = file_info
        self.msgin(2, "load_module", fqname, fp and "fp", pathname)
        if type == imp.PKG_DIRECTORY:
            m = self.load_package(fqname, pathname)
            self.msgout(2, "load_module ->", m)
            return m

        if type == imp.PY_SOURCE:
            if fqname in overrideModules:
                # This module has a custom override.
                code = overrideModules[fqname]
            else:
                code = fp.read()

            code += b'\n' if isinstance(code, bytes) else '\n'
            co = compile(code, pathname, 'exec')
        elif type == imp.PY_COMPILED:
            try:
                marshal_data = importlib._bootstrap_external._validate_bytecode_header(fp.read())
            except ImportError as exc:
                self.msgout(2, "raise ImportError: " + str(exc), pathname)
                raise
            co = marshal.loads(marshal_data)
        else:
            co = None

        m = self.add_module(fqname)
        m.__file__ = pathname
        if co:
            if self.replace_paths:
                co = self.replace_paths_in_code(co)
            m.__code__ = co
            self.scan_code(co, m)
        self.msgout(2, "load_module ->", m)
        return m

    # This function is provided here since the Python library version has a bug
    # (see bpo-35376)
    def _safe_import_hook(self, name, caller, fromlist, level=-1):
        # wrapper for self.import_hook() that won't raise ImportError
        if name in self.badmodules:
            self._add_badmodule(name, caller)
            return
        try:
            self.import_hook(name, caller, level=level)
        except ImportError as msg:
            self.msg(2, "ImportError:", str(msg))
            self._add_badmodule(name, caller)
        else:
            if fromlist:
                for sub in fromlist:
                    fullname = name + "." + sub
                    if fullname in self.badmodules:
                        self._add_badmodule(fullname, caller)
                        continue
                    try:
                        self.import_hook(name, caller, [sub], level=level)
                    except ImportError as msg:
                        self.msg(2, "ImportError:", str(msg))
                        self._add_badmodule(fullname, caller)

    def find_module(self, name, path=None, parent=None):
        """ Finds a module with the indicated name on the given search path
        (or self.path if None).  Returns a tuple like (fp, path, stuff), where
        stuff is a tuple like (suffix, mode, type). """

        if imp.is_frozen(name):
            # Don't pick up modules that are frozen into p3dpython.
            raise ImportError("'%s' is a frozen module" % (name))

        if parent is not None:
            fullname = parent.__name__+'.'+name
        else:
            fullname = name
        if fullname in self.excludes:
            raise ImportError(name)

        # If we have a custom override for this module, we know we have it.
        if fullname in overrideModules:
            return (None, '', ('.py', 'r', imp.PY_SOURCE))

        # If no search path is given, look for a built-in module.
        if path is None:
            if name in sys.builtin_module_names:
                return (None, None, ('', '', imp.C_BUILTIN))

            path = self.path

        # Look for the module on the search path.
        for dir_path in path:
            basename = os.path.join(dir_path, name.split('.')[-1])

            # Look for recognized extensions.
            for stuff in self.suffixes:
                suffix, mode, _ = stuff
                fp = self._open_file(basename + suffix, mode)
                if fp:
                    return (fp, basename + suffix, stuff)

            # Consider a package, i.e. a directory containing __init__.py.
            for suffix, mode, _ in self.suffixes:
                init = os.path.join(basename, '__init__' + suffix)
                if self._open_file(init, mode):
                    return (None, basename, ('', '', imp.PKG_DIRECTORY))

        # It wasn't found through the normal channels.  Maybe it's one of
        # ours, or maybe it's frozen?
        if not path:
            # Only if we're not looking on a particular path, though.
            if p3extend_frozen and p3extend_frozen.is_frozen_module(name):
                # It's a frozen module.
                return (None, name, ('', '', imp.PY_FROZEN))

        raise ImportError(name)

    def find_all_submodules(self, m):
        # Overridden so that we can define our own suffixes.
        if not m.__path__:
            return
        modules = {}
        for dir in m.__path__:
            try:
                names = os.listdir(dir)
            except OSError:
                self.msg(2, "can't list directory", dir)
                continue
            for name in names:
                mod = None
                for suff in self.suffixes:
                    n = len(suff)
                    if name[-n:] == suff:
                        mod = name[:-n]
                        break
                if mod and mod != "__init__":
                    modules[mod] = mod
        return modules.keys()
