// Filename: P3DAuthSession.cxx
// Created by:  drose (17Sep09)
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

#include "p3dAuthSession.h"
#include "p3dInstance.h"
#include "p3dInstanceManager.h"
#include "p3dMultifileReader.h"
#include "p3d_plugin_config.h"
#include "mkdir_complete.h"
#include "wstring_encode.h"

#include <ctype.h>

#ifndef _WIN32
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <dlfcn.h>
#include <unistd.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthSession::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DAuthSession::
P3DAuthSession(P3DInstance *inst) :
  _inst(inst)
{
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

#ifdef _WIN32
  _p3dcert_handle = INVALID_HANDLE_VALUE;
#else
  _p3dcert_pid = -1;
#endif
  _p3dcert_started = false;
  _p3dcert_running = false;
  _started_wait_thread = false;
  INIT_THREAD(_wait_thread);

  // Write the cert chain to a temporary file.
  _cert_filename = new P3DTemporaryFile(".crt");
  string filename = _cert_filename->get_filename();
  FILE *fp = fopen(filename.c_str(), "w");
  if (fp == NULL) {
    nout << "Couldn't write temporary file\n";
    return;
  }

  if (inst->_mf_reader.get_num_signatures() > 0) {
    const P3DMultifileReader::CertChain &cert_chain = 
      inst->_mf_reader.get_signature(0);

    if (cert_chain.size() > 0) {
      // Save the cert_dir, this is where the p3dcert program will
      // need to write the cert when it is approved.
      P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
      _cert_dir = inst_mgr->get_cert_dir(cert_chain[0]._cert);
    }

    P3DMultifileReader::CertChain::const_iterator ci;
    for (ci = cert_chain.begin(); ci != cert_chain.end(); ++ci) {
      X509 *c = (*ci)._cert;
      PEM_write_X509(fp, c);
    }
  }
  fclose(fp);

  start_p3dcert();
}


////////////////////////////////////////////////////////////////////
//     Function: P3DAuthSession::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DAuthSession::
~P3DAuthSession() {
  shutdown(false);

  if (_cert_filename != NULL) {
    delete _cert_filename;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthSession::shutdown
//       Access: Public
//  Description: Terminates the session by killing the subprocess.
////////////////////////////////////////////////////////////////////
void P3DAuthSession::
shutdown(bool send_message) {
  if (!send_message) {
    // If we're not to send the instance the shutdown message as a
    // result of this, then clear the _inst pointer now.
    _inst = NULL;
  }

  if (_p3dcert_running) {
    nout << "Killing p3dcert process\n";
#ifdef _WIN32
    TerminateProcess(_p3dcert_handle, 2);
    CloseHandle(_p3dcert_handle);

#else  // _WIN32
    kill(_p3dcert_pid, SIGKILL);

    // Wait a few milliseconds for the process to exit, and then get
    // its return status to clean up the zombie status.  If we don't
    // wait long enough, don't sweat it.
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    select(0, NULL, NULL, NULL, &tv);
    int status;
    waitpid(_p3dcert_pid, &status, WNOHANG);
#endif  // _WIN32
    
    _p3dcert_running = false;
  }
  _p3dcert_started = false;

  // Now that the process has stopped, the thread should stop itself
  // quickly too.
  join_wait_thread();

  // We're no longer bound to any particular instance.
  _inst = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthSession::start_p3dcert
//       Access: Private
//  Description: Starts the p3dcert program running in a child
//               process.
////////////////////////////////////////////////////////////////////
void P3DAuthSession::
start_p3dcert() {
  if (_p3dcert_started) {
    // Already started.
    return;
  }

  if (_inst->_p3dcert_package == NULL) {
    nout << "Couldn't start Python: no p3dcert package.\n";
    return;
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  _start_dir = inst_mgr->get_root_dir() + "/start";

  string root_dir = _inst->_p3dcert_package->get_package_dir();
  mkdir_complete(_start_dir, nout);

  _p3dcert_exe = root_dir + "/p3dcert";
#ifdef _WIN32
  _p3dcert_exe += ".exe";
#endif
#ifdef __APPLE__
  // On OSX, run from the packaged bundle.
  _p3dcert_exe = root_dir + "/P3DCert.app/Contents/MacOS/p3dcert";
#endif

  // Populate the new process' environment.

#ifdef _WIN32
  // These are the enviroment variables we forward from the current
  // environment, if they are set.
  const wchar_t *keep[] = {
    L"TMP", L"TEMP", L"HOME", L"USER", 
    L"SYSTEMROOT", L"USERPROFILE", L"COMSPEC",
    NULL
  };

  wstring env_w;

  for (int ki = 0; keep[ki] != NULL; ++ki) {
    wchar_t *value = _wgetenv(keep[ki]);
    if (value != NULL) {
      env_w += keep[ki];
      env_w += L"=";
      env_w += value;
      env_w += (wchar_t)'\0';
    }
  }

  wstring_to_string(_env, env_w);

#else  // _WIN32

  _env = string();

  // These are the enviroment variables we forward from the current
  // environment, if they are set.
  const char *keep[] = {
    "TMP", "TEMP", "HOME", "USER", 
#ifdef HAVE_X11
    "DISPLAY", "XAUTHORITY",
#endif
    NULL
  };
  for (int ki = 0; keep[ki] != NULL; ++ki) {
    char *value = getenv(keep[ki]);
    if (value != NULL) {
      _env += keep[ki];
      _env += "=";
      _env += value;
      _env += '\0';
    }
  }
#endif  // _WIN32

  // Define some new environment variables.
  _env += "PATH=";
  _env += root_dir;
  _env += '\0';

  _env += "LD_LIBRARY_PATH=";
  _env += root_dir;
  _env += '\0';

  _env += "DYLD_LIBRARY_PATH=";
  _env += root_dir;
  _env += '\0';

  _env += "P3DCERT_ROOT=";
  _env += root_dir;
  _env += '\0';

  nout << "Setting environment:\n";
  write_env();

  nout << "Attempting to start p3dcert from " << _p3dcert_exe << "\n";

  bool started_p3dcert = false;
#ifdef _WIN32
  _p3dcert_handle = win_create_process();
  started_p3dcert = (_p3dcert_handle != INVALID_HANDLE_VALUE);
#else
  _p3dcert_pid = posix_create_process();
  started_p3dcert = (_p3dcert_pid > 0);
#endif
  if (!started_p3dcert) {
    nout << "Failed to create process.\n";
    return;
  }

  _p3dcert_started = true;
  _p3dcert_running = true;
  
  spawn_wait_thread();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthSession::spawn_wait_thread
//       Access: Private
//  Description: Starts the wait thread.  This thread is responsible
//               for waiting for the process to finish, and notifying
//               the instance when it does.
////////////////////////////////////////////////////////////////////
void P3DAuthSession::
spawn_wait_thread() {
  SPAWN_THREAD(_wait_thread, wt_thread_run, this);
  _started_wait_thread = true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthSession::join_wait_thread
//       Access: Private
//  Description: Waits for the wait thread to stop.
////////////////////////////////////////////////////////////////////
void P3DAuthSession::
join_wait_thread() {
  if (!_started_wait_thread) {
    return;
  }

  JOIN_THREAD(_wait_thread);
  _started_wait_thread = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthSession::write_env
//       Access: Private
//  Description: Writes _env, which is formatted as a string
//               containing zero-byte-terminated environment
//               defintions, to the nout stream, one definition per
//               line.
////////////////////////////////////////////////////////////////////
void P3DAuthSession::
write_env() const {
  size_t p = 0;
  size_t zero = _env.find('\0', p);
  while (zero != string::npos) {
    nout << "  ";
    nout.write(_env.data() + p, zero - p);
    nout << "\n";
    p = zero + 1;
    zero = _env.find('\0', p);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthSession::wt_thread_run
//       Access: Private
//  Description: The main function for the wait thread.
////////////////////////////////////////////////////////////////////
void P3DAuthSession::
wt_thread_run() {
  // All we do here is wait for the process to terminate.
  nout << "wt_thread_run in " << this << " beginning\n";
#ifdef _WIN32
  DWORD result = WaitForSingleObject(_p3dcert_handle, INFINITE);
  if (result != 0) {
    nout << "Wait for process failed: " << GetLastError() << "\n";
  }
  CloseHandle(_p3dcert_handle);
  _p3dcert_handle = INVALID_HANDLE_VALUE;
  _p3dcert_running = false;
  nout << "p3dcert process has successfully stopped.\n";
#else
  int status;
  pid_t result = waitpid(_p3dcert_pid, &status, 0);
  if (result == -1) {
    perror("waitpid");
  }
  _p3dcert_pid = -1;
  _p3dcert_running = false;
      
  nout << "p3dcert process has successfully stopped.\n";
  if (WIFEXITED(status)) {
    nout << "  exited normally, status = "
         << WEXITSTATUS(status) << "\n";
  } else if (WIFSIGNALED(status)) {
    nout << "  signalled by " << WTERMSIG(status) << ", core = " 
         << WCOREDUMP(status) << "\n";
  } else if (WIFSTOPPED(status)) {
    nout << "  stopped by " << WSTOPSIG(status) << "\n";
  }
#endif  // _WIN32

  // Notify the instance that we're done.
  P3DInstance *inst = _inst;
  if (inst != NULL) {
    inst->auth_finished_sub_thread();
  }

  nout << "Exiting wt_thread_run in " << this << "\n";
}

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DAuthSession::win_create_process
//       Access: Private
//  Description: Creates a sub-process to run _p3dcert_exe, with
//               the appropriate command-line arguments, and the
//               environment string defined in _env.
//
//               Returns the handle to the created process on success,
//               or INVALID_HANDLE_VALUE on falure.
////////////////////////////////////////////////////////////////////
HANDLE P3DAuthSession::
win_create_process() {
  // Make sure we see an error dialog if there is a missing DLL.
  SetErrorMode(0);

  STARTUPINFO startup_info;
  ZeroMemory(&startup_info, sizeof(startup_info));
  startup_info.cb = sizeof(startup_info); 

  // Make sure the initial window is *shown* for this graphical app.
  startup_info.wShowWindow = SW_SHOW;
  startup_info.dwFlags |= STARTF_USESHOWWINDOW;

  const char *start_dir_cstr = _start_dir.c_str();

  // Construct the command-line string, containing the quoted
  // command-line arguments.
  ostringstream stream;
  stream << "\"" << _p3dcert_exe << "\" \"" 
         << _cert_filename->get_filename() << "\" \"" << _cert_dir << "\"";

  // I'm not sure why CreateProcess wants a non-const char pointer for
  // its command-line string, but I'm not taking chances.  It gets a
  // non-const char array that it can modify.
  string command_line_str = stream.str();
  char *command_line = new char[command_line_str.size() + 1];
  memcpy(command_line, command_line_str.c_str(), command_line_str.size() + 1);

  nout << "Command line: " << command_line_str << "\n";

  // Something about p3dCert_wx tends to become crashy when we call it
  // from CreateProcessW().  Something about the way wx parses the
  // command-line parameters?  Well, whatever, we don't really need
  // the Unicode form anyway.
  PROCESS_INFORMATION process_info; 
  BOOL result = CreateProcess
    (_p3dcert_exe.c_str(), command_line, NULL, NULL, TRUE,
     0, (void *)_env.c_str(), 
     start_dir_cstr, &startup_info, &process_info);
  bool started_program = (result != 0);

  delete[] command_line;

  if (!started_program) {
    nout << "CreateProcess failed, error: " << GetLastError() << "\n";
    return INVALID_HANDLE_VALUE;
  }

  CloseHandle(process_info.hThread);
  return process_info.hProcess;
}
#endif // _WIN32


#ifndef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DAuthSession::posix_create_process
//       Access: Private
//  Description: Creates a sub-process to run _p3dcert_exe, with
//               the appropriate command-line arguments, and the
//               environment string defined in _env.
//
//               Returns the pid of the created process on success, or
//               -1 on falure.
////////////////////////////////////////////////////////////////////
int P3DAuthSession::
posix_create_process() {
  // Fork and exec.
  pid_t child = fork();
  if (child < 0) {
    perror("fork");
    return -1;
  }

  if (child == 0) {
    // Here we are in the child process.

    if (chdir(_start_dir.c_str()) < 0) {
      nout << "Could not chdir to " << _start_dir << "\n";
      // This is a warning, not an error.  We don't actually care
      // that much about the starting directory.
    }

    // build up an array of char strings for the environment.
    vector<const char *> ptrs;
    size_t p = 0;
    size_t zero = _env.find('\0', p);
    while (zero != string::npos) {
      ptrs.push_back(_env.data() + p);
      p = zero + 1;
      zero = _env.find('\0', p);
    }
    ptrs.push_back((char *)NULL);

    execle(_p3dcert_exe.c_str(), _p3dcert_exe.c_str(),
           _cert_filename->get_filename().c_str(), _cert_dir.c_str(),
           (char *)0, &ptrs[0]);
    nout << "Failed to exec " << _p3dcert_exe << "\n";
    _exit(1);
  }

  return child;
}
#endif  // _WIN32
