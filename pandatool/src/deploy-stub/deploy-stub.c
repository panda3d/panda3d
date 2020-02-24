/* Python interpreter main program for frozen scripts */

#include "Python.h"
#ifdef _WIN32
#  include "malloc.h"
#  include <Shlobj.h>
#else
#  include <sys/mman.h>
#  include <pwd.h>
#endif

#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifdef __APPLE__
#  include <mach-o/dyld.h>
#  include <libgen.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>

#if PY_MAJOR_VERSION >= 3
#  include <locale.h>

#  if PY_MINOR_VERSION < 5
#    define Py_DecodeLocale _Py_char2wchar
#  endif
#endif

/* Leave room for future expansion.  We only read pointer 0, but there are
   other pointers that are being read by configPageManager.cxx. */
#define MAX_NUM_POINTERS 24

/* Stored in the flags field of the blobinfo structure below. */
enum Flags {
  F_log_append = 1,
};

/* Define an exposed symbol where we store the offset to the module data. */
#ifdef _MSC_VER
__declspec(dllexport)
#else
__attribute__((__visibility__("default"), used))
#endif
volatile struct {
  uint64_t blob_offset;
  uint64_t blob_size;
  uint16_t version;
  uint16_t num_pointers;
  uint16_t codepage;
  uint16_t flags;
  uint64_t reserved;
  void *pointers[MAX_NUM_POINTERS];

  // The reason we initialize it to -1 is because otherwise, smart linkers may
  // end up putting it in the .bss section for zero-initialized data.
} blobinfo = {(uint64_t)-1};

#ifdef MS_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

extern void PyWinFreeze_ExeInit(void);
extern void PyWinFreeze_ExeTerm(void);

static struct _inittab extensions[] = {
    {0, 0},
};

#if PY_MAJOR_VERSION >= 3
#  define WIN_UNICODE
#endif
#endif

#ifdef _WIN32
static wchar_t *log_pathw = NULL;
#endif

#if defined(_WIN32) && PY_VERSION_HEX < 0x03060000
static int supports_code_page(UINT cp) {
  if (cp == 0) {
    cp = GetACP();
  }

  /* Shortcut, because we know that these encodings are bundled by default--
   * see FreezeTool.py and Python's encodings/aliases.py */
  if (cp != 0 && cp != 1252 && cp != 367 && cp != 437 && cp != 850 && cp != 819) {
    const struct _frozen *moddef;
    char codec[100];

    /* Check if the codec was frozen into the program.  We can't check this
     * using _PyCodec_Lookup, since Python hasn't been initialized yet. */
    PyOS_snprintf(codec, sizeof(codec), "encodings.cp%u", (unsigned int)cp);

    moddef = PyImport_FrozenModules;
    while (moddef->name) {
      if (strcmp(moddef->name, codec) == 0) {
        return 1;
      }
      ++moddef;
    }
    return 0;
  }

  return 1;
}
#endif

/**
 * Sets the main_dir field of the blobinfo structure, but only if it wasn't
 * already set.
 */
static void set_main_dir(char *main_dir) {
  if (blobinfo.num_pointers >= 10) {
    if (blobinfo.num_pointers == 10) {
      ++blobinfo.num_pointers;
      blobinfo.pointers[10] = NULL;
    }
    if (blobinfo.pointers[10] == NULL) {
      blobinfo.pointers[10] = main_dir;
    }
  }
}

/**
 * Creates the parent directories of the given path.  Returns 1 on success.
 */
#ifdef _WIN32
static int mkdir_parent(const wchar_t *path) {
  // Copy the path to a temporary buffer.
  wchar_t buffer[4096];
  size_t buflen = wcslen(path);
  if (buflen + 1 >= _countof(buffer)) {
    return 0;
  }
  wcscpy_s(buffer, _countof(buffer), path);

  // Seek back to find the last path separator.
  while (buflen-- > 0) {
    if (buffer[buflen] == '/' || buffer[buflen] == '\\') {
      buffer[buflen] = 0;
      break;
    }
  }
  if (buflen == (size_t)-1 || buflen == 0) {
    // There was no path separator, or this was the root directory.
    return 0;
  }

  if (CreateDirectoryW(buffer, NULL) != 0) {
    // Success!
    return 1;
  }

  // Failed.
  DWORD last_error = GetLastError();
  if (last_error == ERROR_ALREADY_EXISTS) {
    // Not really an error: the directory is already there.
    return 1;
  }

  if (last_error == ERROR_PATH_NOT_FOUND) {
    // We need to make the parent directory first.
    if (mkdir_parent(buffer)) {
      // Parent successfully created.  Try again to make the child.
      if (CreateDirectoryW(buffer, NULL) != 0) {
        // Got it!
        return 1;
      }
    }
  }
  return 0;
}
#else
static int mkdir_parent(const char *path) {
  // Copy the path to a temporary buffer.
  char buffer[4096];
  size_t buflen = strlen(path);
  if (buflen + 1 >= sizeof(buffer)) {
    return 0;
  }
  strcpy(buffer, path);

  // Seek back to find the last path separator.
  while (buflen-- > 0) {
    if (buffer[buflen] == '/') {
      buffer[buflen] = 0;
      break;
    }
  }
  if (buflen == (size_t)-1 || buflen == 0) {
    // There was no path separator, or this was the root directory.
    return 0;
  }
  if (mkdir(buffer, 0755) == 0) {
    // Success!
    return 1;
  }

  // Failed.
  if (errno == EEXIST) {
    // Not really an error: the directory is already there.
    return 1;
  }

  if (errno == ENOENT || errno == EACCES) {
    // We need to make the parent directory first.
    if (mkdir_parent(buffer)) {
      // Parent successfully created.  Try again to make the child.
      if (mkdir(buffer, 0755) == 0) {
        // Got it!
        return 1;
      }
    }
  }
  return 0;
}
#endif

/**
 * Redirects the output streams to point to the log file with the given path.
 *
 * @param path specifies the location of log file, may start with ~
 * @param append should be nonzero if it should not truncate the log file.
 */
static int setup_logging(const char *path, int append) {
#ifdef _WIN32
  // Does it start with a tilde?  Perform tilde expansion if so.
  wchar_t *pathw = (wchar_t *)malloc(sizeof(wchar_t) * MAX_PATH);
  pathw[0] = 0;
  size_t offset = 0;
  if (path[0] == '~' && (path[1] == 0 || path[1] == '/' || path[1] == '\\')) {
    // Strip off the tilde.
    ++path;

    // Get the home directory path for the current user.
    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, pathw))) {
      free(pathw);
      return 0;
    }
    offset = wcslen(pathw);
  }

  // We need to convert the rest of the path from UTF-8 to UTF-16.
  if (MultiByteToWideChar(CP_UTF8, 0, path, -1, pathw + offset,
                          (int)(MAX_PATH - offset)) == 0) {
    free(pathw);
    return 0;
  }

  DWORD access = append ? FILE_APPEND_DATA : (GENERIC_READ | GENERIC_WRITE);
  int creation = append ? OPEN_ALWAYS : CREATE_ALWAYS;
  HANDLE handle = CreateFileW(pathw, access, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);

  if (handle == INVALID_HANDLE_VALUE) {
    // Make the parent directories first.
    mkdir_parent(pathw);
    handle = CreateFileW(pathw, access, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
  }

  if (handle == INVALID_HANDLE_VALUE) {
    free(pathw);
    return 0;
  }

  log_pathw = pathw;

  if (append) {
    SetFilePointer(handle, 0, NULL, FILE_END);
  }

  SetStdHandle(STD_OUTPUT_HANDLE, handle);
  SetStdHandle(STD_ERROR_HANDLE, handle);

  // If we are running under the UCRT in a GUI application, we can't be sure
  // that we have valid fds for stdout and stderr, so we have to set them up.
  // One way to do this is to reopen them to something silly (like NUL).
  if (_fileno(stdout) < 0) {
    _close(1);
    _wfreopen(L"\\\\.\\NUL", L"w", stdout);
  }

  if (_fileno(stderr) < 0) {
    _close(2);
    _wfreopen(L"\\\\.\\NUL", L"w", stderr);
  }

  // Now replace the stdout and stderr file descriptors with one pointing to
  // our desired handle.
  int fd = _open_osfhandle((intptr_t)handle, _O_WRONLY | _O_TEXT | _O_APPEND);
  _dup2(fd, _fileno(stdout));
  _dup2(fd, _fileno(stderr));
  _close(fd);

  return 1;
#else
  // Does it start with a tilde?  Perform tilde expansion if so.
  char buffer[PATH_MAX * 2];
  size_t offset = 0;
  if (path[0] == '~' && (path[1] == 0 || path[1] == '/')) {
    // Strip off the tilde.
    ++path;

    // Get the home directory path for the current user.
    const char *home_dir = getenv("HOME");
    if (home_dir == NULL) {
      home_dir = getpwuid(getuid())->pw_dir;
    }
    offset = strlen(home_dir);
    assert(offset < sizeof(buffer));
    strncpy(buffer, home_dir, sizeof(buffer));
  }

  // Copy over the rest of the path.
  strcpy(buffer + offset, path);

  mode_t mode = O_CREAT | O_WRONLY | (append ? O_APPEND : O_TRUNC);
  int fd = open(buffer, mode, 0644);
  if (fd == -1) {
    // Make the parent directories first.
    mkdir_parent(buffer);
    fd = open(buffer, mode, 0644);
  }

  if (fd == -1) {
    perror(buffer);
    return 0;
  }

  fflush(stdout);
  fflush(stderr);

  dup2(fd, 1);
  dup2(fd, 2);

  close(fd);
  return 1;
#endif
}

/* Main program */

#ifdef WIN_UNICODE
int Py_FrozenMain(int argc, wchar_t **argv)
#else
int Py_FrozenMain(int argc, char **argv)
#endif
{
    char *p;
    int n, sts = 1;
    int inspect = 0;
    int unbuffered = 0;

#if PY_MAJOR_VERSION >= 3 && !defined(WIN_UNICODE)
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

#if defined(MS_WINDOWS) && PY_VERSION_HEX >= 0x03040000 && PY_VERSION_HEX < 0x03060000
    if (!supports_code_page(GetConsoleOutputCP()) ||
        !supports_code_page(GetConsoleCP())) {
      /* Revert to the active codepage, and tell Python to use the 'mbcs'
       * encoding (which always uses the active codepage).  In 99% of cases,
       * this will be the same thing anyway. */
      UINT acp = GetACP();
      SetConsoleCP(acp);
      SetConsoleOutputCP(acp);
      Py_SetStandardStreamEncoding("mbcs", NULL);
    }
#endif

    Py_FrozenFlag = 1; /* Suppress errors from getpath.c */
    Py_NoSiteFlag = 0;
    Py_NoUserSiteDirectory = 1;

    if ((p = Py_GETENV("PYTHONINSPECT")) && *p != '\0')
        inspect = 1;
    if ((p = Py_GETENV("PYTHONUNBUFFERED")) && *p != '\0')
        unbuffered = 1;

    if (unbuffered) {
        setbuf(stdin, (char *)NULL);
        setbuf(stdout, (char *)NULL);
        setbuf(stderr, (char *)NULL);
    }

#if PY_MAJOR_VERSION >= 3 && !defined(WIN_UNICODE)
    oldloc = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, "");
    for (i = 0; i < argc; i++) {
        argv_copy[i] = Py_DecodeLocale(argv[i], NULL);
        argv_copy2[i] = argv_copy[i];
        if (!argv_copy[i]) {
            fprintf(stderr, "Unable to decode the command line argument #%i\n",
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
#if PY_MAJOR_VERSION >= 3 && !defined(WIN_UNICODE)
        Py_SetProgramName(argv_copy[0]);
#else
        Py_SetProgramName(argv[0]);
#endif
    }

    Py_Initialize();
#ifdef MS_WINDOWS
    PyWinFreeze_ExeInit();
#endif

#if defined(MS_WINDOWS) && PY_VERSION_HEX < 0x03040000
    /* We can't rely on our overriding of the standard I/O to work on older
     * versions of Python, since they are compiled with an incompatible CRT.
     * The best solution I've found was to just replace sys.stdout/stderr with
     * the log file reopened in append mode (which requires not locking it for
     * write, and also passing in _O_APPEND above, and disabling buffering).
     * It's not the most elegant solution, but it's better than crashing. */
#if PY_MAJOR_VERSION < 3
    if (log_pathw != NULL) {
      PyObject *uniobj = PyUnicode_FromWideChar(log_pathw, (Py_ssize_t)wcslen(log_pathw));
      PyObject *file = PyObject_CallFunction((PyObject*)&PyFile_Type, "Nsi", uniobj, "a", 0);

      if (file != NULL) {
        PyFile_SetEncodingAndErrors(file, "utf-8", NULL);

        PySys_SetObject("stdout", file);
        PySys_SetObject("stderr", file);
        PySys_SetObject("__stdout__", file);
        PySys_SetObject("__stderr__", file);

        /* Be sure to disable buffering, otherwise we'll get overlap */
        setbuf(stdout, (char *)NULL);
        setbuf(stderr, (char *)NULL);
      }
    }
    else
#endif
    if (!supports_code_page(GetConsoleOutputCP()) ||
        !supports_code_page(GetConsoleCP())) {
      /* Same hack as before except for Python 2.7, which doesn't seem to have
       * a way to set the encoding ahead of time, and setting PYTHONIOENCODING
       * doesn't seem to work.  Fortunately, Python 2.7 doesn't usually start
       * causing codec errors until the first print statement. */
      PyObject *sys_stream;
      UINT acp = GetACP();
      SetConsoleCP(acp);
      SetConsoleOutputCP(acp);

      sys_stream = PySys_GetObject("stdin");
      if (sys_stream && PyFile_Check(sys_stream)) {
        PyFile_SetEncodingAndErrors(sys_stream, "mbcs", NULL);
      }
      sys_stream = PySys_GetObject("stdout");
      if (sys_stream && PyFile_Check(sys_stream)) {
        PyFile_SetEncodingAndErrors(sys_stream, "mbcs", NULL);
      }
      sys_stream = PySys_GetObject("stderr");
      if (sys_stream && PyFile_Check(sys_stream)) {
        PyFile_SetEncodingAndErrors(sys_stream, "mbcs", NULL);
      }
    }
#endif

    if (Py_VerboseFlag)
        fprintf(stderr, "Python %s\n%s\n",
            Py_GetVersion(), Py_GetCopyright());

#if PY_MAJOR_VERSION >= 3 && !defined(WIN_UNICODE)
    PySys_SetArgv(argc, argv_copy);
#else
    PySys_SetArgv(argc, argv);
#endif

#ifdef MACOS_APP_BUNDLE
    // Add the Frameworks directory to sys.path.
    char buffer[PATH_MAX];
    uint32_t bufsize = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &bufsize) != 0) {
      assert(false);
      return 1;
    }
    char resolved[PATH_MAX];
    if (!realpath(buffer, resolved)) {
      perror("realpath");
      return 1;
    }
    const char *dir = dirname(resolved);
    sprintf(buffer, "%s/../Frameworks", dir);

    PyObject *sys_path = PyList_New(1);
  #if PY_MAJOR_VERSION >= 3
    PyList_SET_ITEM(sys_path, 0, PyUnicode_FromString(buffer));
  #else
    PyList_SET_ITEM(sys_path, 0, PyString_FromString(buffer));
  #endif
    PySys_SetObject("path", sys_path);
    Py_DECREF(sys_path);

    // Now, store a path to the Resources directory into the main_dir pointer,
    // for ConfigPageManager to read out and assign to MAIN_DIR.
    sprintf(buffer, "%s/../Resources", dir);
    set_main_dir(buffer);
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

#if PY_MAJOR_VERSION >= 3 && !defined(WIN_UNICODE)
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

/**
 * Maps the binary blob at the given memory address to memory, and returns the
 * pointer to the beginning of it.
 */
static void *map_blob(off_t offset, size_t size) {
  void *blob;
  FILE *runtime;

#ifdef _WIN32
  wchar_t buffer[2048];
  GetModuleFileNameW(NULL, buffer, 2048);
  runtime = _wfopen(buffer, L"rb");
#elif defined(__FreeBSD__)
  size_t bufsize = 4096;
  char buffer[4096];
  int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
  mib[3] = getpid();
  if (sysctl(mib, 4, (void *)buffer, &bufsize, NULL, 0) == -1) {
    perror("sysctl");
    return NULL;
  }
  runtime = fopen(buffer, "rb");
#elif defined(__APPLE__)
  char buffer[4096];
  uint32_t bufsize = sizeof(buffer);
  if (_NSGetExecutablePath(buffer, &bufsize) != 0) {
    return NULL;
  }
  runtime = fopen(buffer, "rb");
#else
  char buffer[4096];
  ssize_t pathlen = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (pathlen <= 0) {
    perror("readlink(/proc/self/exe)");
    return NULL;
  }
  buffer[pathlen] = '\0';
  runtime = fopen(buffer, "rb");
#endif

  // Get offsets.  In version 0, we read it from the end of the file.
  if (blobinfo.version == 0) {
    uint64_t end, begin;
    fseek(runtime, -8, SEEK_END);
    end = ftell(runtime);
    fread(&begin, 8, 1, runtime);

    offset = (off_t)begin;
    size = (size_t)(end - begin);
  }

  // mmap the section indicated by the offset (or malloc/fread on windows)
#ifdef _WIN32
  blob = (void *)malloc(size);
  assert(blob != NULL);
  fseek(runtime, (long)offset, SEEK_SET);
  fread(blob, size, 1, runtime);
#else
  blob = (void *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(runtime), offset);
  assert(blob != MAP_FAILED);
#endif

  fclose(runtime);
  return blob;
}

/**
 * The inverse of map_blob.
 */
static void unmap_blob(void *blob) {
  if (blob) {
#ifdef _WIN32
    free(blob);
#else
    munmap(blob, blobinfo.blob_size);
#endif
  }
}

/**
 * Main entry point to deploy-stub.
 */
#if defined(_WIN32) && PY_MAJOR_VERSION >= 3
int wmain(int argc, wchar_t *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
  int retval;
  struct _frozen *moddef;
  const char *log_filename;
  void *blob = NULL;
  log_filename = NULL;

#ifdef __APPLE__
  // Strip a -psn_xxx argument passed in by macOS when run from an .app bundle.
  if (argc > 1 && strncmp(argv[1], "-psn_", 5) == 0) {
    argv[1] = argv[0];
    ++argv;
    --argc;
  }
#endif

  /*
  printf("blob_offset: %d\n", (int)blobinfo.blob_offset);
  printf("blob_size: %d\n", (int)blobinfo.blob_size);
  printf("version: %d\n", (int)blobinfo.version);
  printf("num_pointers: %d\n", (int)blobinfo.num_pointers);
  printf("codepage: %d\n", (int)blobinfo.codepage);
  printf("flags: %d\n", (int)blobinfo.flags);
  printf("reserved: %d\n", (int)blobinfo.reserved);
  */

  // If we have a blob offset, we have to map the blob to memory.
  if (blobinfo.version == 0 || blobinfo.blob_offset != 0) {
    void *blob = map_blob((off_t)blobinfo.blob_offset, (size_t)blobinfo.blob_size);
    assert(blob != NULL);

    // Offset the pointers in the header using the base mmap address.
    if (blobinfo.version > 0 && blobinfo.num_pointers > 0) {
      uint32_t i;
      assert(blobinfo.num_pointers <= MAX_NUM_POINTERS);
      for (i = 0; i < blobinfo.num_pointers; ++i) {
        // Only offset if the pointer is non-NULL.  Except for the first
        // pointer, which may never be NULL and usually (but not always)
        // points to the beginning of the blob.
        if (i == 0 || blobinfo.pointers[i] != 0) {
          blobinfo.pointers[i] = (void *)((uintptr_t)blobinfo.pointers[i] + (uintptr_t)blob);
        }
      }
      if (blobinfo.num_pointers >= 12) {
        log_filename = blobinfo.pointers[11];
      }
    } else {
      blobinfo.pointers[0] = blob;
    }

    // Offset the pointers in the module table using the base mmap address.
    moddef = blobinfo.pointers[0];
    while (moddef->name) {
      moddef->name = (char *)((uintptr_t)moddef->name + (uintptr_t)blob);
      if (moddef->code != 0) {
        moddef->code = (unsigned char *)((uintptr_t)moddef->code + (uintptr_t)blob);
      }
      //printf("MOD: %s %p %d\n", moddef->name, (void*)moddef->code, moddef->size);
      moddef++;
    }
  }

  if (log_filename != NULL) {
    setup_logging(log_filename, (blobinfo.flags & F_log_append) != 0);
  }

#ifdef _WIN32
  if (blobinfo.codepage != 0) {
    SetConsoleCP(blobinfo.codepage);
    SetConsoleOutputCP(blobinfo.codepage);
  }
#endif

  // Run frozen application
  PyImport_FrozenModules = blobinfo.pointers[0];
  retval = Py_FrozenMain(argc, argv);

  unmap_blob(blob);
  return retval;
}

#ifdef WIN_UNICODE
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t *lpCmdLine, int nCmdShow) {
  return wmain(__argc, __wargv);
}
#elif defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpCmdLine, int nCmdShow) {
  return main(__argc, __argv);
}
#endif
