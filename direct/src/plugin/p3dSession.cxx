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
#include "p3d_plugin_config.h"
#include "p3dUndefinedObject.h"
#include "p3dNoneObject.h"
#include "p3dBoolObject.h"
#include "p3dIntObject.h"
#include "p3dFloatObject.h"
#include "p3dPythonObject.h"
#include "p3dConcreteSequence.h"
#include "p3dConcreteStruct.h"
#include "binaryXml.h"

#ifndef _WIN32
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
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
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  _session_id = inst_mgr->get_unique_id();
  _session_key = inst->get_session_key();
  _python_version = inst->get_python_version();

  _p3dpython_running = false;

  _started_read_thread = false;
  _read_thread_continue = false;

  _output_filename = inst->get_fparams().lookup_token("output_filename");

  _panda3d_callback = NULL;

  INIT_LOCK(_instances_lock);
  INIT_THREAD(_read_thread);

  _panda3d = inst_mgr->get_package("panda3d", "dev", "Panda3D");
  _python_root_dir = _panda3d->get_package_dir();
  inst->add_package(_panda3d);
}


////////////////////////////////////////////////////////////////////
//     Function: P3DSession::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DSession::
~P3DSession() {
  assert(!_p3dpython_running);
  DESTROY_LOCK(_instances_lock);  
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::shutdown
//       Access: Public
//  Description: Terminates the session by shutting down Python and
//               stopping the subprocess.
////////////////////////////////////////////////////////////////////
void P3DSession::
shutdown() {
  if (_panda3d_callback != NULL) {
    _panda3d->cancel_callback(_panda3d_callback);
    delete _panda3d_callback;
    _panda3d_callback = NULL;
  }

  if (_p3dpython_running) {
    // Tell the process we're going away.
    TiXmlDocument doc;
    TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "exit");
    doc.LinkEndChild(decl);
    doc.LinkEndChild(xcommand);
    write_xml(_pipe_write, &doc, nout);

    // Also close the pipe, to help underscore the point.
    _pipe_write.close();

    // Closing _pipe_read before the thread has stopped can result in
    // a hang.  Don't need to close it yet.
    //  _pipe_read.close();

    static const int max_wait_ms = 2000;

#ifdef _WIN32
    // Now give the process a chance to terminate itself cleanly.
    if (WaitForSingleObject(_p3dpython_handle, max_wait_ms) == WAIT_TIMEOUT) {
      // It didn't shut down cleanly, so kill it the hard way.
      nout << "Force-killing python process.\n" << flush;
      TerminateProcess(_p3dpython_handle, 2);
    }

    CloseHandle(_p3dpython_handle);
#else  // _WIN32

    // Wait for a certain amount of time for the process to stop by
    // itself.
    struct timeval start;
    gettimeofday(&start, NULL);
    int start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
    
    int status;
    nout << "waiting for pid " << _p3dpython_pid << "\n" << flush;
    pid_t result = waitpid(_p3dpython_pid, &status, WNOHANG);
    while (result != _p3dpython_pid) {
      if (result == -1) {
        perror("waitpid");
        break;
      }

      struct timeval now;
      gettimeofday(&now, NULL);
      int now_ms = now.tv_sec * 1000 + now.tv_usec / 1000;
      int elapsed = now_ms - start_ms;

      if (elapsed > max_wait_ms) {
        // Tired of waiting.  Kill the process.
        nout << "Force-killing python process, pid " << _p3dpython_pid 
             << "\n" << flush;
        kill(_p3dpython_pid, SIGKILL);
        start_ms = now_ms;
      }
      
      // Yield the timeslice and wait some more.
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 1;
      select(0, NULL, NULL, NULL, &tv);
      result = waitpid(_p3dpython_pid, &status, WNOHANG);
    }

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

    _p3dpython_running = false;
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

  inst->ref();
  ACQUIRE_LOCK(_instances_lock);
  inst->_session = this;
  bool inserted = _instances.insert(Instances::value_type(inst->get_instance_id(), inst)).second;
  RELEASE_LOCK(_instances_lock);
  assert(inserted);

  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "start_instance");
  TiXmlElement *xinstance = inst->make_xml();
  
  doc->LinkEndChild(decl);
  doc->LinkEndChild(xcommand);
  xcommand->LinkEndChild(xinstance);

  send_command(doc);
  inst->send_browser_script_object();

  if (_panda3d->get_ready()) {
    // If it's ready immediately, go ahead and start.
    start_p3dpython();
  } else {
    // Otherwise, set a callback, so we'll know when it is ready.
    if (_panda3d_callback == NULL) {
      _panda3d_callback = new PackageCallback(this);
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
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "terminate_instance");
  xcommand->SetAttribute("instance_id", inst->get_instance_id());
  
  doc->LinkEndChild(decl);
  doc->LinkEndChild(xcommand);

  send_command(doc);

  ACQUIRE_LOCK(_instances_lock);
  if (inst->_session == this) {
    inst->_session = NULL;
    _instances.erase(inst->get_instance_id());
  }
  RELEASE_LOCK(_instances_lock);
  unref_delete(inst);
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
    write_xml(_pipe_write, command, nout);
    delete command;
  } else {
    // Python not yet running.  Queue up the command instead.
    _commands.push_back(command);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::command_and_response
//       Access: Public
//  Description: Sends the indicated command to the running Python
//               process, and waits for a response.  Returns the
//               newly-allocated response on success, or NULL on
//               failure.
//
//               The command must be a newly-allocated TiXmlDocument;
//               it will be deleted after it has been delivered to the
//               process.
//
//               This will fail if the python process is not running
//               or if it suddenly stops.
////////////////////////////////////////////////////////////////////
TiXmlDocument *P3DSession::
command_and_response(TiXmlDocument *command) {
  if (!_p3dpython_running) {
    return NULL;
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  int response_id = inst_mgr->get_unique_id();

  // Add the "want_response_id" attribute to the toplevel command, so
  // the sub-process knows we'll be waiting for its response.
  TiXmlElement *xcommand = command->FirstChildElement("command");
  assert(xcommand != NULL);
  xcommand->SetAttribute("want_response_id", response_id);

  write_xml(_pipe_write, command, nout);
  delete command;

  // Now block, waiting for a response to be delivered.  We assume
  // only one thread will be waiting at a time.
  nout << "waiting for response " << response_id << "\n" << flush;

  _response_ready.acquire();
  Responses::iterator ri = _responses.find(response_id);
  while (ri == _responses.end()) {
    if (!_p3dpython_running) {
      // Hmm, looks like Python has gone away.

      // TODO: make sure _p3dpython_running gets set to false when the
      // process dies unexpectedly.
      _response_ready.release();
      return NULL;
    }

    // Make sure we bake requests while we are waiting, to process
    // recursive script requests.  (The child process might have to
    // wait for us to process some of these before it can fulfill the
    // command we're actually waiting for.)

    // Release the mutex while we do this, so we can safely call back
    // in recursively.
    _response_ready.release();
    Instances::iterator ii;
    for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
      P3DInstance *inst = (*ii).second;
      inst->bake_requests();
    }
    _response_ready.acquire();

    ri = _responses.find(response_id);
    if (ri != _responses.end()) {
      // We got the response we were waiting for while we had the
      // mutex unlocked.
      break;
    }

#ifdef _WIN32
    // Make sure we process the Windows event loop while we're
    // waiting, or everything that depends on Windows messages within
    // the subprocess will starve, and we could end up with deadlock.

    // A single call to PeekMessage() appears to be sufficient.  This
    // will scan the message queue and deliver messages to the
    // appropriate threads, so that our subprocess can find them.  If
    // we don't do this, the messages that come into this parent
    // window will never get delivered to the subprocess, even though
    // somehow the subprocess will know they're coming and will block
    // waiting for them.

    MSG msg;
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_NOYIELD);

    // We wait with a timeout, so we can go back and spin the event
    // loop some more.  On Windows, the timeout needs to be small, so
    // we continue to process windows messages in a timely fashion.
    _response_ready.wait(0.01);

#else
    // On other platforms, we shouldn't need a timeout at all--we
    // could just block indefinitely--but we go ahead and put one in
    // anyway, just in case a notification slips past somehow, and
    // also so we can see evidence that we're actively waiting.  This
    // timeout doesn't need to be nearly so small, since it's only a
    // "just in case" sort of thing.
    _response_ready.wait(0.5);
#endif  // _WIN32

    nout << "." << flush;

    ri = _responses.find(response_id);
  }
  // When we exit the loop, we've found the desired response.
  TiXmlDocument *response = (*ri).second;
  nout << "got response: " << *response << "\n" << flush;
  _responses.erase(ri);

  _response_ready.release();

  return response;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::xml_to_p3dobj
//       Access: Public
//  Description: Converts the XML representation of the particular
//               object value into a corresponding P3D_object.
//               Returns the object, a new reference.
////////////////////////////////////////////////////////////////////
P3D_object *P3DSession::
xml_to_p3dobj(const TiXmlElement *xvalue) {
  const char *type = xvalue->Attribute("type");
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  if (strcmp(type, "undefined") == 0) {
    return inst_mgr->new_undefined_object();

  } else if (strcmp(type, "none") == 0) {
    return inst_mgr->new_none_object();

  } else if (strcmp(type, "bool") == 0) {
    int value;
    if (xvalue->QueryIntAttribute("value", &value) == TIXML_SUCCESS) {
      return inst_mgr->new_bool_object(value != 0);
    }

  } else if (strcmp(type, "int") == 0) {
    int value;
    if (xvalue->QueryIntAttribute("value", &value) == TIXML_SUCCESS) {
      return new P3DIntObject(value);
    }

  } else if (strcmp(type, "float") == 0) {
    double value;
    if (xvalue->QueryDoubleAttribute("value", &value) == TIXML_SUCCESS) {
      return new P3DFloatObject(value);
    }

  } else if (strcmp(type, "string") == 0) {
    // Using the string form here instead of the char * form, so we
    // don't get tripped up on embedded null characters.
    const string *value = xvalue->Attribute(string("value"));
    if (value != NULL) {
      return new P3DStringObject(*value);
    }

  } else if (strcmp(type, "concrete_sequence") == 0) {
    P3DConcreteSequence *obj = new P3DConcreteSequence;
    const TiXmlElement *xitem = xvalue->FirstChildElement("value");
    while (xitem != NULL) {
      P3D_object *item = xml_to_p3dobj(xitem);
      if (item != NULL) {
        obj->append(item);
        P3D_OBJECT_DECREF(item);
      }
      xitem = xitem->NextSiblingElement("value");
    }
    return obj;

  } else if (strcmp(type, "concrete_struct") == 0) {
    P3DConcreteStruct *obj = new P3DConcreteStruct;
    const TiXmlElement *xitem = xvalue->FirstChildElement("value");
    while (xitem != NULL) {
      const char *key = xitem->Attribute("key");
      if (key != NULL) {
        P3D_object *item = xml_to_p3dobj(xitem);
        if (item != NULL) {
          obj->set_property(key, item);
          P3D_OBJECT_DECREF(item);
        }
      }
      xitem = xitem->NextSiblingElement("value");
    }
    return obj;

  } else if (strcmp(type, "browser") == 0) {
    int object_id;
    if (xvalue->QueryIntAttribute("object_id", &object_id) == TIXML_SUCCESS) {
      SentObjects::iterator si = _sent_objects.find(object_id);
      if (si == _sent_objects.end()) {
        // Hmm, the child process gave us a bogus object ID.
        return inst_mgr->new_undefined_object();
      }

      P3D_object *obj = (*si).second;

      P3D_OBJECT_INCREF(obj);
      return obj;
    }

  } else if (strcmp(type, "python") == 0) {
    int object_id;
    if (xvalue->QueryIntAttribute("object_id", &object_id) == TIXML_SUCCESS) {
      return new P3DPythonObject(this, object_id);
    }
  }

  // Something went wrong in decoding.
  return inst_mgr->new_undefined_object();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::p3dobj_to_xml
//       Access: Public
//  Description: Allocates and returns a new XML structure
//               corresponding to the indicated value.  The supplied
//               P3DObject's reference count is not decremented; the
//               caller remains responsible for decrementing it later.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DSession::
p3dobj_to_xml(P3D_object *obj) {
  TiXmlElement *xvalue = new TiXmlElement("value");

  switch (P3D_OBJECT_GET_TYPE(obj)) {
  case P3D_OT_undefined:
    xvalue->SetAttribute("type", "undefined");
    break;

  case P3D_OT_none:
    xvalue->SetAttribute("type", "none");
    break;

  case P3D_OT_bool:
    xvalue->SetAttribute("type", "bool");
    xvalue->SetAttribute("value", (int)P3D_OBJECT_GET_BOOL(obj));
    break;

  case P3D_OT_int:
    xvalue->SetAttribute("type", "int");
    xvalue->SetAttribute("value", P3D_OBJECT_GET_INT(obj));
    break;

  case P3D_OT_float:
    xvalue->SetAttribute("type", "float");
    xvalue->SetDoubleAttribute("value", P3D_OBJECT_GET_FLOAT(obj));
    break;

  case P3D_OT_string:
    {
      xvalue->SetAttribute("type", "string");
      int size = P3D_OBJECT_GET_STRING(obj, NULL, 0);
      char *buffer = new char[size];
      P3D_OBJECT_GET_STRING(obj, buffer, size);
      xvalue->SetAttribute("value", string(buffer, size));
      delete [] buffer;
    }
    break;

  case P3D_OT_object:
    P3DObject *p3dobj = NULL;
    if (obj->_class == &P3DObject::_object_class) {
      p3dobj = (P3DObject *)obj;
    }

    if (p3dobj != NULL && p3dobj->fill_xml(xvalue, this)) {
      // This object has a specialized XML representation, valid for
      // this particular session.  It has already been filled into
      // xvalue.

    } else {
      // Otherwise, it must a host-provided object, or a Python object
      // from another session; which means we should pass a reference
      // down to this particular object, so the Python process knows
      // to call back up to here to query it.

      P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
      int object_id = inst_mgr->get_unique_id();
      bool inserted = _sent_objects.insert(SentObjects::value_type(object_id, obj)).second;
      while (!inserted) {
        // Hmm, we must have cycled around the entire int space?  Either
        // that, or there's a logic bug somewhere.  Assume the former,
        // and keep looking for an empty slot.
        object_id = inst_mgr->get_unique_id();
        inserted = _sent_objects.insert(SentObjects::value_type(object_id, obj)).second;
      }

      // Now that it's stored in the map, increment its reference count.
      P3D_OBJECT_INCREF(obj);

      xvalue->SetAttribute("type", "browser");
      xvalue->SetAttribute("object_id", object_id);
    }
    break;
  }

  return xvalue;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::signal_request_ready
//       Access: Public
//  Description: May be called in any thread to indicate that a new
//               P3D_request is available in the indicated instance.
////////////////////////////////////////////////////////////////////
void P3DSession::
signal_request_ready(P3DInstance *inst) {
  // Since a new request might require baking, we should wake up a
  // blocked command_and_request() process, so the main thread can go
  // back and bake the new request.

  // Technically, a response isn't really ready now, but we still need
  // the main thread to wake up and look around for a bit.
  _response_ready.acquire();
  _response_ready.notify();
  _response_ready.release();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::drop_pyobj
//       Access: Public
//  Description: If the session is still active, issues the command to
//               the child process to release the indicated PyObject
//               from its table.  This is intended to be called
//               strictly by the P3DPythonObject destructor.
////////////////////////////////////////////////////////////////////
void P3DSession::
drop_pyobj(int object_id) {
  nout << "got drop_pyobj(" << object_id << ")\n" << flush;
  if (_p3dpython_running) {
    TiXmlDocument doc;
    TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "drop_pyobj");
    xcommand->SetAttribute("object_id", object_id);
    doc.LinkEndChild(decl);
    doc.LinkEndChild(xcommand);
    write_xml(_pipe_write, &doc, nout);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::drop_p3dobj
//       Access: Public
//  Description: Responds to a drop_p3dobj message from the child
//               process indicating that a particular P3D_object is no
//               longer being used by the child.  This removes the
//               corresponding P3D_object from our tables and
//               decrements its reference count.
////////////////////////////////////////////////////////////////////
void P3DSession::
drop_p3dobj(int object_id) {
  nout << "got drop_p3dobj(" << object_id << ")\n" << flush;
  SentObjects::iterator si = _sent_objects.find(object_id);
  if (si != _sent_objects.end()) {
    P3D_object *obj = (*si).second;
    P3D_OBJECT_DECREF(obj);
    _sent_objects.erase(si);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::install_progress
//       Access: Private
//  Description: Notified as the _panda3d package is downloaded.
////////////////////////////////////////////////////////////////////
void P3DSession::
install_progress(P3DPackage *package, double progress) {
  Instances::iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    P3DInstance *inst = (*ii).second;
    inst->install_progress(package, progress);
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
    nout << "Failed to create process.\n" << flush;
    return;
  }
  _p3dpython_running = true;

  if (!_pipe_read) {
    nout << "unable to open read pipe\n" << flush;
  }
  if (!_pipe_write) {
    nout << "unable to open write pipe\n" << flush;
  }
  
  spawn_read_thread();

  // The very first command we send to the process is its session_id.
  TiXmlDocument doc;
  TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "init");
  xcommand->SetAttribute("session_id", _session_id);
  doc.LinkEndChild(decl);
  doc.LinkEndChild(xcommand);
  write_xml(_pipe_write, &doc, nout);
  
  // Also feed it any commands we may have queued up from before the
  // process was started.
  Commands::iterator ci;
  for (ci = _commands.begin(); ci != _commands.end(); ++ci) {
    write_xml(_pipe_write, (*ci), nout);
    delete (*ci);
  }
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

  _read_thread_continue = true;
  SPAWN_THREAD(_read_thread, rt_thread_run, this);
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

  _read_thread_continue = false;
  _pipe_read.close();

  JOIN_THREAD(_read_thread);
  _started_read_thread = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::rt_thread_run
//       Access: Private
//  Description: The main function for the read thread.
////////////////////////////////////////////////////////////////////
void P3DSession::
rt_thread_run() {
  while (_read_thread_continue) {
    TiXmlDocument *doc = read_xml(_pipe_read, nout);
    if (doc == NULL) {
      // Some error on reading.  Abort.
      rt_terminate();
      return;
    }

    // Successfully read an XML document.
    rt_handle_request(doc);
  }

  logfile << "Exiting rt_thread_run in " << this << "\n" << flush;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::rt_handle_request
//       Access: Private
//  Description: Processes a single request or notification received
//               from an instance.
////////////////////////////////////////////////////////////////////
void P3DSession::
rt_handle_request(TiXmlDocument *doc) {
  nout << "rt_handle_request in " << this << "\n" << flush;
  TiXmlElement *xresponse = doc->FirstChildElement("response");
  if (xresponse != (TiXmlElement *)NULL) {
    int response_id;
    if (xresponse->QueryIntAttribute("response_id", &response_id) == TIXML_SUCCESS) {
      // This is a response to a previous command-and-response.  Send
      // it to the parent thread.
      _response_ready.acquire();
      bool inserted = _responses.insert(Responses::value_type(response_id, doc)).second;
      assert(inserted);
      _response_ready.notify();
      _response_ready.release();
      nout << "done a, rt_handle_request in " << this << "\n" << flush;
      return;
    }
  }

  TiXmlElement *xrequest = doc->FirstChildElement("request");
  if (xrequest != (TiXmlElement *)NULL) {
    int instance_id;
    if (xrequest->QueryIntAttribute("instance_id", &instance_id) == TIXML_SUCCESS) {
      // Look up the particular instance this is related to.
      ACQUIRE_LOCK(_instances_lock);
      Instances::const_iterator ii;
      ii = _instances.find(instance_id);
      if (ii != _instances.end()) {
        P3DInstance *inst = (*ii).second;
        inst->add_raw_request(doc);
        doc = NULL;
      }
      RELEASE_LOCK(_instances_lock);
    }
  }

  if (doc != NULL) {
    delete doc;
  }
  nout << "done rt_handle_request in " << this << "\n" << flush;
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
    nout << "failed to create pipe\n" << flush;
  } else {
    // Make sure the right end of the pipe is inheritable.
    SetHandleInformation(r_to, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(w_to, HANDLE_FLAG_INHERIT, 0);
  }

  // Create the pipe from the process.
  if (!CreatePipe(&r_from, &w_from, NULL, 0)) {
    nout << "failed to create pipe\n" << flush;
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
      nout << "Unable to open " << output_filename << "\n" << flush;
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
        nout << "Unable to open " << output_filename << "\n" << flush;
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
      nout << "Could not chdir to " << start_dir << "\n" << flush;
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
    nout << "Failed to exec " << program << "\n" << flush;
    _exit(1);
  }

  pipe_read.open_read(from_fd[0]);
  pipe_write.open_write(to_fd[1]);
  close(to_fd[0]);
  close(from_fd[1]);

  return child;
}
#endif  // _WIN32


////////////////////////////////////////////////////////////////////
//     Function: P3DSession::PackageCallback::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DSession::PackageCallback::
PackageCallback(P3DSession *session) :
  _session(session)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::PackageCallback::package_ready
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DSession::PackageCallback::
package_ready(P3DPackage *package, bool success) {
  if (this == _session->_panda3d_callback) {
    _session->_panda3d_callback = NULL;
    if (package == _session->_panda3d) {
      if (success) {
        _session->start_p3dpython();
      } else {
        nout << "Failed to install " << package->get_package_name()
             << "_" << package->get_package_version() << "\n";
      }
    } else {
      nout << "Unexpected panda3d package: " << package << "\n";
    }
  } else {
    nout << "Unexpected callback for P3DSession\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::PackageCallback::install_progress
//       Access: Public, Virtual
//  Description: This callback is received during the download process
//               to inform us how much has been installed so far.
////////////////////////////////////////////////////////////////////
void P3DSession::PackageCallback::
install_progress(P3DPackage *package, double progress) {
  if (this == _session->_panda3d_callback) {
    _session->install_progress(package, progress);
  }
}
