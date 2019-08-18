/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayapath.cxx
 * @author drose
 * @date 2008-04-07
 */

// This program works as a stub to launch maya2egg, egg2maya, and similar
// programs that invoke OpenMaya and require certain environment variables to
// be set first.

// It used to duplicate code in mayaWrapper.cxx, but now the functionality for
// these two separate programs are unified here.

// If MAYAVERSION is defined at the time this is compiled, then that
// particular version of Maya is insisted upon, and the desired Maya location
// is found in the Registry; otherwise, we require that $MAYA_LOCATION be set
// at runtime and points to the desired Maya installation.

/*
 * If MAYAVERSION is defined and $MAYA_LOCATION is also set, then we check
 * that definition of $MAYA_LOCATION is reasonable, which we define as
 * pointing to the same version of OpenMaya.dll.  If so, then we use the
 * runtime $MAYA_LOCATION, allowing the user to (slightly) override the
 * runtime Maya directory.  If $MAYA_LOCATION is set but points to a different
 * version of OpenMaya.dll, we ignore it altogether and replace it with our
 * registry data, which allows the user to have MAYA_LOCATION pointing to a
 * different version of Maya without interfering with this program.
 */

#include "dtoolbase.h"
#include "filename.h"
#include "globPattern.h"
#include "dSearchPath.h"
#include "executionEnvironment.h"
#include "hashVal.h"
#include <stdlib.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#endif

using std::cerr;
using std::endl;
using std::string;

#define QUOTESTR(x) #x
#define TOSTRING(x) QUOTESTR(x)

#if defined(_WIN32)
// Note: Filename::dso_filename changes .so to .dll automatically.
static const Filename openmaya_filename = "bin/OpenMaya.so";
#elif defined(IS_OSX)
static const Filename openmaya_filename = "MacOS/libOpenMaya.dylib";
#else
static const Filename openmaya_filename = "lib/libOpenMaya.so";
#endif  // _WIN32

// Searches for python26.zip or whatever version it is.
static Filename
find_pyzip(const Filename &maya_location) {
  // This is where python26.zip appears on Windows.  Should it be in other
  // locations on other platforms?
  Filename dirname(maya_location, "bin");

  vector_string results;
  GlobPattern glob("python*.zip");
  if (glob.match_files(results, dirname) != 0) {
    return Filename(dirname, results[0]);
  }

  return Filename();
}

struct MayaVerInfo {
  const char *ver, *key;
};

struct MayaVerInfo maya_versions[] = {
  { "MAYA6",    "6.0" },
  { "MAYA65",   "6.5" },
  { "MAYA7",    "7.0" },
  { "MAYA8",    "8.0" },
  { "MAYA85",   "8.5" },
  { "MAYA2008", "2008"},
  { "MAYA2009", "2009"},
  { "MAYA2010", "2010"},
  { "MAYA2011", "2011"},
  { "MAYA2012", "2012"},
  { "MAYA2013", "2013"},
  { "MAYA20135", "2013.5"},
  { "MAYA2014", "2014"},
  { "MAYA2015", "2015"},
  { "MAYA2016", "2016"},
  { "MAYA20165", "2016.5"},
  { "MAYA2017", "2017"},
  { "MAYA2018", "2018"},
  { "MAYA2019", "2019"},
  { 0, 0 },
};

static const char *
get_version_number(const char *ver) {
  for (int i = 0; maya_versions[i].ver != 0; ++i) {
    if (strcmp(maya_versions[i].ver, ver) == 0) {
      return maya_versions[i].key;
    }
  }
  return 0;
}

#if defined(_WIN32)
static void
get_maya_location(const char *ver, string &loc) {
  char fullkey[1024];
  const char *developer;
  LONG res;

  for (int dev=0; dev<3; dev++) {
    switch (dev) {
    case 0: developer="Alias|Wavefront"; break;
    case 1: developer="Alias"; break;
    case 2: developer="Autodesk"; break;
    }
    sprintf(fullkey, "SOFTWARE\\%s\\Maya\\%s\\Setup\\InstallPath", developer, ver);
    for (int hive=0; hive<2; hive++) {
      HKEY hkey;
      res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fullkey, 0, KEY_READ | (hive ? 256:0), &hkey);
      if (res == ERROR_SUCCESS) {
        DWORD dtype;
        DWORD size = 4096;
        char result[4096 + 1];
        res = RegQueryValueEx(hkey, "MAYA_INSTALL_LOCATION", nullptr, &dtype, (LPBYTE)result, &size);
        if ((res == ERROR_SUCCESS)&&(dtype == REG_SZ)) {
          result[size] = 0;
          loc = result;
        }
        RegCloseKey(hkey);
      }
    }
  }
}

#elif defined(__APPLE__)
static void
get_maya_location(const char *ver, string &loc) {
  char mpath[64];
  sprintf(mpath, "/Applications/Autodesk/maya%s/Maya.app/Contents", ver);
  struct stat st;
  if(stat(mpath, &st) == 0) {
    loc = mpath;
  }
}

#else  // _WIN32
static void
get_maya_location(const char *ver, string &loc) {
  char mpath[64];
#if __WORDSIZE == 64
  sprintf(mpath, "/usr/autodesk/maya%s-x64", ver);
#else
  sprintf(mpath, "/usr/autodesk/maya%s", ver);
#endif
  struct stat st;
  if(stat(mpath, &st) == 0) {
    loc = mpath;
  } else {
#if __WORDSIZE == 64
    sprintf(mpath, "/usr/aw/maya%s-x64", ver);
#else
    sprintf(mpath, "/usr/aw/maya%s", ver);
#endif
    if(stat(mpath, &st) == 0) {
      loc = mpath;
    }
  }
}

#endif  // _WIN32


int
main(int argc, char *argv[]) {
  // First, get the command line and append _bin, so we will actually run
  // maya2egg_bin.exe, egg2maya_bin.exe, etc.
  Filename command = Filename::from_os_specific(argv[0]);
  if (!command.is_fully_qualified()) {
    DSearchPath path;
    path.append_path(ExecutionEnvironment::get_environment_variable("PATH"));
#ifdef _WIN32
    command.set_extension("exe");
#endif
    command.resolve_filename(path);
  }

#ifdef _WIN32
  if (command.get_extension() == "exe") {
    command.set_extension("");
  }
#endif

  command = command.get_fullpath() + string("_bin");
#ifdef _WIN32
  command.set_extension("exe");
#endif
  string os_command = command.to_os_specific();

  // First start with $PANDA_MAYA_LOCATION.  If it is set, it overrides
  // everything else.
  Filename maya_location = Filename::expand_from("$PANDA_MAYA_LOCATION");
  if (!maya_location.empty()) {
    // Reset maya_location to its full long name, because Maya requires this.
    maya_location.make_canonical();
    maya_location = Filename::from_os_specific(maya_location.to_os_long_name());

  } else {
    // $PANDA_MAYA_LOCATION wasn't set, so check the normal locations.  First,
    // we get the standard location, as a point of reference.
    Filename standard_maya_location;
#ifdef MAYAVERSION
    const char *key = get_version_number(TOSTRING(MAYAVERSION));
    if (key == nullptr) {
      cerr << "Unknown Maya version: " << TOSTRING(MAYAVERSION) << "\n";
    } else {
      string loc;
      get_maya_location(key, loc);
      if (loc.empty()) {
        cerr << "Cannot locate " << TOSTRING(MAYAVERSION) << ": it does not appear to be installed.\n";
      } else {
        standard_maya_location = Filename::from_os_specific(loc);
      }
    }
    if (!standard_maya_location.empty()) {
      // Reset standard_maya_location to its full long name, so we can compare
      // reliably to the given version.
      standard_maya_location.make_canonical();
      standard_maya_location = Filename::from_os_specific(standard_maya_location.to_os_long_name());
    }
#endif  // MAYAVERSION

    // Now check if $MAYA_LOCATION is set.  If it is, and it's consistent with
    // the standard location, we respect it.
    maya_location = Filename::expand_from("$MAYA_LOCATION");
    if (!maya_location.empty()) {
      // Reset maya_location to its full long name, so we can compare it
      // reliably to the standard location; and also because Maya requires
      // this.
      maya_location.make_canonical();
      maya_location = Filename::from_os_specific(maya_location.to_os_long_name());
    }

    if (maya_location.empty()) {
      // If it is not set, we use the standard version instead.
      maya_location = standard_maya_location;

    } else if (maya_location != standard_maya_location) {
      // If it *is* set, we verify that OpenMaya.dll matches the standard
      // version.
      Filename openmaya_given = Filename::dso_filename(Filename(maya_location, openmaya_filename));
      Filename openmaya_standard = Filename::dso_filename(Filename(standard_maya_location, openmaya_filename));

      if (openmaya_given != openmaya_standard) {
#ifdef HAVE_OPENSSL
        // If we have OpenSSL, we can use it to check the md5 hashes of the
        // DLL.
        HashVal hash_given, hash_standard;
        if (!hash_standard.hash_file(openmaya_standard)) {
          // Couldn't read the standard file, so use the given one.

        } else {
          if (!hash_given.hash_file(openmaya_given)) {
            // Couldn't even read the given file; use the standard one
            // instead.
            maya_location = standard_maya_location;

          } else {
            if (hash_standard != hash_given) {
              // No match; it must be the wrong version.
              cerr << "$MAYA_LOCATION points to wrong version; using standard location instead.\n";
              maya_location = standard_maya_location;
            } else {
              // The hash matches; keep the given MAYA_LOCATION setting.
            }
          }
        }
#else // HAVE_OPENSSL
      // Without OpenSSL, just check the DLL filesize only.
        off_t size_given, size_standard;
        size_standard = openmaya_standard.get_file_size();
        if (size_standard == 0) {
          // Couldn't read the standard file, so use the given one.

        } else {
          size_given = openmaya_given.get_file_size();
          if (size_given == 0) {
            // Couldn't even read the given file; use the standard one
            // instead.
            maya_location = standard_maya_location;

          } else {
            if (size_standard != size_given) {
              // No match; it must be the wrong version.
              cerr << "$MAYA_LOCATION points to wrong version; using standard location instead.\n";
              maya_location = standard_maya_location;

            } else {
              // The size matches; keep the given MAYA_LOCATION setting.
            }
          }
        }

#endif  // HAVE_OPENSSL
      }
    }
  }

  if (maya_location.empty()) {
    cerr << "$MAYA_LOCATION is not set!\n";
    exit(1);
  }

  cerr << "MAYA_LOCATION: " << maya_location.to_os_specific() << endl;
  if (!maya_location.is_directory()) {
    cerr << "The directory referred to by $MAYA_LOCATION does not exist!\n";
    exit(1);
  }

  // Look for OpenMaya.dll as a sanity check.
  Filename openmaya = Filename::dso_filename(Filename(maya_location, openmaya_filename));
  if (!openmaya.is_regular_file()) {
    cerr << "Could not find $MAYA_LOCATION/" << Filename::dso_filename(openmaya_filename).to_os_specific() << "!\n";
    exit(1);
  }

  // Re-set MAYA_LOCATION to its properly sanitized form.
  {
    string putenv_str = "MAYA_LOCATION=" + maya_location.to_os_specific();
    char *putenv_cstr = strdup(putenv_str.c_str());
    putenv(putenv_cstr);
  }

#ifdef WIN32
  string sep = ";";
#else
  string sep = ":";
#endif

  // Now set PYTHONHOME & PYTHONPATH.  Maya2008 requires this to be set and
  // pointing within $MAYA_LOCATION, or it might get itself confused with
  // another Python installation (e.g.  Panda's).
  Filename python = Filename(maya_location, "Python");
  if (python.is_directory()) {
    {
      string putenv_str = "PYTHONHOME=" + python.to_os_specific();
      char *putenv_cstr = strdup(putenv_str.c_str());
      putenv(putenv_cstr);
    }
    {
      string putenv_str = "PYTHONPATH=" + python.to_os_specific();

      Filename pyzip = find_pyzip(maya_location);
      if (!pyzip.empty() && pyzip.exists()) {
        putenv_str += sep;
        putenv_str += pyzip.to_os_specific();
      }

      Filename site_packages(python, "lib/site-packages");
      if (site_packages.is_directory()) {
        putenv_str += sep;
        putenv_str += site_packages.to_os_specific();
      }

      char *putenv_cstr = strdup(putenv_str.c_str());
      putenv(putenv_cstr);
    }
  }

  // Also put the Maya bin directory on the PATH.
#ifdef IS_OSX
  Filename bin = Filename(maya_location, "MacOS");
#else
  Filename bin = Filename(maya_location, "bin");
#endif
  if (bin.is_directory()) {
    const char *path = getenv("PATH");
    if (path == nullptr) {
      path = "";
    }
    string putenv_str = "PATH=" + bin.to_os_specific() + sep + path;
    char *putenv_cstr = strdup(putenv_str.c_str());
    putenv(putenv_cstr);
  }

#ifdef IS_OSX
  // And on DYLD_LIBRARY_PATH.
  if (bin.is_directory()) {
    const char *path = getenv("DYLD_LIBRARY_PATH");
    if (path == nullptr) {
      path = "";
    }
    string sep = ":";
    string putenv_str = "DYLD_LIBRARY_PATH=" + bin.to_os_specific() + sep + path;
    char *putenv_cstr = strdup(putenv_str.c_str());
    putenv(putenv_cstr);
  }

  // We also have to give it a way to find the Maya frameworks.
  Filename fw_dir = Filename(maya_location, "Frameworks");
  if (fw_dir.is_directory()) {
    const char *path = getenv("DYLD_FALLBACK_FRAMEWORK_PATH");
    if (path == nullptr) {
      path = "";
    }
    string sep = ":";
    string putenv_str = "DYLD_FALLBACK_FRAMEWORK_PATH=" + fw_dir.to_os_specific() + sep + path;
    char *putenv_cstr = strdup(putenv_str.c_str());
    putenv(putenv_cstr);
  }

#elif !defined(_WIN32)
  // Linux (or other non-Windows OS) gets it added to LD_LIBRARY_PATH.
  if (bin.is_directory()) {
    const char *path = getenv("LD_LIBRARY_PATH");
    if (path == nullptr) {
      path = "";
    }
    string sep = ":";
    string putenv_str = "LD_LIBRARY_PATH=" + bin.to_os_specific() + sep + path;
    char *putenv_cstr = strdup(putenv_str.c_str());
    putenv(putenv_cstr);
  }

#endif // IS_OSX

  // Now that we have set up the environment variables properly, chain to the
  // actual maya2egg_bin (or whichever) executable.

#ifdef _WIN32
  // Windows case.
  char *command_line = strdup(GetCommandLine());
  STARTUPINFO startup_info;
  PROCESS_INFORMATION process_info;
  GetStartupInfo(&startup_info);
  BOOL result = CreateProcess(os_command.c_str(),
                              command_line,
                              nullptr, nullptr, true, 0,
                              nullptr, nullptr,
                              &startup_info,
                              &process_info);
  if (result) {
    WaitForSingleObject(process_info.hProcess, INFINITE);
    DWORD exit_code = 0;

    if (GetExitCodeProcess(process_info.hProcess, &exit_code)) {
      if (exit_code != 0) {
        cerr << "Program exited with status " << exit_code << "\n";
      }
    }

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
    exit(exit_code);
  }
  cerr << "Couldn't execute " << command << ": " << GetLastError() << "\n";

#else
  // Unix case.
  execvp(os_command.c_str(), argv);
#endif

  // Couldn't execute for some reason.
  return 1;
}
