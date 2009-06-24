// Filename: p3dSession.cxx
// Created by:  drose (03Jun09)
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

#include "p3dSession.h"
#include "p3dInstance.h"
#include "p3dInstanceManager.h"
#include "p3dProgressWindow.h"
#include "p3dWinProgressWindow.h"
#include "p3d_plugin_config.h"

#ifndef _WIN32
#include <fcntl.h>
#endif

#ifdef _WIN32
typedef P3DWinProgressWindow ProgressWinType;
#else
typedef P3DProgressWindow ProgressWinType;
#endif

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::Constructor
//       Access: Public
//  Description: Creates a new session, corresponding to a new
//               subprocess with its own copy of Python.  The initial
//               parameters for the session are taken from the
//               indicated instance object (but the instance itself is
//               not automatically started within the session).
////////////////////////////////////////////////////////////////////
P3DSession::
P3DSession(P3DInstance *inst) {
  _session_key = inst->get_session_key();
  _python_version = inst->get_python_version();

  _p3dpython_running = false;
  _started_read_thread = false;
  _read_thread_continue = false;

  _output_filename = inst->get_fparams().lookup_token("output_filename");

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  _panda3d = inst_mgr->get_package("panda3d", "dev", "Panda3D");
  _panda3d_callback = NULL;
  _python_root_dir = _panda3d->get_package_dir();

  INIT_LOCK(_instances_lock);
}


////////////////////////////////////////////////////////////////////
//     Function: P3DSession::Destructor
//       Access: Public
//  Description: Terminates the session by shutting down Python and
//               stopping the subprocess.
////////////////////////////////////////////////////////////////////
P3DSession::
~P3DSession() {
  if (_panda3d_callback != NULL) {
    _panda3d->cancel_callback(_panda3d_callback);
    delete _panda3d_callback;
  }

  if (_p3dpython_running) {
    // Tell the process we're going away.
    TiXmlDocument doc;
    TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "exit");
    doc.LinkEndChild(decl);
    doc.LinkEndChild(xcommand);
    _pipe_write << doc << flush;

    // Also close the pipe, to help underscore the point.
    _pipe_write.close();
    _pipe_read.close();

#ifdef _WIN32
    // Now give the process a chance to terminate itself cleanly.
    if (WaitForSingleObject(_p3dpython_handle, 2000) == WAIT_TIMEOUT) {
      // It didn't shut down cleanly, so kill it the hard way.
      nout << "Terminating process.\n";
      TerminateProcess(_p3dpython_handle, 2);
    }

    CloseHandle(_p3dpython_handle);

#else  // _WIN32

    // TODO: posix kill().

#endif  // _WIN32
  }

  // If there are any leftover commands in the queue (presumably
  // implying we have never started the python process), then delete
  // them now, unsent.
  Commands::iterator ci;
  for (ci = _commands.begin(); ci != _commands.end(); ++ci) {
    delete (*ci);
  }
  _commands.clear();

  join_read_thread();
  DESTROY_LOCK(_instances_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::start_instance
//       Access: Public
//  Description: Adds the indicated instance to the session, and
//               starts it running.  It is an error if the instance
//               has been started anywhere else.
//
//               The instance must have the same session_key as the
//               one that was passed to the P3DSession constructor.
////////////////////////////////////////////////////////////////////
void P3DSession::
start_instance(P3DInstance *inst) {
  assert(inst->_session == NULL);
  assert(inst->get_session_key() == _session_key);
  assert(inst->get_python_version() == _python_version);

  ACQUIRE_LOCK(_instances_lock);
  inst->_session = this;
  bool inserted = _instances.insert(Instances::value_type(inst->get_instance_id(), inst)).second;
  RELEASE_LOCK(_instances_lock);
  assert(inserted);

  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "start_instance");
  TiXmlElement *xinstance = inst->make_xml();
  
  doc->LinkEndChild(decl);
  doc->LinkEndChild(xcommand);
  xcommand->LinkEndChild(xinstance);

  send_command(doc);

  if (_panda3d->get_ready()) {
    // If it's ready immediately, go ahead and start.
    start_p3dpython();
  } else {
    // Otherwise, set a callback, so we'll know when it is ready.
    if (_panda3d_callback == NULL) {

      _panda3d_callback = new P3DProgressWindow(_panda3d, this, inst);

      /*
      // The callback object will be a ProgressWindow, to show
      // visual progress to the user while we're downloading.
      if (inst->get_window_type() == P3D_WT_hidden) {
        // For a hidden window, just create an instance of the base
        // class, which manifests no actual window.
        _panda3d_callback = new P3DProgressWindow(_panda3d, this, inst);
      } else {
        // For a non-hidden window, create an instance of
        // ProgressWinType, which is typedeffed above to be the kind
        // of class that actually does manifest a window.
        _panda3d_callback = new ProgressWinType(_panda3d, this, inst);
      }
      */
      _panda3d->set_callback(_panda3d_callback);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::terminate_instance
//       Access: Public
//  Description: Removes the indicated instance from the session, and
//               stops it.  It is an error if the instance is not
//               already running on this session.
////////////////////////////////////////////////////////////////////
void P3DSession::
terminate_instance(P3DInstance *inst) {
  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "terminate_instance");
  xcommand->SetAttribute("id", inst->get_instance_id());
  
  doc->LinkEndChild(decl);
  doc->LinkEndChild(xcommand);

  send_command(doc);

  ACQUIRE_LOCK(_instances_lock);
  if (inst->_session == this) {
    inst->_session = NULL;
    _instances.erase(inst->get_instance_id());
  }
  RELEASE_LOCK(_instances_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::send_command
//       Access: Public
//  Description: Sends the indicated command to the running Python
//               process.  If the process has not yet been started,
//               queues it up until it is ready.
//
//               The command must be a newly-allocated TiXmlDocument;
//               it will be deleted after it has been delivered to the
//               process.
////////////////////////////////////////////////////////////////////
void P3DSession::
send_command(TiXmlDocument *command) {
  if (_p3dpython_running) {
    // Python is running.  Send the command.
    _pipe_write << *command << flush;
    delete command;
  } else {
    // Python not yet running.  Queue up the command instead.
    _commands.push_back(command);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::start_p3dpython
//       Access: Private
//  Description: Starts Python running in a child process.
////////////////////////////////////////////////////////////////////
void P3DSession::
start_p3dpython() {
  string p3dpython = P3D_PLUGIN_P3DPYTHON;
  if (p3dpython.empty()) {
    p3dpython = _python_root_dir + "/p3dpython";
#ifdef _WIN32
    p3dpython += ".exe";
#endif
  }

  // Populate the new process' environment.
  string env;

  // These are the enviroment variables we forward from the current
  // environment, if they are set.
  const char *keep[] = {
    "TMP", "TEMP", "HOME", "USER", 
#ifdef _WIN32
    "SYSTEMROOT", "USERPROFILE",
#endif
    NULL
  };
  for (int ki = 0; keep[ki] != NULL; ++ki) {
    char *value = getenv(keep[ki]);
    if (value != NULL) {
      env += keep[ki];
      env += "=";
      env += value;
      env += '\0';
    }
  }

  // Define some new environment variables.
  env += "PATH=";
  env += _python_root_dir;
  env += '\0';

  env += "PYTHONPATH=";
  env += _python_root_dir;
  env += '\0';

  env += "PRC_DIR=";
  env += _python_root_dir;
  env += '\0';

  env += "PANDA_PRC_DIR=";
  env += _python_root_dir;
  env += '\0';

#ifdef _WIN32
  _p3dpython_handle = win_create_process
    (p3dpython, _python_root_dir, env, _output_filename,
     _pipe_read, _pipe_write);
  bool started_p3dpython = (_p3dpython_handle != INVALID_HANDLE_VALUE);
#else
  _p3dpython_pid = posix_create_process
    (p3dpython, _python_root_dir, env, _output_filename,
     _pipe_read, _pipe_write);
  bool started_p3dpython = (_p3dpython_pid > 0);
#endif

  if (!started_p3dpython) {
    nout << "Failed to create process.\n";
    return;
  }
  _p3dpython_running = true;

  nout << "Created child process\n";

  if (!_pipe_read) {
    nout << "unable to open read pipe\n";
  }
  if (!_pipe_write) {
    nout << "unable to open write pipe\n";
  }
  
  spawn_read_thread();

  // Now that the process has been started, feed it any commands we
  // may have queued up.
  Commands::iterator ci;
  for (ci = _commands.begin(); ci != _commands.end(); ++ci) {
    _pipe_write << *(*ci);
    delete (*ci);
  }
  _pipe_write << flush;
  _commands.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::spawn_read_thread
//       Access: Private
//  Description: Starts the read thread.  This thread is responsible
//               for reading the standard input socket for XML
//               requests and storing them in the _requests queue.
////////////////////////////////////////////////////////////////////
void P3DSession::
spawn_read_thread() {
  assert(!_read_thread_continue);

  // We have to use direct OS calls to create the thread instead of
  // Panda constructs, because it has to be an actual thread, not
  // necessarily a Panda thread (we can't use Panda's simple threads
  // implementation, because we can't get overlapped I/O on an
  // anonymous pipe in Windows).

  _read_thread_continue = true;
#ifdef _WIN32
  _read_thread = CreateThread(NULL, 0, &win_rt_thread_run, this, 0, NULL);
#else
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  pthread_create(&_read_thread, &attr, &posix_rt_thread_run, (void *)this);
  pthread_attr_destroy(&attr);
#endif
  _started_read_thread = true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::join_read_thread
//       Access: Private
//  Description: Waits for the read thread to stop.
////////////////////////////////////////////////////////////////////
void P3DSession::
join_read_thread() {
  if (!_started_read_thread) {
    return;
  }

  nout << "session waiting for thread\n";
  _read_thread_continue = false;
  _pipe_read.close();
  
#ifdef _WIN32
  assert(_read_thread != NULL);
  WaitForSingleObject(_read_thread, INFINITE);
  CloseHandle(_read_thread);
  _read_thread = NULL;
#else
  void *return_val;
  pthread_join(_read_thread, &return_val);
#endif
  nout << "session done waiting for thread\n";

  _started_read_thread = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::rt_thread_run
//       Access: Private
//  Description: The main function for the read thread.
////////////////////////////////////////////////////////////////////
void P3DSession::
rt_thread_run() {
  nout << "session thread reading.\n";
  while (_read_thread_continue) {
    TiXmlDocument *doc = new TiXmlDocument;

    _pipe_read >> *doc;
    if (!_pipe_read || _pipe_read.eof()) {
      // Some error on reading.  Abort.
      nout << "Error on session reading.\n";
      rt_terminate();
      return;
    }

    // Successfully read an XML document.
    nout << "Session got request: " << *doc << "\n";

    // TODO: feed the request up to the parent.
    delete doc;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::rt_terminate
//       Access: Private
//  Description: Got a closed pipe from the sub-process.  Send a
//               terminate request for all instances.
////////////////////////////////////////////////////////////////////
void P3DSession::
rt_terminate() {
  Instances icopy;
  ACQUIRE_LOCK(_instances_lock);
  icopy = _instances;
  RELEASE_LOCK(_instances_lock);

  // TODO: got a race condition here.  What happens if someone deletes
  // an instance while we're processing this loop?

  for (Instances::iterator ii = icopy.begin(); ii != icopy.end(); ++ii) {
    P3DInstance *inst = (*ii).second;
    inst->request_stop();
  }
}

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DSession::win_rt_thread_run
//       Access: Private, Static
//  Description: The Windows flavor of the thread callback function.
////////////////////////////////////////////////////////////////////
DWORD P3DSession::
win_rt_thread_run(LPVOID data) {
  ((P3DSession *)data)->rt_thread_run();
  return 0;
}
#endif

#ifndef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DSession::posix_rt_thread_run
//       Access: Private, Static
//  Description: The Posix flavor of the thread callback function.
////////////////////////////////////////////////////////////////////
void *P3DSession::
posix_rt_thread_run(void *data) {
  ((P3DSession *)data)->rt_thread_run();
  return NULL;
}
#endif


#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DSession::win_create_process
//       Access: Private, Static
//  Description: Creates a sub-process to run the named program
//               executable, with the indicated environment string.
//               Standard error is logged to output_filename, if that
//               string is nonempty.
//
//               Opens the two HandleStreams as the read and write
//               pipes to the child process's standard output and
//               standard input, respectively.
//
//               Returns the handle to the created process on success,
//               or INVALID_HANDLE_VALUE on falure.
////////////////////////////////////////////////////////////////////
HANDLE P3DSession::
win_create_process(const string &program, const string &start_dir,
                   const string &env, const string &output_filename,
                   HandleStream &pipe_read, HandleStream &pipe_write) {

  // Create a bi-directional pipe to communicate with the sub-process.
  HANDLE r_to, w_to, r_from, w_from;

  // Create the pipe to the process.
  if (!CreatePipe(&r_to, &w_to, NULL, 0)) {
    nout << "failed to create pipe\n";
  } else {
    // Make sure the right end of the pipe is inheritable.
    SetHandleInformation(r_to, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(w_to, HANDLE_FLAG_INHERIT, 0);
  }

  // Create the pipe from the process.
  if (!CreatePipe(&r_from, &w_from, NULL, 0)) {
    nout << "failed to create pipe\n";
  } else { 
    // Make sure the right end of the pipe is inheritable.
    SetHandleInformation(w_from, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(r_from, HANDLE_FLAG_INHERIT, 0);
  }

  HANDLE error_handle = GetStdHandle(STD_ERROR_HANDLE);
  bool got_output_filename = !output_filename.empty();
  if (got_output_filename) {
    // Open the named file for output and redirect the child's stderr
    // into it.
    HANDLE handle = CreateFile
      (output_filename.c_str(), GENERIC_WRITE, 
       FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
       NULL, CREATE_ALWAYS, 0, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
      error_handle = handle;
      SetHandleInformation(error_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    } else {
      nout << "Unable to open " << output_filename << "\n";
    }
  }

  // Make sure we see an error dialog if there is a missing DLL.
  SetErrorMode(0);

  // Pass the appropriate ends of the bi-directional pipe as the
  // standard input and standard output of the child process.
  STARTUPINFO startup_info;
  ZeroMemory(&startup_info, sizeof(STARTUPINFO));
  startup_info.cb = sizeof(startup_info); 
  startup_info.hStdError = error_handle;
  startup_info.hStdOutput = w_from;
  startup_info.hStdInput = r_to;
  startup_info.dwFlags |= STARTF_USESTDHANDLES;

  // Make sure the "python" console window is hidden.
  startup_info.wShowWindow = SW_HIDE;
  startup_info.dwFlags |= STARTF_USESHOWWINDOW;

  PROCESS_INFORMATION process_info; 
  BOOL result = CreateProcess
    (program.c_str(), NULL, NULL, NULL, TRUE, 0,
     (void *)env.c_str(), start_dir.c_str(),
     &startup_info, &process_info);
  bool started_program = (result != 0);

  // Close the pipe handles that are now owned by the child.
  CloseHandle(w_from);
  CloseHandle(r_to);
  if (got_output_filename) {
    CloseHandle(error_handle);
  }

  if (!started_program) {
    CloseHandle(r_from);
    CloseHandle(w_to);
    return INVALID_HANDLE_VALUE;
  }

  pipe_read.open_read(r_from);
  pipe_write.open_write(w_to);

  CloseHandle(process_info.hThread);
  return process_info.hProcess;
}
#endif // _WIN32


#ifndef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DSession::posix_create_process
//       Access: Private, Static
//  Description: Creates a sub-process to run the named program
//               executable, with the indicated environment string.
//
//               Opens the two HandleStreams as the read and write
//               pipes to the child process's standard output and
//               standard input, respectively.  Returns true on
//               success, false on failure.
//
//               Returns the pid of the created process on success, or
//               -1 on falure.
////////////////////////////////////////////////////////////////////
int P3DSession::
posix_create_process(const string &program, const string &start_dir,
                     const string &env, const string &output_filename,
                     HandleStream &pipe_read, HandleStream &pipe_write) {
  // Create a bi-directional pipe to communicate with the sub-process.
  int to_fd[2];
  if (pipe(to_fd) < 0) {
    perror("failed to create pipe");
  }
  int from_fd[2];
  if (pipe(from_fd) < 0) {
    perror("failed to create pipe");
  }

  // Fork and exec.
  pid_t child = fork();
  if (child < 0) {
    close(to_fd[0]);
    close(to_fd[1]);
    close(from_fd[0]);
    close(from_fd[1]);
    perror("fork");
    return -1;
  }

  if (child == 0) {
    // Here we are in the child process.
    bool got_output_filename = !output_filename.empty();
    if (got_output_filename) {
      // Open the named file for output and redirect the child's stderr
      // into it.
      int logfile_fd = open(output_filename.c_str(), 
                            O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (logfile_fd < 0) {
        nout << "Unable to open " << output_filename << "\n";
      } else {
        dup2(logfile_fd, STDERR_FILENO);
        close(logfile_fd);
      }
    }

    // Set the appropriate ends of the bi-directional pipe as the
    // standard input and standard output of the child process.
    dup2(to_fd[0], STDIN_FILENO);
    dup2(from_fd[1], STDOUT_FILENO);
    close(to_fd[1]);
    close(from_fd[0]);

    if (chdir(start_dir.c_str()) < 0) {
      nout << "Could not chdir to " << start_dir << "\n";
      _exit(1);
    }

    // build up an array of char strings for the environment.
    vector<const char *> ptrs;
    size_t p = 0;
    size_t zero = env.find('\0', p);
    while (zero != string::npos) {
      ptrs.push_back(env.data() + p);
      p = zero + 1;
      zero = env.find('\0', p);
    }
    ptrs.push_back((char *)NULL);
    
    execle(program.c_str(), program.c_str(), (char *)0, &ptrs[0]);
    nout << "Failed to exec " << program << "\n";
    _exit(1);
  }

  pipe_read.open_read(from_fd[0]);
  pipe_write.open_write(to_fd[1]);
  close(to_fd[0]);
  close(from_fd[1]);

  return child;
}
#endif  // _WIN32

