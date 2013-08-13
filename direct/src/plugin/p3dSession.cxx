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
#include "mkdir_complete.h"
#include "wstring_encode.h"
#include "run_p3dpython.h"

#include <ctype.h>
#include <time.h>

#ifndef _WIN32
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <dlfcn.h>
#endif

#ifdef __APPLE__
#include <crt_externs.h>
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
  _matches_script_origin = inst->get_matches_script_origin();
  _keep_user_env = false;
  _failed = false;

#ifdef _WIN32
  _p3dpython_handle = INVALID_HANDLE_VALUE;
#else
  _p3dpython_pid = -1;
#endif
  _p3dpython_one_process = false;
  _p3dpython_started = false;
  _p3dpython_running = false;

  _started_read_thread = false;
  _read_thread_continue = false;

  INIT_LOCK(_instances_lock);
  INIT_THREAD(_read_thread);
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
  set_failed();

  if (_p3dpython_started) {
    // Tell the process we're going away.
    TiXmlDocument doc;
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "exit");
    doc.LinkEndChild(xcommand);
    write_xml(_pipe_write, &doc, nout);

    // Also close the pipe, to help underscore the point.
    _pipe_write.close();

    // Closing _pipe_read before the thread has stopped can result in
    // a hang.  Don't need to close it yet.
    //  _pipe_read.close();

    static const int max_wait_ms = 2000;

    if (_p3dpython_one_process) {
      // Since it's running in a thread, we can't reliably force-kill
      // it.  So, just wait.
      nout << "Waiting for Python thread to exit\n";
      JOIN_THREAD(_p3dpython_thread);
      nout << "Done waiting.\n";
      _p3dpython_one_process = false;

    } else {
      // Python's running in a sub-process, the preferred way.  In
      // this case, we can wait a brief amount of time before it
      // closes itself; but if it doesn't, we can safely force-kill
      // it.

#ifdef _WIN32
      // Wait for a certain amount of time for the process to stop by
      // itself.
      while (WaitForSingleObject(_p3dpython_handle, max_wait_ms) == WAIT_TIMEOUT) {
        // It didn't shut down cleanly, so kill it the hard way.
        nout << "Force-killing python process.\n";
        TerminateProcess(_p3dpython_handle, 2);
      }
      
      CloseHandle(_p3dpython_handle);
      _p3dpython_handle = INVALID_HANDLE_VALUE;

#else  // _WIN32
      // Wait for a certain amount of time for the process to stop by
      // itself.
      struct timeval start;
      gettimeofday(&start, NULL);
      int start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
      
      int status;
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
               << "\n";
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
      _p3dpython_pid = -1;
      
      nout << "Python process has successfully stopped.\n";
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
    }

    _p3dpython_running = false;
    _p3dpython_started = false;
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

  // Close the pipe now.
  _pipe_read.close();
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
  if (_failed) {
    inst->set_failed();
    return;
  }

  inst->ref();
  ACQUIRE_LOCK(_instances_lock);
  inst->_session = this;
  inst->_instance_started = true;
  bool inserted = _instances.insert(Instances::value_type(inst->get_instance_id(), inst)).second;
  RELEASE_LOCK(_instances_lock);
  assert(inserted);

  TiXmlDocument *doc = new TiXmlDocument;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "start_instance");
  TiXmlElement *xinstance = inst->make_xml();
  
  doc->LinkEndChild(xcommand);
  xcommand->LinkEndChild(xinstance);

  send_command(doc);
  inst->send_browser_script_object();

  // We shouldn't have gotten here unless the instance is fully
  // downloaded and ready to start.
  assert(inst->get_packages_ready());

  start_p3dpython(inst);
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
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "terminate_instance");
  xcommand->SetAttribute("instance_id", inst->get_instance_id());
  
  doc->LinkEndChild(xcommand);

  send_command(doc);

  ACQUIRE_LOCK(_instances_lock);
  if (inst->_session == this) {
    nout << "Assigning " << inst << "->log_pathname = " << _log_pathname << "\n";
    inst->_log_pathname = _log_pathname;
    inst->_session = NULL;
    _instances.erase(inst->get_instance_id());
  }
  RELEASE_LOCK(_instances_lock);
  p3d_unref_delete(inst);
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
  if (_p3dpython_started) {
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
  if (!_p3dpython_started) {
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

  // Now block, waiting for the response to be delivered.
  _response_ready.acquire();
  Responses::iterator ri = _responses.find(response_id);
  while (ri == _responses.end()) {
    if (!_p3dpython_running) {
      // Hmm, looks like Python has gone away.
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
    // TODO: should we acquire _instances_lock?  Deadlock concerns?
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

    ri = _responses.find(response_id);
  }
  // When we exit the loop, we've found the desired response.
  TiXmlDocument *response = (*ri).second;
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
//     Function: P3DSession::send_windows_message
//       Access: Public
//  Description: This is called by the splash window to deliver a
//               windows keyboard message to the Panda process.  It
//               will be called in a sub-thread, but that's OK, since
//               write_xml() supports locking.
////////////////////////////////////////////////////////////////////
void P3DSession::
send_windows_message(P3DInstance *inst, unsigned int msg, int wparam, int lparam) {
  if (_p3dpython_started) {
    TiXmlDocument doc;
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "windows_message");
    xcommand->SetAttribute("instance_id", inst->get_instance_id());
    xcommand->SetAttribute("msg", msg);
    xcommand->SetAttribute("wparam", wparam);
    xcommand->SetAttribute("lparam", lparam);
    doc.LinkEndChild(xcommand);
    write_xml(_pipe_write, &doc, nout);
  }
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
  if (_p3dpython_started) {
    TiXmlDocument doc;
    TiXmlElement *xcommand = new TiXmlElement("command");
    xcommand->SetAttribute("cmd", "drop_pyobj");
    xcommand->SetAttribute("object_id", object_id);
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
  SentObjects::iterator si = _sent_objects.find(object_id);
  if (si != _sent_objects.end()) {
    P3D_object *obj = (*si).second;
    P3D_OBJECT_DECREF(obj);
    _sent_objects.erase(si);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::start_p3dpython
//       Access: Private
//  Description: Starts Python running in a child process.
////////////////////////////////////////////////////////////////////
void P3DSession::
start_p3dpython(P3DInstance *inst) {
  if (_p3dpython_started) {
    // Already started.
    return;
  }

  if (inst->_panda3d_package == NULL) {
    nout << "Couldn't start Python: no panda3d dependency.\n";
    set_failed();
    return;
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  _python_root_dir = inst->_panda3d_package->get_package_dir();
  replace_slashes(_python_root_dir);

  // If we're not to be preserving the user's current directory, then
  // we'll need to change to the standard start directory.
  _keep_user_env = false;
  if (inst_mgr->get_trusted_environment() && 
      inst_mgr->get_console_environment() && 
      inst->_keep_user_env) {
    _keep_user_env = true;
  }
  if (!_keep_user_env) {
    _start_dir = inst_mgr->get_root_dir() + "/start" + inst->get_start_dir_suffix();
    mkdir_complete(_start_dir, nout);
  }
  replace_slashes(_start_dir);

  // Also make sure the prc directory is present.
  string prc_root = inst_mgr->get_root_dir() + "/prc";
  replace_slashes(prc_root);
  mkdir_complete(prc_root, nout);

#ifdef _WIN32
  char sep = ';';
#else
  char sep = ':';
#endif  // _WIN32

  // Build up a search path that includes all of the required packages
  // that have already been installed.  We build this in reverse
  // order, so that the higher-order packages come first in the list;
  // that allows them to shadow settings in the lower-order packages.
  assert(!inst->_packages.empty());
  string search_path;
  size_t pi = inst->_packages.size() - 1;
  search_path = inst->_packages[pi]->get_package_dir();
  while (pi > 0) {
    --pi;
    search_path += sep;
    search_path += inst->_packages[pi]->get_package_dir();
  }

  replace_slashes(search_path);
  nout << "Search path is " << search_path << "\n";

  bool keep_pythonpath = false;
  if (inst->_allow_python_dev) {
    // If "allow_python_dev" is set in the instance's p3d_info.xml,
    // *and* we have keep_pythonpath in the tokens, then we set
    // keep_pythonpath true.
    keep_pythonpath = (inst->get_fparams().lookup_token_int("keep_pythonpath") != 0);
  }

  string sys_path = search_path;
  string ld_path = search_path;
  string dyld_path = search_path;
  string python_path = search_path;
  string prc_path = prc_root + sep + search_path;

  string prc_name = inst->get_fparams().lookup_token("prc_name");
  if (prc_name.empty()) {
    prc_name = inst->_prc_name;
    
    if (!prc_name.empty()) {
      // If the prc_name is taken from the p3d file (and not from the
      // HTML tokens), then we also append the alt_host name to the
      // prc_name, so that each alt_host variant will run in a
      // different directory.
      string alt_host = inst->get_fparams().lookup_token("alt_host");
      if (!alt_host.empty()) {
        prc_name += "_";
        prc_name += alt_host;
      }
    }
  }
  if (!prc_name.empty()) {
    // Add the prc_name to the path too, even if this directory doesn't
    // actually exist.
    string this_prc_dir = inst_mgr->get_root_dir() + "/prc";
    inst_mgr->append_safe_dir(this_prc_dir, prc_name);
    replace_slashes(this_prc_dir);
    prc_path = this_prc_dir + sep + prc_path;    
  }

  if (keep_pythonpath) {
    // With keep_pythonpath true, we preserve the PYTHONPATH setting
    // from the caller's environment; in fact, we put it in the front.
    // This allows the caller's on-disk Python files to shadow the
    // similar-named files in the p3d file, allowing easy iteration on
    // the code in the p3d file.
    if (get_env(python_path, "PYTHONPATH")) {
      replace_slashes(python_path);
      python_path += sep;
      python_path += search_path;
    }

    // We also preserve PRC_PATH.
    if (get_env(prc_path, "PRC_PATH") || get_env(prc_path, "PANDA_PRC_PATH")) {
      replace_slashes(prc_path);
      prc_path += sep;
      prc_path += search_path;
    }

    nout << "keep_pythonpath is true\n"
         << "PYTHONPATH set to: " << python_path << "\n"
         << "PRC_PATH set to: " << prc_path << "\n";
  }

  // Get the name of the executable to run.  Ideally, we'll run the
  // executable successfully, in a sub-process; this will in turn load
  // and run the dynamic library.  If that fails for some reason, we
  // can fall back to loading and running the library directly.
  _p3dpython_exe = P3D_PLUGIN_P3DPYTHON;
  string p3dpythonw_exe = _p3dpython_exe + "w";
  if (_p3dpython_exe.empty()) {
    // Allow package to override the name of the p3dpython executables.
    const char *p3dpython_name_xconfig = NULL; 
    const char *p3dpythonw_name_xconfig = NULL;
    const TiXmlElement *panda3d_xconfig = inst->_panda3d_package->get_xconfig();
    if (panda3d_xconfig != NULL) {
      p3dpython_name_xconfig = panda3d_xconfig->Attribute("p3dpython_name");
      p3dpythonw_name_xconfig = panda3d_xconfig->Attribute("p3dpythonw_name");
    }

    string p3dpython_name = "p3dpython";
    if (p3dpython_name_xconfig != NULL) {
      nout << "p3dpython_name from panda3d xconfig: " << p3dpython_name_xconfig << "\n";
      p3dpython_name = p3dpython_name_xconfig;
    }

    string p3dpythonw_name = p3dpython_name + "w";
    if (p3dpythonw_name_xconfig != NULL) {
      nout << "p3dpythonw_name from panda3d xconfig: " << p3dpythonw_name_xconfig << "\n";
      p3dpythonw_name = p3dpythonw_name_xconfig;
    }

    // Build full executable path.
    _p3dpython_exe = _python_root_dir + "/" + p3dpython_name;
    p3dpythonw_exe = _python_root_dir + "/" + p3dpythonw_name;
#ifdef __APPLE__
    // On OSX, run from the packaged bundle, if it exists.
    string bundle_exe = _python_root_dir + "/P3DPython.app/Contents/MacOS/p3dpython";
    if (access(bundle_exe.c_str(), X_OK) == 0) {
      _p3dpython_exe = bundle_exe;
    }
#endif
  }
#ifdef _WIN32
  if (!inst_mgr->get_console_environment()) {
    _p3dpython_exe = p3dpythonw_exe;
  }
  _p3dpython_exe += ".exe";
#endif
  replace_slashes(_p3dpython_exe);
  nout << "_p3dpython_exe: " << _p3dpython_exe << "\n";

  // Populate the new process' environment.
  _env = string();

  if (!_keep_user_env) {
    // Reconstruct an environment just for running the process.
    // Completely replace most of the existing environment variables
    // with our own.

    // These are the enviroment variables we forward from the current
    // environment, if they are set.
    const char *keep[] = {
      "HOME", "USER", 
#ifdef _WIN32
      "SYSTEMROOT", "USERPROFILE", "COMSPEC",
      "SYSTEMDRIVE", "ALLUSERSPROFILE", "APPDATA", "COMMONPROGRAMFILES",
      "PROGRAMFILES", "WINDIR", "PROGRAMDATA", "USERDOMAIN",
#endif
#ifdef HAVE_X11
      "DISPLAY", "XAUTHORITY",
#endif
      NULL
    };
    for (int ki = 0; keep[ki] != NULL; ++ki) {
      string value;
      if (get_env(value, keep[ki])) {
        _env += keep[ki];
        _env += "=";
        _env += value;
        _env += '\0';
      }
    }

  } else {
    // In a trusted environment, when the application asks us to, we
    // forward *all* environment variables, except those defined
    // specifically below.
    const char *dont_keep[] = {
      "PATH", "LD_LIBRARY_PATH", "DYLD_LIBRARY_PATH",
      "PYTHONPATH", "PYTHONHOME", "PRC_PATH", "PANDA_PRC_PATH",
      "TEMP", "CTPROJS",
      NULL
    };

#ifdef _WIN32
    // Windows has a leading underscore in the name, and the word
    // "environ" is a keyword. (!)
    extern char **_environ;
    char **global_environ = _environ;
#elif defined(__APPLE__)
    // Apple doesn't guarantee that environ is available for shared
    // libraries, but provides _NSGetEnviron().
    char **global_environ = *_NSGetEnviron();
#else
    // Posix is straightforward.
    extern char **environ;
    char **global_environ = environ;
#endif  // _WIN32

    char **ep;
    for (ep = global_environ; *ep != NULL; ++ep) {
      string env = *ep;
      size_t equals = env.find('=');
      if (equals != string::npos) {
        string var = env.substr(0, equals);
        const char *varc = var.c_str();
        bool found = false;
        for (int i = 0; dont_keep[i] != NULL && !found; ++i) {
          found = (strcmp(dont_keep[i], varc) == 0);
        }
        if (!found) {
          // This variable is OK, keep it.
          _env += env;
          _env += '\0';
        }
      }
    }
  }

  // We also append the original PATH et al to the *end* of the new
  // definitions, even if keep_user_env is not set.  This is necessary
  // for os.system() and such to work as expected within the embedded
  // app.  It's also necessary for webbrowser on Linux.
  string orig_path;
  if (get_env(orig_path, "PATH")) {
    sys_path += sep;
    sys_path += orig_path;
  }
  string orig_ld_path;
  if (get_env(orig_ld_path, "LD_LIBRARY_PATH")) {
    ld_path += sep;
    ld_path += orig_ld_path;
  }
  string orig_dyld_path;
  if (get_env(orig_dyld_path, "DYLD_LIBRARY_PATH")) {
    dyld_path += sep;
    dyld_path += orig_dyld_path;
  }

  // Define some new environment variables.
  _env += "PATH=";
  _env += sys_path;
  _env += '\0';

  _env += "LD_LIBRARY_PATH=";
  _env += ld_path;
  _env += '\0';

  _env += "DYLD_LIBRARY_PATH=";
  _env += dyld_path;
  _env += '\0';

  _env += "PYTHONPATH=";
  _env += python_path;
  _env += '\0';

  // Let's leave PYTHONHOME empty.  Setting it adds junk to our
  // carefully-constructed PYTHONPATH.
  _env += "PYTHONHOME=";
  _env += '\0';

  _env += "PRC_PATH=";
  _env += prc_path;
  _env += '\0';

  _env += "PANDA_PRC_PATH=";
  _env += prc_path;
  _env += '\0';

  _env += "TEMP=";
  string temp_dir = inst_mgr->get_temp_directory();
  replace_slashes(temp_dir);
  _env += temp_dir;
  _env += '\0';
  
  // Define each package's root directory in an environment variable
  // named after the package, for the convenience of the packages in
  // setting up their config files.
  for (size_t pi = 0; pi < inst->_packages.size(); ++pi) {
    P3DPackage *package = inst->_packages[pi];
    const string package_name = package->get_package_name();
    for (string::const_iterator si = package_name.begin();
         si != package_name.end();
         ++si) {
      _env += toupper(*si);
    }
    _env += string("_ROOT=");
    _env += package->get_package_dir();
    _env += '\0';

    package->mark_used();
  }

  // Check for a few tokens that have special meaning at this level.
  bool console_output = (inst->get_fparams().lookup_token_int("console_output") != 0);
  bool one_process = (inst->get_fparams().lookup_token_int("one_process") != 0);
  _interactive_console = (inst->get_fparams().lookup_token_int("interactive_console") != 0);

  if (!inst->_allow_python_dev) {
    // interactive_console is only allowed to be enabled if
    // allow_python_dev is also set within the p3d file.
    _interactive_console = false;
  }

  if (_interactive_console) {
    // If we have interactive_console set, it follows we also need
    // console_output.
    console_output = true;
  }

  // Get the log filename from the HTML tokens, or from the
  // p3d_info.xml file.
  string log_basename = inst->get_fparams().lookup_token("log_basename");
  if (log_basename.empty()) {
    log_basename = inst->_log_basename;

    if (!log_basename.empty()) {
      // If the log_basename is taken from the p3d file (and not from
      // the HTML tokens), then we also append the alt_host name to
      // the log_basename, so that each alt_host variant will run in a
      // different directory.
      string alt_host = inst->get_fparams().lookup_token("alt_host");
      if (!alt_host.empty()) {
        log_basename += "_";
        log_basename += alt_host;
      }
    }
  }

  if (log_basename.empty()) {
#ifdef P3D_PLUGIN_LOG_BASENAME3
    // No log_basename specified for the app; use the compiled-in
    // default.
    log_basename = P3D_PLUGIN_LOG_BASENAME3;
#endif
    if (log_basename.empty()) {
      log_basename = "p3dsession";
    }
  }

  // However, it is always written into the log directory only; the
  // user may not override the log file to put it anywhere else.
  size_t slash = log_basename.rfind('/');
  if (slash != string::npos) {
    log_basename = log_basename.substr(slash + 1);
  }
#ifdef _WIN32
  slash = log_basename.rfind('\\');
  if (slash != string::npos) {
    log_basename = log_basename.substr(slash + 1);
  }
#endif  // _WIN32

  // Get the log history count from the HTML tokens, or from the
  // p3d_info.xml file.
  int log_history = inst->get_fparams().lookup_token_int("log_history");

  // Check if we want to keep copies of recent logs on disk.
  if (!log_basename.empty()) {
    // Get a list of all logs on disk
    vector<string> all_logs;
    string log_directory = inst_mgr->get_log_directory();
    inst_mgr->scan_directory(log_directory, all_logs);

    // If keeping logs, only logs with a -timestamp suffix are valid.
    if (log_history > 0) {
      // Remove exact match (no suffix) file, if it is on disk
      string log_exact_leafname = (log_basename + string(".log"));
      for (int i=0; i<(int)all_logs.size(); ++i) {
        if (all_logs[i] == log_exact_leafname) {
          string log_exact_pathname = (log_directory + log_exact_leafname);
          unlink(log_exact_pathname.c_str());
          break;
        }
      }
    }

    // Remove all but the most recent log_history timestamped logs
    string log_basename_dash = (log_basename + string("-"));
    string log_matching_pathname;
    vector<string> matching_logs;
    for (int i=0; i<(int)all_logs.size(); ++i) {
      if ((all_logs[i].size() > 4) &&
          (all_logs[i].find(log_basename_dash) == 0) &&
          (all_logs[i].substr(all_logs[i].size() - 4) == string(".log"))) {
        log_matching_pathname = (log_directory + all_logs[i]);
        matching_logs.push_back(log_matching_pathname);
      }
    }
    for (int i=0; i<(int)matching_logs.size()-log_history; ++i) {
      unlink(matching_logs[i].c_str());
    }
  }

  // Append a timestamp suffix to the log_basename
  if (!log_basename.empty() && (log_history > 0)) {
#ifdef _WIN32
    _tzset();
#else
    tzset();
#endif
    time_t log_time_seconds = time(NULL);
    struct tm *log_time_local_p = localtime(&log_time_seconds);
    if (log_time_local_p != NULL) {
      struct tm log_time_local = *log_time_local_p;
      static const size_t buffer_size = 16;
      char buffer[buffer_size];
      sprintf(buffer, "%02d%02d%02d_%02d%02d%02d", 
              (int)(log_time_local.tm_year+1900-2000),
              (int)(log_time_local.tm_mon+1),
              (int)(log_time_local.tm_mday),
              (int)(log_time_local.tm_hour),
              (int)(log_time_local.tm_min),
              (int)(log_time_local.tm_sec));
      log_basename += "-";
      log_basename += buffer;
    }
  }

  if (!console_output && !log_basename.empty()) {
    _log_pathname = inst_mgr->get_log_directory();
    _log_pathname += log_basename;

    // We always tack on the extension ".log", to make it even more
    // difficult to overwrite a system file.
    _log_pathname += ".log";
  }

  // Create the pipes for communication.
#ifdef _WIN32
  // Create a bi-directional pipe to communicate with the sub-process.
  HANDLE r_to, w_to, r_from, w_from;

  // Create the pipe to the process.
  if (!CreatePipe(&r_to, &w_to, NULL, 0)) {
    nout << "failed to create pipe\n";
    set_failed();
  } else {
    // Make sure the right end of the pipe is inheritable.
    SetHandleInformation(r_to, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(w_to, HANDLE_FLAG_INHERIT, 0);
  }

  // Create the pipe from the process.
  if (!CreatePipe(&r_from, &w_from, NULL, 0)) {
    nout << "failed to create pipe\n";
    set_failed();
  } else { 
    // Make sure the right end of the pipe is inheritable.
    SetHandleInformation(w_from, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(r_from, HANDLE_FLAG_INHERIT, 0);
  }

  _output_handle = w_from;
  _input_handle = r_to;
  _pipe_read.open_read(r_from);
  _pipe_write.open_write(w_to);

#else
  // Create a bi-directional pipe to communicate with the sub-process.
  int to_fd[2];
  if (pipe(to_fd) < 0) {
    perror("failed to create pipe");
    set_failed();
  }
  int from_fd[2];
  if (pipe(from_fd) < 0) {
    perror("failed to create pipe");
    set_failed();
  }

  _input_handle = to_fd[0];
  _output_handle = from_fd[1];
  _pipe_read.open_read(from_fd[0]);
  _pipe_write.open_write(to_fd[1]);
#endif // _WIN32

  if (_failed) {
    return;
  }

  nout << "Setting environment:\n";
  write_env();

  // Get the filename of the Panda3D multifile.  We need to pass this
  // to p3dpython.
  _mf_filename = inst->_panda3d_package->get_archive_file_pathname();

  nout << "Attempting to start python from " << _p3dpython_exe << "\n";

  bool started_p3dpython;
  if (one_process) {
    nout << "one_process is set; running Python within parent process.\n";
    started_p3dpython = false;
  } else {
#ifdef _WIN32
    _p3dpython_handle = win_create_process();
    started_p3dpython = (_p3dpython_handle != INVALID_HANDLE_VALUE);
#else
    _p3dpython_pid = posix_create_process();
    started_p3dpython = (_p3dpython_pid > 0);
#endif
    if (!started_p3dpython) {
      nout << "Failed to create process.\n";
    }
  }

  if (!started_p3dpython) {
    // Well, we couldn't run python in a sub-process, for some reason.
    // Fall back to running it in a sub-thread within the same
    // process.  This isn't nearly as good, but I guess it's better
    // than nothing.

    INIT_THREAD(_p3dpython_thread);
    SPAWN_THREAD(_p3dpython_thread, p3dpython_thread_run, this);
    _p3dpython_one_process = true;
  }
  _p3dpython_started = true;
  _p3dpython_running = true;

  if (!_pipe_read) {
    nout << "unable to open read pipe\n";
  }
  if (!_pipe_write) {
    nout << "unable to open write pipe\n";
  }
  
  spawn_read_thread();

  // The very first command we send to the process is its session_id.
  TiXmlDocument doc;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "init");
  xcommand->SetAttribute("session_id", _session_id);
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
//     Function: P3DSession::set_failed
//       Access: Private
//  Description: Sets the "failed" indication to display sadness to
//               the user--we're unable to launch the instance for
//               some reason.
//
//               When this is called on the P3DSession instead of on a
//               particular P3DInstance, it means that all instances
//               attached to this session are marked failed.
////////////////////////////////////////////////////////////////////
void P3DSession::
set_failed() {
  _failed = true;

  Instances::iterator ii;
  ACQUIRE_LOCK(_instances_lock);
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    P3DInstance *inst = (*ii).second;
    inst->set_failed();
  }
  RELEASE_LOCK(_instances_lock);
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
  assert(!_started_read_thread && !_read_thread_continue);

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

  JOIN_THREAD(_read_thread);
  _started_read_thread = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::replace_slashes
//       Access: Private, Static
//  Description: Changes the forward slashes to backslashes on
//               Windows.  Does nothing on the other platforms.
////////////////////////////////////////////////////////////////////
void P3DSession::
replace_slashes(string &str) {
#ifdef _WIN32
  // It turns out that some very low-level Windows functions fail when
  // you give them a forward slash instead of a backslash.  In
  // particular, Windows fails to load the MSVS runtime DLL's (and
  // their associated manifest files) correctly in this case.  So we
  // have to be sure to replace forward slashes in our PATH variable
  // (and other environment variables, for good measure) with
  // backslashes.
  for (size_t i = 0; i < str.length(); ++i) {
    if (str[i] == '/') {
      str[i] = '\\';
    }
  }
#endif  // _WIN32
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
      _p3dpython_running = false;
      _response_ready.acquire();
      _response_ready.notify();
      _response_ready.release();
      return;
    }

    // Successfully read an XML document.
    rt_handle_request(doc);
  }

  nout << "Exiting rt_thread_run in " << this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::rt_handle_request
//       Access: Private
//  Description: Processes a single request or notification received
//               from an instance.  This method runs in the read
//               thread.
////////////////////////////////////////////////////////////////////
void P3DSession::
rt_handle_request(TiXmlDocument *doc) {
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
    inst->request_stop_main_thread();
  }
}

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DSession::win_create_process
//       Access: Private
//  Description: Creates a sub-process to run _p3dpython_exe, with
//               the appropriate command-line arguments, and the
//               environment string defined in _env.  Standard error
//               is logged to _log_pathname, if that string is
//               nonempty.
//
//               Opens the two HandleStreams _pipe_read and
//               _pipe_write as the read and write pipes to the child
//               process's standard output and standard input,
//               respectively.
//
//               Returns the handle to the created process on success,
//               or INVALID_HANDLE_VALUE on falure.
////////////////////////////////////////////////////////////////////
HANDLE P3DSession::
win_create_process() {
  // Make sure we see an error dialog if there is a missing DLL.
  SetErrorMode(0);

  // Open the log file.
  HANDLE error_handle = GetStdHandle(STD_ERROR_HANDLE);
  bool got_error_handle = false;
  if (!_log_pathname.empty()) {
    wstring log_pathname_w;
    string_to_wstring(log_pathname_w, _log_pathname);
    HANDLE handle = CreateFileW
      (log_pathname_w.c_str(), GENERIC_WRITE, 
       FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
       NULL, CREATE_ALWAYS, 0, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
      error_handle = handle;
      SetHandleInformation(error_handle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
      got_error_handle = true;
    } else {
      nout << "Unable to open " << _log_pathname << "\n";
    }
  }

  STARTUPINFOW startup_info;
  ZeroMemory(&startup_info, sizeof(startup_info));
  startup_info.cb = sizeof(startup_info); 

  // Set up the I/O handles.  We send stderr and stdout to our
  // error_handle.
  startup_info.hStdError = error_handle;
  startup_info.hStdOutput = error_handle;
  startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  startup_info.dwFlags |= STARTF_USESTDHANDLES;

  // We want to show the output window these days.
  startup_info.wShowWindow = SW_SHOW;
  startup_info.dwFlags |= STARTF_USESHOWWINDOW;

  // If _keep_user_env is true, meaning not to change the current
  // directory, then pass NULL in to CreateProcess().  Otherwise pass
  // in _start_dir.
  const wchar_t *start_dir_cstr;
  wstring start_dir_w;
  if (_keep_user_env) {
    start_dir_cstr = NULL;
    nout << "Not changing working directory.\n";
  } else {
    string_to_wstring(start_dir_w, _start_dir);
    start_dir_cstr = start_dir_w.c_str();
    nout << "Setting working directory: " << _start_dir << "\n";
  }

  // Construct the command-line string, containing the quoted
  // command-line arguments.
  ostringstream stream;
  stream << "\"" << _p3dpython_exe << "\" \"" << _mf_filename
         << "\" \"" << _input_handle << "\" \"" << _output_handle
         << "\" \"" << _interactive_console << "\"";

  // I'm not sure why CreateProcess wants a non-const char pointer for
  // its command-line string, but I'm not taking chances.  It gets a
  // non-const char array that it can modify.
  wstring command_line_str;
  string_to_wstring(command_line_str, stream.str());
  wchar_t *command_line = new wchar_t[command_line_str.size() + 1];
  memcpy(command_line, command_line_str.c_str(), sizeof(wchar_t) * command_line_str.size() + 1);

  nout << "Command line: " << command_line_str << "\n";

  wstring p3dpython_exe_w;
  string_to_wstring(p3dpython_exe_w, _p3dpython_exe);
  wstring env_w;
  string_to_wstring(env_w, _env);

  PROCESS_INFORMATION process_info; 
  BOOL result = CreateProcessW
    (p3dpython_exe_w.c_str(), command_line, NULL, NULL, TRUE, 
     CREATE_UNICODE_ENVIRONMENT, (void *)env_w.c_str(), 
     start_dir_cstr, &startup_info, &process_info);
  bool started_program = (result != 0);

  if (!started_program) {
    nout << "CreateProcess failed, error: " << GetLastError() << "\n";
  }

  delete[] command_line;

  // Close the pipe handles that are now owned by the child.
  CloseHandle(_output_handle);
  CloseHandle(_input_handle);
  if (got_error_handle) {
    CloseHandle(error_handle);
  }

  if (!started_program) {
    _pipe_read.close();
    _pipe_write.close();
    return INVALID_HANDLE_VALUE;
  }

  CloseHandle(process_info.hThread);
  return process_info.hProcess;
}
#endif // _WIN32


#ifndef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: P3DSession::posix_create_process
//       Access: Private
//  Description: Creates a sub-process to run _p3dpython_exe, with
//               the appropriate command-line arguments, and the
//               environment string defined in _env.  Standard error
//               is logged to _log_pathname, if that string is
//               nonempty.
//
//               Opens the two HandleStreams _pipe_read and
//               _pipe_write as the read and write pipes to the child
//               process's standard output and standard input,
//               respectively.
//
//               Returns the pid of the created process on success, or
//               -1 on falure.
////////////////////////////////////////////////////////////////////
int P3DSession::
posix_create_process() {
  // If the program file doesn't exist or isn't executable, don't even
  // bother to try.
  if (access(_p3dpython_exe.c_str(), X_OK) != 0) {
    return -1;
  }

  // Fork and exec.
  pid_t child = fork();
  if (child < 0) {
    perror("fork");
    return -1;
  }

  if (child == 0) {
    // Here we are in the child process.

    // Close the parent's ends of the pipes.
    _pipe_read.close();
    _pipe_write.close();

    if (!_log_pathname.empty()) {
      // Open a logfile.
      int logfile_fd = open(_log_pathname.c_str(), 
                            O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (logfile_fd < 0) {
        nout << "Unable to open " << _log_pathname << "\n";
      } else {
        // Redirect stderr and stdout onto our logfile.
        dup2(logfile_fd, STDERR_FILENO);
        dup2(logfile_fd, STDOUT_FILENO);
        close(logfile_fd);
      }
    }

    if (_keep_user_env) {
      nout << "Not changing working directory.\n";
    } else {
      nout << "Setting working directory: " << _start_dir << "\n";
      if (chdir(_start_dir.c_str()) < 0) {
        nout << "Could not chdir to " << _start_dir << "\n";
        // This is a warning, not an error.  We don't actually care
        // that much about the starting directory.
      }
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

    stringstream input_handle_stream;
    input_handle_stream << _input_handle;
    string input_handle_str = input_handle_stream.str();

    stringstream output_handle_stream;
    output_handle_stream << _output_handle;
    string output_handle_str = output_handle_stream.str();

    execle(_p3dpython_exe.c_str(), _p3dpython_exe.c_str(),
           _mf_filename.c_str(), input_handle_str.c_str(),
           output_handle_str.c_str(),
           _interactive_console ? "1" : "0", (char *)0, &ptrs[0]);
    nout << "Failed to exec " << _p3dpython_exe << "\n";
    _exit(1);
  }

  // Close the handles that are now owned by the child.
  close(_input_handle);
  close(_output_handle);

  // Let's wait a few milliseconds and see if the child is going to
  // immediately exit with a failure status.  This isn't 100%
  // reliable, but it's a lot easier than sending back a "yes I
  // successfully started the program" message.  Maybe I'll put in the
  // more reliable test later.

  struct timeval start;
  gettimeofday(&start, NULL);
  int start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
  
  int status;
  pid_t result = waitpid(child, &status, WNOHANG);
  while (result != child) {
    if (result == -1) {
      perror("waitpid");
      break;
    }
    
    struct timeval now;
    gettimeofday(&now, NULL);
    int now_ms = now.tv_sec * 1000 + now.tv_usec / 1000;
    int elapsed = now_ms - start_ms;
    if (elapsed > 100) {
      // OK, we've waited, and the child process is still alive.
      // Assume it will stay that way.
      nout << "child still alive after " << elapsed << " ms\n";
      return child;
    }
        
    // Yield the timeslice and wait some more.
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    select(0, NULL, NULL, NULL, &tv);
    result = waitpid(child, &status, WNOHANG);
  }

  // The child process died for some reason; maybe it couldn't exec()
  // its process.  Report an error condition.
  nout << "Python process stopped immediately.\n";
  if (WIFEXITED(status)) {
    nout << "  exited normally, status = "
         << WEXITSTATUS(status) << "\n";
  } else if (WIFSIGNALED(status)) {
    nout << "  signalled by " << WTERMSIG(status) << ", core = " 
         << WCOREDUMP(status) << "\n";
  } else if (WIFSTOPPED(status)) {
    nout << "  stopped by " << WSTOPSIG(status) << "\n";
  }
  
  return -1;
}
#endif  // _WIN32

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::p3dpython_thread_run
//       Access: Private
//  Description: This method is called in a sub-thread to fire up
//               p3dpython within this same process, but only if the
//               above attempt to create a sub-process failed.
////////////////////////////////////////////////////////////////////
void P3DSession::
p3dpython_thread_run() {
  nout << "running p3dpython_thread_run()\n";

  // Set the environment.  Hopefully this won't be too destructive to
  // the current process.

  // Note that on OSX at least, changing the DYLD_LIBRARY_PATH after
  // the process has started has no effect (and furthermore you can't
  // specify a full path to dlopen() calls), so this whole one-process
  // approach is fatally flawed on OSX.
  size_t p = 0;
  size_t zero = _env.find('\0', p);
  while (zero != string::npos) {
    const char *start = _env.data() + p;
#ifdef _WIN32
    _putenv(start);
#else
    const char *equals = strchr(start, '=');
    if (equals != NULL) {
      string variable(start, equals - start);
      setenv(variable.c_str(), equals + 1, true);
    }
#endif  // _WIN32
    p = zero + 1;
    zero = _env.find('\0', p);
  }

  // Now load the library.
  string libp3dpython = _python_root_dir + "/libp3dpython";
#ifdef _WIN32
#ifdef _DEBUG
  libp3dpython += "_d.dll";
#else  
  libp3dpython += ".dll";
#endif
  SetErrorMode(0);
  HMODULE module = LoadLibrary(libp3dpython.c_str());
  if (module == NULL) {
    // Couldn't load the DLL.
    nout << "Couldn't load " << libp3dpython << "\n";
    return;
  }

  #define get_func GetProcAddress

#else  // _WIN32
  // Posix case.
  #ifdef __APPLE__
  libp3dpython += ".dylib";
  #else
  libp3dpython += ".so";
  #endif
  void *module = dlopen(libp3dpython.c_str(), RTLD_LAZY | RTLD_LOCAL);
  if (module == NULL) {
    // Couldn't load the .so.
    nout << "Couldn't load " << libp3dpython << "\n";
    return;
  }

  #define get_func dlsym

#endif  // _WIN32

  run_p3dpython_func *run_p3dpython = (run_p3dpython_func *)get_func(module, "run_p3dpython");
  if (run_p3dpython == NULL) {
    nout << "Couldn't find run_p3dpython\n";
    return;
  }

  if (!run_p3dpython(libp3dpython.c_str(), _mf_filename.c_str(),
                     _input_handle, _output_handle, _log_pathname.c_str(),
                     _interactive_console)) {
    nout << "Failure on startup.\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::get_env
//       Access: Private, Static
//  Description: Implements getenv(), respecting Windows' Unicode
//               environment.  Returns true if the variable is
//               defined, false if it is not.  If it is defined, fills
//               value with its definition.
////////////////////////////////////////////////////////////////////
bool P3DSession::
get_env(string &value, const string &varname) {
#ifdef _WIN32
  wstring varname_w;
  string_to_wstring(varname_w, varname);
  const wchar_t *vc = _wgetenv(varname_w.c_str());
  if (vc == NULL) {
    return false;
  }
  wstring_to_string(value, vc);
  return true;
#else  // _WIN32
  const char *vc = getenv(varname.c_str());
  if (vc == NULL) {
    return false;
  }
  value = vc;
  return true;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::write_env
//       Access: Private
//  Description: Writes _env, which is formatted as a string
//               containing zero-byte-terminated environment
//               defintions, to the nout stream, one definition per
//               line.
////////////////////////////////////////////////////////////////////
void P3DSession::
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

