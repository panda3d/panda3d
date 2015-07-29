// Filename: p3dInstanceManager.cxx
// Created by:  drose (29May09)
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

#include "p3dInstanceManager.h"
#include "p3dInstance.h"
#include "p3dSession.h"
#include "p3dAuthSession.h"
#include "p3dHost.h"
#include "p3d_plugin_config.h"
#include "p3dWinSplashWindow.h"
#include "p3dUndefinedObject.h"
#include "p3dNoneObject.h"
#include "p3dBoolObject.h"
#include "p3dPackage.h"
#include "find_root_dir.h"
#include "fileSpec.h"
#include "get_tinyxml.h"
#include "binaryXml.h"
#include "mkdir_complete.h"
#include "wstring_encode.h"

// We can include this header file to get the DTOOL_PLATFORM
// definition, even though we don't link with dtool.
#include "dtool_platform.h"

#ifdef _WIN32
#include <shlobj.h>
#include <io.h>      // chmod()
#include <direct.h>  // rmdir()
#include <windows.h> // GetModuleHandle() etc.
#else
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#endif

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#include <stdio.h>

static ofstream logfile;
ostream *nout_stream = &logfile;

P3DInstanceManager *P3DInstanceManager::_global_ptr;

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstanceManager::
P3DInstanceManager() {
  init_xml();

  _is_initialized = false;
  _created_runtime_environment = false;
  _api_version = 0;
  _next_temp_filename_counter = 1;
  _unique_id = 0;
  _trusted_environment = false;
  _console_environment = false;

  _plugin_major_version = 0;
  _plugin_minor_version = 0;
  _plugin_sequence_version = 0;
  _plugin_official_version = false;
  _coreapi_timestamp = 0;

  _notify_thread_continue = false;
  _started_notify_thread = false;
  INIT_THREAD(_notify_thread);

  // Initialize the singleton objects.
  _undefined_object = new P3DUndefinedObject();
  _none_object = new P3DNoneObject();
  _true_object = new P3DBoolObject(true);
  _false_object = new P3DBoolObject(false);

  _auth_session = NULL;

  // Seed the lame random number generator in rand(); we use it to
  // select a mirror for downloading.
  srand((unsigned int)time(NULL));

#ifdef _WIN32
  // Ensure the appropriate Windows common controls are available to
  // this application.
  INITCOMMONCONTROLSEX icc;
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_PROGRESS_CLASS;
  InitCommonControlsEx(&icc);
#endif

#ifndef _WIN32
  // On Mac or Linux, we'd better ignore SIGPIPE, or this signal will
  // shut down the browser if the plugin exits unexpectedly.
  struct sigaction ignore;
  memset(&ignore, 0, sizeof(ignore));
  ignore.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &ignore, &_old_sigpipe);
#endif  // _WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::Destructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstanceManager::
~P3DInstanceManager() {
  if (_started_notify_thread) {
    _notify_ready.acquire();
    _notify_thread_continue = false;
    _notify_ready.notify();
    _notify_ready.release();
    JOIN_THREAD(_notify_thread);
    _started_notify_thread = false;
  }

#ifndef _WIN32
  // Restore the original SIGPIPE handler.
  sigaction(SIGPIPE, &_old_sigpipe, NULL);
#endif  // _WIN32

  // force-finish any remaining instances.
  while (!_instances.empty()) {
    P3DInstance *inst = *(_instances.begin());
    finish_instance(inst);
  }

  assert(_sessions.empty());
  assert(_instances.empty());

  if (_auth_session != NULL) {
    p3d_unref_delete(_auth_session);
    _auth_session = NULL;
  }

  Hosts::iterator hi;
  for (hi = _hosts.begin(); hi != _hosts.end(); ++hi) {
    delete (*hi).second;
  }
  _hosts.clear();

  // Delete any remaining temporary files.
  TempFilenames::iterator ti;
  for (ti = _temp_filenames.begin(); ti != _temp_filenames.end(); ++ti) {
    const string &filename = (*ti);
    nout << "Removing delinquent temp file " << filename << "\n";
    unlink(filename.c_str());
  }
  _temp_filenames.clear();

  nout << "counts: " << _undefined_object->_ref_count
       << " " << _none_object->_ref_count
       << " " << _true_object->_ref_count
       << " " << _false_object->_ref_count
       << "\n";

  /*
  assert(_undefined_object->_ref_count == 1);
  assert(_none_object->_ref_count == 1);
  assert(_true_object->_ref_count == 1);
  assert(_false_object->_ref_count == 1);
  */

  P3D_OBJECT_DECREF(_undefined_object);
  P3D_OBJECT_DECREF(_none_object);
  P3D_OBJECT_DECREF(_true_object);
  P3D_OBJECT_DECREF(_false_object);

#ifdef _WIN32
  P3DWinSplashWindow::unregister_window_class();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::initialize
//       Access: Public
//  Description: Called by the plugin host at application startup.  It
//               returns true if the DLL is successfully initialized,
//               false if it should be immediately shut down and
//               redownloaded.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
initialize(int api_version, const string &contents_filename, 
           const string &host_url, P3D_verify_contents verify_contents,
           const string &platform, const string &log_directory,
           const string &log_basename, bool trusted_environment,
           bool console_environment,
           const string &root_dir, const string &host_dir) {
  _api_version = api_version;
  _host_url = host_url;
  _verify_contents = verify_contents;
  _platform = platform;
  _log_directory = log_directory;
  _log_basename = log_basename;
  _trusted_environment = trusted_environment;
  _console_environment = console_environment;

  if (_host_url.empty()) {
    _host_url = PANDA_PACKAGE_HOST_URL;
  }

  if (_platform.empty()) {
    // If the platform is compiled in (as opposed to passed in by the
    // caller), we might in fact support multiple platforms.
    _platform = DTOOL_PLATFORM;
#ifdef _WIN32
    if (_platform == "win_amd64") {
      _supported_platforms.push_back("win_amd64");
      _supported_platforms.push_back("win_i386");
      _supported_platforms.push_back("win32");

    } else if (_platform == "win_i386" || _platform == "win32") {
      // This is a WIN32 process, but determine if the underlying OS
      // actually supports WIN64.
      if (supports_win64()) {
        _supported_platforms.push_back("win_amd64");
      }
      _supported_platforms.push_back("win_i386");
      _supported_platforms.push_back("win32");
    }
#elif defined(__APPLE__)
    if (_platform == "osx_amd64") {
      _supported_platforms.push_back("osx_amd64");
      _supported_platforms.push_back("osx_i386");

    } else if (_platform == "osx_i386") {
      // This is a 32-bit process, but determine if the underlying OS
      // supports 64-bit.

      int mib[2] = { CTL_HW, HW_MACHINE };
      char machine[512];
      size_t len = 511;
      if (sysctl(mib, 2, (void *)machine, &len, NULL, 0) == 0) {
        if (strcmp(machine, "x86_64") == 0) {
          _supported_platforms.push_back("osx_amd64");
        }
      }

      _supported_platforms.push_back("osx_i386");
    }
#endif  // _WIN32

    // TODO: Linux multiplatform support.  Just add the
    // appropriate platform strings to _supported_platforms.
  }

  if (_supported_platforms.empty()) {
    // We always support at least the specific platform on which we're
    // running.
    _supported_platforms.push_back(_platform);
  }
      
#ifdef P3D_PLUGIN_LOG_DIRECTORY
  if (_log_directory.empty()) {
    _log_directory = P3D_PLUGIN_LOG_DIRECTORY;
  }
#endif

#ifdef P3D_PLUGIN_LOG_BASENAME2
  if (_log_basename.empty()) {
    _log_basename = P3D_PLUGIN_LOG_BASENAME2;
  }
#endif
  if (_log_basename.empty()) {
    _log_basename = "p3dcore";
  }

  if (root_dir.empty()) {
    _root_dir = find_root_dir();
    if (_root_dir.empty()) {
      cerr << "Could not find root directory.\n";
      return false;
    }
  } else {
    _root_dir = root_dir;
  }
  
  _host_dir = host_dir;

  // Allow the caller (e.g. panda3d.exe) to specify a log directory.
  // Or, allow the developer to compile one in.
  
  // Failing that, we write logfiles to Panda3D/log.
  if (_log_directory.empty()) {
    _log_directory = _root_dir + "/log";
  }

  // Ensure that the log directory ends with a slash.
  if (!_log_directory.empty() && _log_directory[_log_directory.size() - 1] != '/') {
#ifdef _WIN32
    if (_log_directory[_log_directory.size() - 1] != '\\')
#endif
      _log_directory += "/";
  }

  // Construct the logfile pathname.
  _log_pathname = _log_directory;
  _log_pathname += _log_basename;
  _log_pathname += ".log";

  create_runtime_environment();
  _is_initialized = true;

  if (!host_url.empty() && !contents_filename.empty()) {
    // Attempt to pre-read the supplied contents.xml file, to avoid an
    // unnecessary download later.
    P3DHost *host = get_host(host_url);
    if (!host->read_contents_file(contents_filename, false)) {
      nout << "Couldn't read " << contents_filename << "\n";
    }
  }

  nout << "Supported platforms:";
  for (size_t pi = 0; pi < _supported_platforms.size(); ++pi) {
    nout << " " << _supported_platforms[pi];
  }
  nout << "\n";

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::set_plugin_version
//       Access: Public
//  Description: Specifies the version of the calling plugin, for
//               reporting to JavaScript and the like.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
set_plugin_version(int major, int minor, int sequence,
                   bool official, const string &distributor,
                   const string &coreapi_host_url,
                   time_t coreapi_timestamp,
                   const string &coreapi_set_ver) {
  reconsider_runtime_environment();
  _plugin_major_version = major;
  _plugin_minor_version = minor;
  _plugin_sequence_version = sequence;
  _plugin_official_version = official;
  _plugin_distributor = distributor;

  // The Core API "host URL" is both compiled in, and comes in
  // externally; we trust the external source in the case of a
  // conflict.
  string internal_host_url = PANDA_PACKAGE_HOST_URL;
  if (coreapi_host_url != internal_host_url) {
    nout << "Warning!  Downloaded Core API from " << coreapi_host_url
         << ", but its internal URL was " << internal_host_url << "\n";
  }
  _coreapi_host_url = coreapi_host_url;
  if (_coreapi_host_url.empty()) {
    _coreapi_host_url = internal_host_url;
  }

  // The Core API timestamp is only available externally.
  _coreapi_timestamp = coreapi_timestamp;

  // The Core API "set ver", or version, is both compiled in and comes
  // in externally; for this one we trust the internal version in the
  // case of a conflict.
  string internal_set_ver = P3D_COREAPI_VERSION_STR;
  if (coreapi_set_ver != internal_set_ver && !coreapi_set_ver.empty() && !internal_set_ver.empty()) {
    nout << "Warning!  contents.xml reports Core API version number "
         << coreapi_set_ver << ", but its actual version number is " 
         << internal_set_ver << "\n";
  }
  _coreapi_set_ver = internal_set_ver;
  if (_coreapi_set_ver.empty()) {
    _coreapi_set_ver = coreapi_set_ver;
  }

  nout << "Plugin version: "
       << _plugin_major_version << "."
       << _plugin_minor_version << "."
       << _plugin_sequence_version;
  if (!_plugin_official_version) {
    nout << "c";
  }
  nout << "\n";
  nout << "Plugin distributor: " << _plugin_distributor << "\n";
  nout << "Core API host URL: " <<  _coreapi_host_url << "\n";
  nout << "Core API version: " << _coreapi_set_ver << "\n";

  const char *timestamp_string = ctime(&_coreapi_timestamp);
  if (timestamp_string == NULL) {
    timestamp_string = "";
  }
  nout << "Core API date: " << timestamp_string << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::set_super_mirror
//       Access: Public
//  Description: Specifies the "super mirror" URL.  See p3d_plugin.h.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
set_super_mirror(const string &super_mirror_url) {
  reconsider_runtime_environment();
  if (!super_mirror_url.empty()) {
    nout << "super_mirror = " << super_mirror_url << "\n";
  }
  _super_mirror_url = super_mirror_url;

  // Make sure it ends with a slash.
  if (!_super_mirror_url.empty() && _super_mirror_url[_super_mirror_url.size() - 1] != '/') {
    _super_mirror_url += '/';
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::create_instance
//       Access: Public
//  Description: Returns a newly-allocated P3DInstance with the
//               indicated startup information.
////////////////////////////////////////////////////////////////////
P3DInstance *P3DInstanceManager::
create_instance(P3D_request_ready_func *func, 
                const P3D_token tokens[], size_t num_tokens, 
                int argc, const char *argv[], void *user_data) {
  reconsider_runtime_environment();
  P3DInstance *inst = new P3DInstance(func, tokens, num_tokens, argc, argv,
                                      user_data);
  inst->ref();
  _instances.insert(inst);

  return inst;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::set_p3d_filename
//       Access: Public
//  Description: Sets the p3d_filename (or p3d_url) on a particular
//               instance.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
set_p3d_filename(P3DInstance *inst, bool is_local,
                 const string &p3d_filename, const int &p3d_offset) {
  if (inst->is_started()) {
    nout << "Instance started twice: " << inst << "\n";
    return false;
  }
  if (is_local) {
    inst->set_p3d_filename(p3d_filename, p3d_offset);
  } else {
    inst->set_p3d_url(p3d_filename);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::make_p3d_stream
//       Access: Public
//  Description: Indicates an intention to transmit the p3d data as a
//               stream.  Should return a new unique stream ID to
//               receive it.
////////////////////////////////////////////////////////////////////
int P3DInstanceManager::
make_p3d_stream(P3DInstance *inst, const string &p3d_url) {
  if (inst->is_started()) {
    nout << "Instance started twice: " << inst << "\n";
    return -1;
  }
  return inst->make_p3d_stream(p3d_url);
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::start_instance
//       Access: Public
//  Description: Actually starts the instance running on a particular
//               session.  This is called by the P3DInstance when it
//               successfully loads its instance file.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
start_instance(P3DInstance *inst) {
  if (inst->is_failed()) {
    // Don't bother trying again.
    return false;
  }

  if (inst->is_started()) {
    // Already started.
    return true;
  }

  P3DSession *session;
  Sessions::iterator si = _sessions.find(inst->get_session_key());
  if (si == _sessions.end()) {
    session = new P3DSession(inst);
    session->ref();
    bool inserted = _sessions.insert(Sessions::value_type(session->get_session_key(), session)).second;
    assert(inserted);
  } else {
    session = (*si).second;
  }

  session->start_instance(inst);

  return inst->is_started();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::finish_instance
//       Access: Public
//  Description: Terminates and removes a previously-returned
//               instance.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
finish_instance(P3DInstance *inst) {
  nout << "finish_instance: " << inst << "\n";
  Instances::iterator ii;
  ii = _instances.find(inst);
  if (ii != _instances.end()) {
    _instances.erase(ii);
  }

  Sessions::iterator si = _sessions.find(inst->get_session_key());
  if (si != _sessions.end()) {
    P3DSession *session = (*si).second;
    session->terminate_instance(inst);

    // If that was the last instance in this session, terminate the
    // session.
    if (session->get_num_instances() == 0) {
      _sessions.erase(session->get_session_key());
      session->shutdown();
      p3d_unref_delete(session);
    }
  }

  inst->cleanup();
  p3d_unref_delete(inst);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::authorize_instance
//       Access: Public
//  Description: Creates a new P3DAuthSession object, to pop up a
//               window for the user to authorize the certificate on
//               this instance.  Automatically terminates any
//               previously-created P3DAuthSession.
////////////////////////////////////////////////////////////////////
P3DAuthSession *P3DInstanceManager::
authorize_instance(P3DInstance *inst) {
  if (_auth_session != NULL) {
    // We only want one auth_session window open at a time, to
    // minimize user confusion, so close any previous window.
    _auth_session->shutdown(true);
    p3d_unref_delete(_auth_session);
    _auth_session = NULL;
  }

  _auth_session = new P3DAuthSession(inst);
  _auth_session->ref();
  return _auth_session;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::validate_instance
//       Access: Public
//  Description: Returns the P3DInstance pointer corresponding to the
//               indicated P3D_instance if it is valid, or NULL if it
//               is not.
////////////////////////////////////////////////////////////////////
P3DInstance *P3DInstanceManager::
validate_instance(P3D_instance *instance) {
  Instances::iterator ii;
  ii = _instances.find((P3DInstance *)instance);
  if (ii != _instances.end()) {
    return (*ii);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::check_request
//       Access: Public
//  Description: If a request is currently pending on any instance,
//               returns its pointer.  Otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
P3DInstance *P3DInstanceManager::
check_request() {
  Instances::iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    P3DInstance *inst = (*ii);
    if (inst->has_request()) {
      return inst;
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::wait_request
//       Access: Public
//  Description: Does not return until a request is pending on some
//               instance, or until no instances remain, or until the
//               indicated time in seconds has elapsed.  Use
//               check_request to retrieve the pending request.  Due
//               to the possibility of race conditions, it is possible
//               for this function to return when there is in fact no
//               request pending (another thread may have extracted
//               the request first).
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
wait_request(double timeout) {
#ifdef _WIN32
  int stop_tick = int(GetTickCount() + timeout * 1000.0);
#else
  struct timeval stop_time;
  gettimeofday(&stop_time, NULL);

  int seconds = (int)floor(timeout);
  stop_time.tv_sec += seconds;
  stop_time.tv_usec += (int)((timeout - seconds) * 1000.0);
  if (stop_time.tv_usec > 1000) {
    stop_time.tv_usec -= 1000;
    ++stop_time.tv_sec;
  }
#endif

  _request_ready.acquire();
  if (check_request() != (P3DInstance *)NULL) {
    _request_ready.release();
    return;
  }
  if (_instances.empty()) {
    _request_ready.release();
    return;
  }
  
  // No pending requests; go to sleep.
  _request_ready.wait(timeout);

  while (true) {
#ifdef _WIN32
    int remaining_ticks = stop_tick - GetTickCount();
    if (remaining_ticks <= 0) {
      break;
    }
    timeout = remaining_ticks * 0.001;
#else
    struct timeval now;
    gettimeofday(&now, NULL);

    struct timeval remaining;
    remaining.tv_sec = stop_time.tv_sec - now.tv_sec;
    remaining.tv_usec = stop_time.tv_usec - now.tv_usec;

    if (remaining.tv_usec < 0) {
      remaining.tv_usec += 1000;
      --remaining.tv_sec;
    }
    if (remaining.tv_sec < 0) {
      break;
    }
    timeout = remaining.tv_sec + remaining.tv_usec * 0.001;
#endif

    if (check_request() != (P3DInstance *)NULL) {
      _request_ready.release();
      return;
    }
    if (_instances.empty()) {
      _request_ready.release();
      return;
    }
    
    // No pending requests; go to sleep.
    _request_ready.wait(timeout);
  }
  _request_ready.release();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_host
//       Access: Public
//  Description: Returns a (possibly shared) pointer to the indicated
//               download host.
////////////////////////////////////////////////////////////////////
P3DHost *P3DInstanceManager::
get_host(const string &host_url) {
  Hosts::iterator pi = _hosts.find(host_url);
  if (pi != _hosts.end()) {
    return (*pi).second;
  }

  P3DHost *host = new P3DHost(host_url, _host_dir);
  bool inserted = _hosts.insert(Hosts::value_type(host_url, host)).second;
  assert(inserted);

  return host;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::forget_host
//       Access: Public
//  Description: Removes the indicated host from the cache.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
forget_host(P3DHost *host) {
  const string &host_url = host->get_host_url();

  nout << "Forgetting host " << host_url << "\n";
  
  // Hmm, this is a memory leak.  But we allow it to remain, since
  // it's an unusual circumstance (uninstalling), and it's safer to
  // leak than to risk a floating pointer.
  _hosts.erase(host_url);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_unique_id
//       Access: Public
//  Description: Returns a number used to uniquify different
//               instances.  This number is guaranteed to be different
//               at each call, at least until the int space rolls
//               over.
////////////////////////////////////////////////////////////////////
int P3DInstanceManager::
get_unique_id() {
  ++_unique_id;
  return _unique_id;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::signal_request_ready
//       Access: Public
//  Description: May be called in any thread to indicate that a new
//               P3D_request is available in the indicated instance.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
signal_request_ready(P3DInstance *inst) {
  if (inst->get_request_ready_func() != NULL) {
    // This instance requires asynchronous notifications of requests.
    // Thus, we should tell the notify thread to wake up and make the
    // callback.
    _notify_ready.acquire();
    _notify_instances.push_back(inst);
    _notify_ready.notify();
    _notify_ready.release();

    // Oh, and we should spawn the thread if we haven't already.
    if (!_started_notify_thread) {
      _notify_thread_continue = true;
      SPAWN_THREAD(_notify_thread, nt_thread_run, this);
      _started_notify_thread = true;
    }
  }

  // Then, wake up the main thread, in case it's sleeping on
  // wait_request().
  _request_ready.acquire();
  _request_ready.notify();
  _request_ready.release();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::make_class_definition
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3D_class_definition *P3DInstanceManager::
make_class_definition() const {
  P3D_class_definition *new_class = new P3D_class_definition(P3DObject::_generic_class);
  // TODO: save this pointer so we can delete it on destruction.
  return new_class;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::make_temp_filename
//       Access: Public
//  Description: Constructs a new, unique temporary filename with the
//               indicated extension.  You should use the
//               P3DTemporaryFilename interface instead of calling
//               this method directly.
////////////////////////////////////////////////////////////////////
string P3DInstanceManager::
make_temp_filename(const string &extension) {
  string result;
  bool exists;

  do {
    int tid;
#ifdef _WIN32
    tid = GetCurrentProcessId();
#else
    tid = getpid();
#endif
    if (tid == 0) {
      tid = 1;
    }
    int hash = ((clock() + _next_temp_filename_counter) * ((time(NULL) * tid) >> 8)) & 0xffffff;
    ++_next_temp_filename_counter;
    char hex_code[10];
    sprintf(hex_code, "%06x", hash);

    result = _temp_directory;
    result += "p3d_";
    result += hex_code;
    result += extension;
    
    exists = false;
    if (_temp_filenames.find(result) != _temp_filenames.end()) {
      // We've previously allocated this file.
      exists = true;

    } else {

      // Check if the file exists on disk.
#ifdef _WIN32
      DWORD results = GetFileAttributes(result.c_str());
      if (results != -1) {
        exists = true;
      }

#else  // _WIN32
      struct stat this_buf;
      if (stat(result.c_str(), &this_buf) == 0) {
        exists = true;
      }
#endif
    }

  } while (exists);

  nout << "make_temp_filename: " << result << "\n";
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::release_temp_filename
//       Access: Public
//  Description: Releases a temporary filename assigned earlier via
//               make_temp_filename().  If the file exists, it will be
//               removed.  You should use the P3DTemporaryFilename
//               interface instead of calling this method directly.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
release_temp_filename(const string &filename) {
  nout << "release_temp_filename: " << filename << "\n";
  _temp_filenames.erase(filename);
  unlink(filename.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::find_cert
//       Access: Public
//  Description: Looks for the particular certificate in the cache of
//               recognized certificates.  Returns true if it is
//               found, false if not.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
find_cert(X509 *cert) {
  // First, we need the DER representation.
  string der = cert_to_der(cert);

  // If we've previously found this certificate, we don't have to hit
  // disk again.
  ApprovedCerts::iterator ci = _approved_certs.find(der);
  if (ci != _approved_certs.end()) {
    return true;
  }

  // Well, we haven't found it already.  Look for it on disk.  For
  // this, we hash the cert into a hex string.  This is similar to
  // OpenSSL's get_by_subject() approach, except we hash the whole
  // cert, not just the subject.  (Since we also store self-signed
  // certs in this list, we can't trust the subject name alone.)
  string this_cert_dir = get_cert_dir(cert);
  nout << "looking in " << this_cert_dir << "\n";

  vector<string> contents;
  scan_directory(this_cert_dir, contents);

  // Now look at each of the files in this directory and see if any of
  // them matches the certificate.
  vector<string>::iterator si;
  for (si = contents.begin(); si != contents.end(); ++si) {
    string filename = this_cert_dir + "/" + (*si);
    X509 *x509 = NULL;
    FILE *fp = NULL;
#ifdef _WIN32
    wstring filename_w;
    if (string_to_wstring(filename_w, filename)) {
      fp = _wfopen(filename_w.c_str(), L"r");
    }
#else // _WIN32
    fp = fopen(filename.c_str(), "r");
#endif  // _WIN32
    if (fp != NULL) {
      x509 = PEM_read_X509(fp, NULL, NULL, (void *)"");
      fclose(fp);
    }

    if (x509 != NULL) {
      string der2 = cert_to_der(x509);
      // We might as well save this cert in the table for next time,
      // even if it's not the one we're looking for right now.
      _approved_certs.insert(der2);
      
      if (der == der2) {
        return true;
      }
    }
  }

  // Nothing matched.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::read_certlist
//       Access: Public
//  Description: Reads the pre-approved certificates in the certlist
//               package and adds them to the in-memory cache.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
read_certlist(P3DPackage *package) {
  nout << "reading certlist in " << package->get_package_dir() << "\n";

  vector<string> contents;
  scan_directory(package->get_package_dir(), contents);

  vector<string>::iterator si;
  for (si = contents.begin(); si != contents.end(); ++si) {
    const string &basename = (*si);
    if (basename.length() > 4) {
      string suffix = basename.substr(basename.length() - 4);
      if (suffix == ".pem" || suffix == ".crt") {
        string filename = package->get_package_dir() + "/" + basename;
        X509 *x509 = NULL;
        FILE *fp = NULL;
#ifdef _WIN32
        wstring filename_w;
        if (string_to_wstring(filename_w, filename)) {
          fp = _wfopen(filename_w.c_str(), L"r");
        }
#else // _WIN32
        fp = fopen(filename.c_str(), "r");
#endif  // _WIN32
        if (fp != NULL) {
          x509 = PEM_read_X509(fp, NULL, NULL, (void *)"");
          fclose(fp);
        }
        
        if (x509 != NULL) {
          string der2 = cert_to_der(x509);
          _approved_certs.insert(der2);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_cert_dir
//       Access: Public
//  Description: Returns the directory searched for this particular
//               certificate.
////////////////////////////////////////////////////////////////////
string P3DInstanceManager::
get_cert_dir(X509 *cert) {
  string der = cert_to_der(cert);

  static const size_t hash_size = 16;
  unsigned char md[hash_size];

  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, der.data(), der.size());
  MD5_Final(md, &ctx);

  string basename;
  static const size_t keep_hash = 6;
  for (size_t i = 0; i < keep_hash; ++i) {
    int high = (md[i] >> 4) & 0xf;
    int low = md[i] & 0xf;
    basename += P3DInstanceManager::encode_hexdigit(high);
    basename += P3DInstanceManager::encode_hexdigit(low);
  }

  return _certs_dir + "/" + basename;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::cert_to_der
//       Access: Public, Static
//  Description: Converts the indicated certificate to its binary DER
//               representation.
////////////////////////////////////////////////////////////////////
string P3DInstanceManager::
cert_to_der(X509 *cert) {
  int buffer_size = i2d_X509(cert, NULL);
  unsigned char *buffer = new unsigned char[buffer_size];
  unsigned char *p = buffer;
  i2d_X509(cert, &p);
  
  string result((char *)buffer, buffer_size);
  delete[] buffer;

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::uninstall_all
//       Access: Public
//  Description: Stops all active instances and removes *all*
//               downloaded files from all hosts, and empties the
//               current user's Panda3D directory as much as possible.
//
//               This cannot remove the coreapi dll or directory on
//               Windows.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
uninstall_all() {
  Instances::iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    P3DInstance *inst = (*ii);
    inst->uninstall_host();
  }

  Hosts::iterator hi;
  for (hi = _hosts.begin(); hi != _hosts.end(); ++hi) {
    P3DHost *host = (*hi).second;
    host->uninstall();
  }

  // Close the logfile so we can remove that too.
  logfile.close();

  if (!_root_dir.empty()) {
    // This won't be able to delete the coreapi directory on Windows,
    // because we're running that DLL right now.  But it will delete
    // everything else.
    delete_directory_recursively(_root_dir);
  }

  _created_runtime_environment = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_global_ptr
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstanceManager *P3DInstanceManager::
get_global_ptr() {
  if (_global_ptr == NULL) {
    _global_ptr = new P3DInstanceManager;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::delete_global_ptr
//       Access: Public, Static
//  Description: This is called only at plugin shutdown time; it
//               deletes the global instance manager pointer and
//               clears it to NULL.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
delete_global_ptr() {
  if (_global_ptr != NULL) {
    delete _global_ptr;
    _global_ptr = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::scan_directory
//       Access: Public, Static
//  Description: Attempts to open the named filename as if it were a
//               directory and looks for the non-hidden files within
//               the directory.  Fills the given vector up with the
//               sorted list of filenames that are local to this
//               directory.
//
//               It is the user's responsibility to ensure that the
//               contents vector is empty before making this call;
//               otherwise, the new files will be appended to it.
//
//               Returns true on success, false if the directory could
//               not be read for some reason.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
scan_directory(const string &dirname, vector<string> &contents) {
#ifdef _WIN32
  // Use Windows' FindFirstFile() / FindNextFile() to walk through the
  // list of files in a directory.
  size_t orig_size = contents.size();

  string match = dirname + "\\*.*";
  WIN32_FIND_DATAW find_data;

  wstring match_w;
  string_to_wstring(match_w, match);

  HANDLE handle = FindFirstFileW(match_w.c_str(), &find_data);
  if (handle == INVALID_HANDLE_VALUE) {
    if (GetLastError() == ERROR_NO_MORE_FILES) {
      // No matching files is not an error.
      return true;
    }
    return false;
  }

  do {
    string filename;
    wstring_to_string(filename, find_data.cFileName);
    if (filename != "." && filename != "..") {
      contents.push_back(filename);
    }
  } while (FindNextFileW(handle, &find_data));

  bool scan_ok = (GetLastError() == ERROR_NO_MORE_FILES);
  FindClose(handle);

  sort(contents.begin() + orig_size, contents.end());
  return scan_ok;

#else  // _WIN32
  // Use Posix's opendir() / readdir() to walk through the list of
  // files in a directory.
  size_t orig_size = contents.size();

  DIR *root = opendir(dirname.c_str());
  if (root == (DIR *)NULL) {
    return false;
  }

  struct dirent *d;
  d = readdir(root);
  while (d != (struct dirent *)NULL) {
    if (d->d_name[0] != '.') {
      contents.push_back(d->d_name);
    }
    d = readdir(root);
  }

  closedir(root);

  sort(contents.begin() + orig_size, contents.end());
  return true;

#endif  // _WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::scan_directory_recursively
//       Access: Public, Static
//  Description: Fills up filename_contents with the list of all
//               files (but not directories), and dirname_contents
//               with the list of all directories, rooted at the
//               indicated dirname and below.  The filenames generated
//               are relative to the root of the dirname, with slashes
//               (not backslashes) as the directory separator
//               character.
//
//               Returns true on success, false if the original
//               dirname wasn't a directory or something like that.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
scan_directory_recursively(const string &dirname, 
                           vector<string> &filename_contents,
                           vector<string> &dirname_contents,
                           const string &prefix) {
  vector<string> dir_contents;
  if (!scan_directory(dirname, dir_contents)) {
    // Apparently dirname wasn't a directory.
    return false;
  }

  // Walk through the contents of dirname.
  vector<string>::const_iterator si;
  for (si = dir_contents.begin(); si != dir_contents.end(); ++si) {
    // Here's a particular file within dirname.  Is it another
    // directory, or is it a regular file?
    string pathname = dirname + "/" + (*si);
    string rel_filename = prefix + (*si);
    if (scan_directory_recursively(pathname, filename_contents, 
                                   dirname_contents, rel_filename + "/")) {
      // It's a directory, and it's just added its results to the
      // contents.
      dirname_contents.push_back(rel_filename);

    } else {
      // It's not a directory, so assume it's an ordinary file, and
      // add it to the contents.
      filename_contents.push_back(rel_filename);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::delete_directory_recursively
//       Access: Public, Static
//  Description: Deletes all of the files and directories in the named
//               directory and below, like rm -rf.  Use with extreme
//               caution.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
delete_directory_recursively(const string &root_dir) {
  vector<string> contents, dirname_contents;
  if (!scan_directory_recursively(root_dir, contents, dirname_contents)) {
    // Maybe it's just a single file, not a directory.  Delete it.
#ifdef _WIN32
    wstring root_dir_w;
    string_to_wstring(root_dir_w, root_dir);
    // Windows can't delete a file if it's read-only.
    _wchmod(root_dir_w.c_str(), 0644);
    int result = _wunlink(root_dir_w.c_str());
#else  // _WIN32
    int result = unlink(root_dir.c_str());
#endif  // _WIN32
    if (result == 0) {
      nout << "Deleted " << root_dir << "\n";
    } else {
#ifdef _WIN32
      result = _waccess(root_dir_w.c_str(), 0);
#else  // _WIN32
      result = access(root_dir.c_str(), 0);
#endif  // _WIN32
      if (result == 0) {
        nout << "Could not delete " << root_dir << "\n";
      }
    }
    return;
  }

  vector<string>::iterator ci;
  for (ci = contents.begin(); ci != contents.end(); ++ci) {
    string filename = (*ci);
    string pathname = root_dir + "/" + filename;

#ifdef _WIN32
    wstring pathname_w;
    string_to_wstring(pathname_w, pathname);
    // Windows can't delete a file if it's read-only.
    _wchmod(pathname_w.c_str(), 0644);
    int result = _wunlink(pathname_w.c_str());
#else  // _WIN32
    int result = unlink(pathname.c_str());
#endif  // _WIN32
    if (result == 0) {
      nout << "  Deleted " << filename << "\n";
    } else {
      nout << "  Could not delete " << filename << "\n";
    }
  }

  // Now delete all of the directories too.  They're already in
  // reverse order, so we remove deeper directories first.
  for (ci = dirname_contents.begin(); ci != dirname_contents.end(); ++ci) {
    string filename = (*ci);
    string pathname = root_dir + "/" + filename;

#ifdef _WIN32
    wstring pathname_w;
    string_to_wstring(pathname_w, pathname);
    _wchmod(pathname_w.c_str(), 0755);
    int result = _wrmdir(pathname_w.c_str());
#else  // _WIN32
    int result = rmdir(pathname.c_str());
#endif  // _WIN32
    if (result == 0) {
      nout << "  Removed directory " << filename << "\n";
    } else {
      nout << "  Could not remove directory " << filename << "\n";
    }
  }

  // Finally, delete the root directory itself.
  string pathname = root_dir;
#ifdef _WIN32
  wstring pathname_w;
  string_to_wstring(pathname_w, pathname);
  _wchmod(pathname_w.c_str(), 0755);
  int result = _wrmdir(pathname_w.c_str());
#else  // _WIN32
  int result = rmdir(pathname.c_str());
#endif  // _WIN32
  if (result == 0) {
    nout << "Removed directory " << root_dir << "\n";
  } else {
#ifdef _WIN32
    result = _waccess(pathname_w.c_str(), 0);
#else  // _WIN32
    result = access(pathname.c_str(), 0);
#endif  // _WIN32
    if (result == 0) {
      nout << "Could not remove directory " << root_dir << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::remove_file_from_list
//       Access: Public, Static
//  Description: Removes the first instance of the indicated file
//               from the given list.  Returns true if removed, false
//               if it was not found.
//
//               On Windows, the directory separator characters are
//               changed from backslash to forward slash before
//               searching in the list; so it is assumed that the list
//               contains filenames with a forward slash used as a
//               separator.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
remove_file_from_list(vector<string> &contents, const string &filename) {
#ifdef _WIN32
  // Convert backslashes to slashes.
  string clean_filename;
  for (string::const_iterator pi = filename.begin(); 
       pi != filename.end(); 
       ++pi) {
    if ((*pi) == '\\') {
      clean_filename += '/';
    } else {
      clean_filename += (*pi);
    }
  }
#else
  const string &clean_filename = filename;
#endif  // _WIN32

  vector<string>::iterator ci;
  for (ci = contents.begin(); ci != contents.end(); ++ci) {
    if ((*ci) == clean_filename) {
      contents.erase(ci);
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::append_safe_dir
//       Access: Public, Static
//  Description: Appends the indicated basename to the root directory
//               name, which is modified in-place.  The basename is
//               allowed to contain nested slashes, but no directory
//               component of the basename may begin with a ".", thus
//               precluding ".." and hidden files.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
append_safe_dir(string &root, const string &basename) {
  if (basename.empty()) {
    return;
  }

  size_t p = 0;
  while (p < basename.length()) {
    size_t q = basename.find('/', p);
    if (q == string::npos) {
      if (q != p) {
        append_safe_dir_component(root, basename.substr(p));
      }
      return;
    }
    if (q != p) {
      append_safe_dir_component(root, basename.substr(p, q - p));
    }
    p = q + 1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::create_runtime_environment
//       Access: Private
//  Description: Called during initialize, or after a previous call to
//               uninstall_all(), to make sure all needed
//               directories exist and the logfile is open.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
create_runtime_environment() {
  mkdir_complete(_log_directory, cerr);

  logfile.close();
  logfile.clear();
#ifdef _WIN32
  wstring log_pathname_w;
  string_to_wstring(log_pathname_w, _log_pathname);
  logfile.open(log_pathname_w.c_str(), ios::out | ios::trunc);
#else
  logfile.open(_log_pathname.c_str(), ios::out | ios::trunc);
#endif  // _WIN32
  if (logfile) {
    logfile.setf(ios::unitbuf);
    nout_stream = &logfile;
  }

  // Determine the temporary directory.
#ifdef _WIN32
  wchar_t buffer_1[MAX_PATH];
  wstring temp_directory_w;

  // Figuring out the correct path for temporary files is a real mess
  // on Windows.  We should be able to use GetTempPath(), but that
  // relies on $TMP or $TEMP being defined, and it appears that
  // Mozilla clears these environment variables for the plugin, which
  // forces GetTempPath() into $USERPROFILE instead.  This is really
  // an inappropriate place for temporary files, so, GetTempPath()
  // isn't a great choice.

  // We could use SHGetSpecialFolderPath() instead to get us the path
  // to "Temporary Internet Files", which is acceptable.  The trouble
  // is, if we happen to be running in "Protected Mode" on Vista, this
  // folder isn't actually writable by us!  On Vista, we're supposed
  // to use IEGetWriteableFolderPath() instead, but *this* function
  // doesn't exist on XP and below.  Good Lord.

  // We could go through a bunch of LoadLibrary() calls to try to find
  // the right path, like we do in find_root_dir(), but I'm just tired
  // of doing all that nonsense.  We'll use a two-stage trick instead.
  // We'll check for $TEMP or $TMP being defined specifically, and if
  // they are, we'll use GetTempPath(); otherwise, we'll fall back to
  // SHGetSpecialFolderPath().

  if (getenv("TEMP") != NULL || getenv("TMP") != NULL) {
    if (GetTempPathW(MAX_PATH, buffer_1) != 0) {
      temp_directory_w = buffer_1;
    }
  }
  if (temp_directory_w.empty()) {
    if (SHGetSpecialFolderPathW(NULL, buffer_1, CSIDL_INTERNET_CACHE, true)) {
      temp_directory_w = buffer_1;

      // That just *might* return a non-writable folder, if we're in
      // Protected Mode.  We'll test this with GetTempFileName().
      wchar_t temp_buffer[MAX_PATH];
      if (!GetTempFileNameW(temp_directory_w.c_str(), L"p3d", 0, temp_buffer)) {
        nout << "GetTempFileName failed on " << temp_directory_w
             << ", switching to GetTempPath\n";
        temp_directory_w.clear();
      } else {
        DeleteFileW(temp_buffer);
      }
    }
  }

  // If both of the above failed, we'll fall back to GetTempPath()
  // once again as a last resort, which is supposed to return
  // *something* that works, even if $TEMP and $TMP are undefined.
  if (temp_directory_w.empty()) {
    if (GetTempPathW(MAX_PATH, buffer_1) != 0) {
      temp_directory_w = buffer_1;
    }
  }

  // Also insist that the temp directory is fully specified.
  size_t needs_size_2 = GetFullPathNameW(temp_directory_w.c_str(), 0, NULL, NULL);
  wchar_t *buffer_2 = new wchar_t[needs_size_2];
  if (GetFullPathNameW(temp_directory_w.c_str(), needs_size_2, buffer_2, NULL) != 0) {
    temp_directory_w = buffer_2;
  }
  delete[] buffer_2;

  // And make sure the directory actually exists.
  mkdir_complete_w(temp_directory_w, nout);
  wstring_to_string(_temp_directory, temp_directory_w);

#else
  _temp_directory = "/tmp/";
#endif  // _WIN32

  // Ensure that the temp directory ends with a slash.
  if (!_temp_directory.empty() && _temp_directory[_temp_directory.size() - 1] != '/') {
#ifdef _WIN32
    if (_temp_directory[_temp_directory.size() - 1] != '\\')
#endif
      _temp_directory += "/";
  }

  nout << "\n_root_dir = " << _root_dir
       << ", _temp_directory = " << _temp_directory
       << ", platform = " << _platform
       << ", host_url = " << _host_url
       << ", verify_contents = " << _verify_contents
       << "\n";
  if (!_host_dir.empty()) {
    nout << "_host_dir = " << _host_dir << "\n";
  }
  nout << "api_version = " << _api_version << "\n";

  // Make the certificate directory.
  _certs_dir = _root_dir + "/certs";
  if (!mkdir_complete(_certs_dir, nout)) {
    nout << "Couldn't mkdir " << _certs_dir << "\n";
  }

  _created_runtime_environment = true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::append_safe_dir_component
//       Access: Private, Static
//  Description: Appends a single directory component, implementing
//               append_safe_dir(), above.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
append_safe_dir_component(string &root, const string &component) {
  if (component.empty()) {
    return;
  }
  root += '/';
  if (component[0] == '.') {
    root += 'x';
  }
  root += component;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::nt_thread_run
//       Access: Private
//  Description: The main function for the notify thread.
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
nt_thread_run() {
  // The notify thread exists because we need to be able to send
  // asynchronous notifications of request events.  These request
  // events were detected in the various read threads associated with
  // each session, but we can't call back into the plugin host space
  // from the read thread, since if the host immediately responds to a
  // callback by calling back into the p3d_plugin space, we will have
  // our read thread doing stuff in here that's not related to the
  // read thread.  Even worse, some of the things it might need to do
  // might require a separate read thread to be running!

  _notify_ready.acquire();
  while (_notify_thread_continue) {
    NotifyInstances instances;
    while (!_notify_instances.empty()) {
      instances.clear();
      instances.swap(_notify_instances);

      // Go ahead and drop the lock while we make the callback, to
      // reduce the risk of deadlock.  We don't want to be holding any
      // locks when we call into client code.
      _notify_ready.release();
      NotifyInstances::iterator ni;
      for (ni = instances.begin(); ni != instances.end(); ++ni) {
        // TODO: a race condition here when instances are deleted.
        P3DInstance *inst = (*ni);
        assert(inst != NULL);
        P3D_request_ready_func *func = inst->get_request_ready_func();
        if (func != NULL) {
          (*func)(inst);
        }
      }
      _notify_ready.acquire();
    }

    _notify_ready.wait();
  }
  _notify_ready.release();
}

#ifdef _WIN32
bool P3DInstanceManager::
supports_win64() {
  BOOL is_win64 = false;

  typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
  LPFN_ISWOW64PROCESS _IsWow64Process;
  _IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");
  
  if (_IsWow64Process != NULL) {
    if (!_IsWow64Process(GetCurrentProcess(), &is_win64)) {
      is_win64 = false;
    }
  }
  return (is_win64 != 0);
}
#endif  // _WIN32
