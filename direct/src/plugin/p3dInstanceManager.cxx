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
#include "p3dPackage.h"
#include "p3d_plugin_config.h"
#include "p3dWinSplashWindow.h"
#include "p3dUndefinedObject.h"
#include "p3dNoneObject.h"
#include "p3dBoolObject.h"
#include "find_root_dir.h"
#include "fileSpec.h"
#include "get_tinyxml.h"
#include "mkdir_complete.h"

// We can include this header file to get the DTOOL_PLATFORM
// definition, even though we don't link with dtool.
#include "dtool_platform.h"

#ifdef _WIN32
#include <shlobj.h>
#else
#include <sys/stat.h>
#endif

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
  _is_initialized = false;
  _next_temp_filename_counter = 0;
  _unique_id = 0;
  _xcontents = NULL;

  _notify_thread_continue = false;
  _started_notify_thread = false;
  INIT_THREAD(_notify_thread);

  // Initialize the singleton objects.
  _undefined_object = new P3DUndefinedObject();
  _none_object = new P3DNoneObject();
  _true_object = new P3DBoolObject(true);
  _false_object = new P3DBoolObject(false);

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

  if (_xcontents != NULL) {
    delete _xcontents;
  }

  assert(_instances.empty());
  assert(_sessions.empty());

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
//  Description: Called by the host at application startup.  It
//               returns true if the DLL is successfully initialized,
//               false if it should be immediately shut down and
//               redownloaded.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
initialize(const string &contents_filename, const string &download_url,
           const string &platform, const string &log_directory,
           const string &log_basename) {

  _root_dir = find_root_dir();
  _download_url = download_url;
#ifdef P3D_PLUGIN_DOWNLOAD
  if (_download_url.empty()) {
    _download_url = P3D_PLUGIN_DOWNLOAD;
  }
#endif
  _platform = platform;
  if (_platform.empty()) {
    _platform = DTOOL_PLATFORM;
  }

  _log_directory = log_directory;
#ifdef P3D_PLUGIN_LOG_DIRECTORY
  if (_log_directory.empty()) {
    _log_directory = P3D_PLUGIN_LOG_DIRECTORY;
  }
#endif

  // Determine the temporary directory.
#ifdef _WIN32
  size_t needs_size_1 = GetTempPath(0, NULL);
  char *buffer_1 = new char[needs_size_1];
  if (GetTempPath(needs_size_1, buffer_1) != 0) {
    _temp_directory = buffer_1;
  }
  delete[] buffer_1;

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];
  if (GetTempPath(buffer_size, buffer) != 0) {
    _temp_directory = buffer;
  }
  
  // Also insist that the temp directory is fully specified.
  size_t needs_size_2 = GetFullPathName(_temp_directory.c_str(), 0, NULL, NULL);
  char *buffer_2 = new char[needs_size_2];
  if (GetFullPathName(_temp_directory.c_str(), needs_size_2, buffer_2, NULL) != 0) {
    _temp_directory = buffer_2;
  }
  delete[] buffer_2;

  // Also make sure the directory actually exists.
  mkdir_complete(_temp_directory, nout);

#else
  _temp_directory = "/tmp/";
#endif  // _WIN32

  // If the log directory is still empty, use the temp directory.
  if (_log_directory.empty()) {
    _log_directory = _temp_directory;
  }

  _log_basename = log_basename;
#ifdef P3D_PLUGIN_LOG_BASENAME2
  if (_log_basename.empty()) {
    _log_basename = P3D_PLUGIN_LOG_BASENAME2;
  }
#endif

  // Ensure that the download URL ends with a slash.
  if (!_download_url.empty() && _download_url[_download_url.size() - 1] != '/') {
    _download_url += "/";
  }

  // Ensure that the temp directory ends with a slash.
  if (!_temp_directory.empty() && _temp_directory[_temp_directory.size() - 1] != '/') {
#ifdef _WIN32
    if (_temp_directory[_temp_directory.size() - 1] != '\\')
#endif
      _temp_directory += "/";
  }

  // Ensure that the log directory ends with a slash.
  if (!_log_directory.empty() && _log_directory[_log_directory.size() - 1] != '/') {
#ifdef _WIN32
    if (_log_directory[_log_directory.size() - 1] != '\\')
#endif
      _log_directory += "/";
  }

  // Construct the logfile pathname.
  if (!_log_basename.empty()) {
    _log_pathname = _log_directory;
    _log_pathname += _log_basename;
    _log_pathname += ".log";

    logfile.open(_log_pathname.c_str(), ios::out | ios::trunc);
    if (logfile) {
      logfile.setf(ios::unitbuf);
      nout_stream = &logfile;
    }
  }

  nout << "_root_dir = " << _root_dir
       << ", contents = " << contents_filename
       << ", download = " << _download_url
       << ", platform = " << _platform
       << "\n";

  if (_root_dir.empty()) {
    nout << "Could not find root directory.\n";
    return false;
  }

  _is_initialized = true;

  // Attempt to read the supplied contents.xml file.
  if (!contents_filename.empty()) {
    if (!read_contents_file(contents_filename)) {
      nout << "Couldn't read " << contents_filename << "\n";
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::read_contents_file
//       Access: Public
//  Description: Reads the contents.xml file in the indicated
//               filename.  On success, copies the contents.xml file
//               into the standard location (if it's not there
//               already).
//
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
read_contents_file(const string &contents_filename) {
  TiXmlDocument doc(contents_filename.c_str());
  if (!doc.LoadFile()) {
    return false;
  }

  TiXmlElement *xcontents = doc.FirstChildElement("contents");
  if (xcontents == NULL) {
    return false;
  }

  if (_xcontents != NULL) {
    delete _xcontents;
  }
  _xcontents = (TiXmlElement *)xcontents->Clone();

  string standard_filename = _root_dir + "/contents.xml";
  if (standardize_filename(standard_filename) != 
      standardize_filename(contents_filename)) {
    copy_file(contents_filename, standard_filename);
  }

  return true;
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
                 const string &p3d_filename) {
  if (inst->is_started()) {
    nout << "Instance started twice: " << inst << "\n";
    return false;
  }
  if (is_local) {
    inst->set_p3d_filename(p3d_filename);
  } else {
    inst->set_p3d_url(p3d_filename);
  }

  return true;
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
  Instances::iterator ii;
  ii = _instances.find(inst);
  assert(ii != _instances.end());
  _instances.erase(ii);

  Sessions::iterator si = _sessions.find(inst->get_session_key());
  if (si != _sessions.end()) {
    P3DSession *session = (*si).second;
    session->terminate_instance(inst);

    // If that was the last instance in this session, terminate the
    // session.
    if (session->get_num_instances() == 0) {
      _sessions.erase(session->get_session_key());
      session->shutdown();
      unref_delete(session);
    }
  }

  unref_delete(inst);
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
//               instance, or until no instances remain.  Use
//               check_request to retrieve the pending request.  Due
//               to the possibility of race conditions, it is possible
//               for this function to return when there is in fact no
//               request pending (another thread may have extracted
//               the request first).
////////////////////////////////////////////////////////////////////
void P3DInstanceManager::
wait_request() {
  _request_ready.acquire();
  while (true) {
    if (check_request() != (P3DInstance *)NULL) {
      _request_ready.release();
      return;
    }
    if (_instances.empty()) {
      _request_ready.release();
      return;
    }
    
    // No pending requests; go to sleep.
    _request_ready.wait();
  }
  _request_ready.release();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_package
//       Access: Public
//  Description: Returns a (possibly shared) pointer to the indicated
//               package.
////////////////////////////////////////////////////////////////////
P3DPackage *P3DInstanceManager::
get_package(const string &package_name, const string &package_version) {
  string package_platform = get_platform();
  string key = package_name + "_" + package_platform + "_" + package_version;
  Packages::iterator pi = _packages.find(key);
  if (pi != _packages.end()) {
    return (*pi).second;
  }

  P3DPackage *package = 
    new P3DPackage(package_name, package_platform, package_version);
  bool inserted = _packages.insert(Packages::value_type(key, package)).second;
  assert(inserted);

  return package;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_package_desc_file
//       Access: Public
//  Description: Fills the indicated FileSpec with the hash
//               information for the package's desc file.  Returns
//               true if successful, false if the package is unknown.
//               This requires has_contents_file() to return true in
//               order to be successful.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
get_package_desc_file(FileSpec &desc_file,
                      const string &package_name,
                      const string &package_version) {
  if (_xcontents == NULL) {
    return false;
  }

  string package_platform = get_platform();

  // Scan the contents data for the indicated package.
  TiXmlElement *xpackage = _xcontents->FirstChildElement("package");
  while (xpackage != NULL) {
    const char *name = xpackage->Attribute("name");
    const char *platform = xpackage->Attribute("platform");
    const char *version = xpackage->Attribute("version");
    if (name != NULL && platform != NULL && version != NULL &&
        package_name == name && 
        package_platform == platform &&
        package_version == version) {
      // Here's the matching package definition.
      desc_file.load_xml(xpackage);
      return true;
    }

    xpackage = xpackage->NextSiblingElement("package");
  }

  // Couldn't find the named package.
  return false;
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
//     Function: P3DInstanceManager::standardize_filename
//       Access: Private, Static
//  Description: Attempts to change the filename into some standard
//               form for comparison with other filenames.  On a
//               case-insensitive filesystem, this converts the
//               filename to lowercase.  On Windows, it further
//               replaces forward slashes with backslashes.
////////////////////////////////////////////////////////////////////
string P3DInstanceManager::
standardize_filename(const string &filename) {
#if defined(_WIN32) || defined(__APPLE__)
  string new_filename;
  for (string::const_iterator si = filename.begin();
       si != filename.end();
       ++si) {
    char ch = *si;
#ifdef _WIN32
    if (ch == '/') {
      ch = '\\';
    }
#endif  // _WIN32
    new_filename += tolower(ch);
  }
  return new_filename;
#else  // _WIN32 || __APPLE__
  return filename;
#endif  // _WIN32 || __APPLE__
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::copy_file
//       Access: Private, Static
//  Description: Copies the data in the file named by from_filename
//               into the file named by to_filename.
////////////////////////////////////////////////////////////////////
bool P3DInstanceManager::
copy_file(const string &from_filename, const string &to_filename) {
  ifstream in(from_filename.c_str(), ios::in | ios::binary);

  // Copy to a temporary file first, in case (a) the filenames
  // actually refer to the same file, or (b) in case we have different
  // processes writing to the same file, and (c) to prevent
  // partially overwriting the file should something go wrong.
  ostringstream strm;
  strm << to_filename << ".t";
#ifdef _WIN32
  strm << GetCurrentProcessId() << "_" << GetCurrentThreadId();
#else
  strm << getpid();
#endif
  string temp_filename = strm.str();
  ofstream out(temp_filename.c_str(), ios::out | ios::binary);
        
  static const size_t buffer_size = 4096;
  char buffer[buffer_size];
  
  in.read(buffer, buffer_size);
  size_t count = in.gcount();
  while (count != 0) {
    out.write(buffer, count);
    if (out.fail()) {
      unlink(temp_filename.c_str());
      return false;
    }
    in.read(buffer, buffer_size);
    count = in.gcount();
  }
  out.close();

  if (!in.eof()) {
    unlink(temp_filename.c_str());
    return false;
  }

  if (rename(temp_filename.c_str(), to_filename.c_str()) == 0) {
    return true;
  }

  unlink(temp_filename.c_str());
  return false;
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
  // each session, but we can't call back into the host space from the
  // read thread, since if the host immediately response to a callback
  // by calling back into the p3d_plugin space, now we have our read
  // thread doing stuff in here that's not related to the read thread.
  // Even worse, some of the things it might need to do might require
  // a separate read thread to be running!

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
        assert(func != NULL);
        (*func)(inst);
      }
      _notify_ready.acquire();
    }

    _notify_ready.wait();
  }
  _notify_ready.release();
}
