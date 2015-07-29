// Filename: p3dMainObject.cxx
// Created by:  drose (10Jul09)
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

#include "p3dMainObject.h"
#include "p3dPythonObject.h"
#include "p3dInstance.h"
#include "p3dSession.h"
#include "p3dStringObject.h"
#include "p3dInstanceManager.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DMainObject::
P3DMainObject() :
  _pyobj(NULL),
  _inst(NULL),
  _unauth_play(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DMainObject::
~P3DMainObject() {
  set_pyobj(NULL);

  // Clear the local properties.
  Properties::const_iterator pi;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    P3D_object *value = (*pi).second;
    P3D_OBJECT_DECREF(value);
  }
  _properties.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::get_type
//       Access: Public, Virtual
//  Description: Returns the fundamental type of this kind of object.
////////////////////////////////////////////////////////////////////
P3D_object_type P3DMainObject::
get_type() {
  return P3D_OT_object;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::get_bool
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a boolean, if
//               possible.
////////////////////////////////////////////////////////////////////
bool P3DMainObject::
get_bool() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::get_int
//       Access: Public, Virtual
//  Description: Returns the object value coerced to an integer, if
//               possible.
////////////////////////////////////////////////////////////////////
int P3DMainObject::
get_int() {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::get_float
//       Access: Public, Virtual
//  Description: Returns the object value coerced to a floating-point
//               value, if possible.
////////////////////////////////////////////////////////////////////
double P3DMainObject::
get_float() {
  return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::make_string
//       Access: Public, Virtual
//  Description: Fills the indicated C++ string object with the value
//               of this object coerced to a string.
////////////////////////////////////////////////////////////////////
void P3DMainObject::
make_string(string &value) {
  if (_pyobj == NULL) {
    value = "P3DMainObject";
  } else {
    int size = P3D_OBJECT_GET_STRING(_pyobj, NULL, 0);
    char *buffer = new char[size];
    P3D_OBJECT_GET_STRING(_pyobj, buffer, size);
    value = string(buffer, size);
    delete[] buffer;
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::get_property
//       Access: Public, Virtual
//  Description: Returns the named property element in the object.  The
//               return value is a new-reference P3D_object, or NULL
//               on error.
////////////////////////////////////////////////////////////////////
P3D_object *P3DMainObject::
get_property(const string &property) {
  if (_pyobj == NULL) {
    // Without a pyobj, we just report whatever's been stored locally.
    Properties::const_iterator pi;
    pi = _properties.find(property);
    if (pi != _properties.end()) {
      P3D_object *result = (*pi).second;
      P3D_OBJECT_INCREF(result);
      return result;
    }
    return NULL;
  }

  // With a pyobj, we pass the query down to it.
  return P3D_OBJECT_GET_PROPERTY(_pyobj, property.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::set_property
//       Access: Public, Virtual
//  Description: Modifies (or deletes, if value is NULL) the named
//               property element in the object.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DMainObject::
set_property(const string &property, bool needs_response, P3D_object *value) {
  // First, we set the property locally.
  if (value != NULL) {
    Properties::iterator pi;
    pi = _properties.insert(Properties::value_type(property, (P3D_object *)NULL)).first;
    assert(pi != _properties.end());
    P3D_object *orig_value = (*pi).second;
    if (orig_value != value) {
      P3D_OBJECT_XDECREF(orig_value);
      (*pi).second = value;
      P3D_OBJECT_INCREF(value);
    }
  } else {
    // (Or delete the property locally.)
    Properties::iterator pi;
    pi = _properties.find(property);
    if (pi != _properties.end()) {
      P3D_object *orig_value = (*pi).second;
      P3D_OBJECT_DECREF(orig_value);
      _properties.erase(pi);
    }
  }

  if (_pyobj == NULL) {
    // Without a pyobj, that's all we do.
    return true;
  }

  // With a pyobj, we also pass this request down.
  return P3D_OBJECT_SET_PROPERTY(_pyobj, property.c_str(), needs_response, value);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::has_method
//       Access: Public, Virtual
//  Description: Returns true if the named method exists on this
//               object, false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DMainObject::
has_method(const string &method_name) {
  // Some special-case methods implemented in-place.
  if (method_name == "play") {
    return true;
  } else if (method_name == "read_game_log") {
    return true;
  } else if (method_name == "read_system_log") {
    return true;
  } else if (method_name == "read_log") {
    return true;
  } else if (method_name == "uninstall") {
    return true;
  }

  if (_pyobj == NULL) {
    // No methods until we get our pyobj.
    return false;
  }

  return P3D_OBJECT_HAS_METHOD(_pyobj, method_name.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::call
//       Access: Public, Virtual
//  Description: Invokes the named method on the object, passing the
//               indicated parameters.  If the method name is empty,
//               invokes the object itself.
//
//               If needs_response is true, the return value is a
//               new-reference P3D_object on success, or NULL on
//               failure.  If needs_response is false, the return
//               value is always NULL, and there is no way to
//               determine success or failure.
////////////////////////////////////////////////////////////////////
P3D_object *P3DMainObject::
call(const string &method_name, bool needs_response,
     P3D_object *params[], int num_params) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  nout << "main." << method_name << "(";
  for (int i = 0; i < num_params; ++i) {
    if (i != 0) {
      nout << ", ";
    }
    int buffer_size = P3D_OBJECT_GET_REPR(params[i], NULL, 0);
    char *buffer = new char[buffer_size];
    P3D_OBJECT_GET_REPR(params[i], buffer, buffer_size);
    nout.write(buffer, buffer_size);
    delete[] buffer;
  }
  nout << ")\n";

  if (method_name == "play") {
    return call_play(params, num_params);
  } else if (method_name == "read_game_log") {
    return call_read_game_log(params, num_params);
  } else if (method_name == "read_system_log") {
    return call_read_system_log(params, num_params);
  } else if (method_name == "read_log") {
    return call_read_log(params, num_params);
  } else if (method_name == "uninstall") {
    return call_uninstall(params, num_params);
  }

  if (_pyobj == NULL) {
    // No methods until we get our pyobj.
    return NULL;
  }

  return P3D_OBJECT_CALL(_pyobj, method_name.c_str(), needs_response,
                         params, num_params);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::output
//       Access: Public, Virtual
//  Description: Writes a formatted representation of the value to the
//               indicated string.  This is intended for developer
//               assistance.
////////////////////////////////////////////////////////////////////
void P3DMainObject::
output(ostream &out) {
  out << "P3DMainObject";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::set_pyobj
//       Access: Public
//  Description: Changes the internal pyobj pointer.  This is the
//               P3D_object that references the actual PyObject held
//               within the child process, corresponding to the true
//               main object there.  The new object's reference
//               count is incremented, and the previous object's is
//               decremented.
////////////////////////////////////////////////////////////////////
void P3DMainObject::
set_pyobj(P3D_object *pyobj) {
  if (pyobj == this) {
    // We are setting a reference directly to ourselves.  This happens
    // when the application has accepted the main object we gave it in
    // set_instance_info().  This means the application is directly
    // manipulating this object as its appRunner.main.  In this case,
    // we don't actually need to set the reference; instead, we clear
    // anything we had set.
    nout << "application shares main object\n";
    pyobj = NULL;

  } else if (pyobj != NULL) {
    // In the alternate case, the application has its own, separate
    // appRunner.main object.  Thus, we do need to set the pointer.
    nout << "application has its own main object\n";
  }
    
  if (_pyobj != pyobj) {
    P3D_OBJECT_XDECREF(_pyobj);
    _pyobj = pyobj;
    if (_pyobj != NULL) {
      P3D_OBJECT_INCREF(_pyobj);

      // Now that we have a pyobj, we have to transfer down all of the
      // properties we'd set locally.
      apply_properties(_pyobj);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::get_pyobj
//       Access: Public
//  Description: Returns the internal pyobj pointer, or NULL if it has
//               not yet been set.
////////////////////////////////////////////////////////////////////
P3D_object *P3DMainObject::
get_pyobj() const {
  return _pyobj;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::apply_properties
//       Access: Public
//  Description: Applies the locally-set properties onto the indicated
//               Python object, but does not store the object.  This
//               is a one-time copy of the locally-set properties
//               (like "coreapiHostUrl" and the like) onto the
//               indicated Python object.
////////////////////////////////////////////////////////////////////
void P3DMainObject::
apply_properties(P3D_object *pyobj) {
  P3DPythonObject *p3dpyobj = NULL;
  if (pyobj->_class == &P3DObject::_object_class) {
    p3dpyobj = ((P3DObject *)pyobj)->as_python_object();
  }

  Properties::const_iterator pi;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    const string &property_name = (*pi).first;
    P3D_object *value = (*pi).second;
    if (p3dpyobj != NULL && P3D_OBJECT_GET_TYPE(value) != P3D_OT_object) {
      // If we know we have an actual P3DPythonObject (we really
      // expect this), then we can call set_property_insecure()
      // directly, because we want to allow setting the initial
      // properties even if Javascript has no permissions to write
      // into Python.  But we don't allow setting objects this way in
      // any event.
      p3dpyobj->set_property_insecure(property_name, false, value);
    } else {
      // Otherwise, we go through the generic interface.
      P3D_OBJECT_SET_PROPERTY(pyobj, property_name.c_str(), false, value);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::set_instance
//       Access: Public
//  Description: Sets a callback pointer to the instance that owns
//               this object.  When this instance destructs, it clears
//               this pointer to NULL.
////////////////////////////////////////////////////////////////////
void P3DMainObject::
set_instance(P3DInstance *inst) {
  if (_inst != NULL) {
    // Save the game log filename of the instance just before it goes
    // away, in case JavaScript asks for it later.
    _game_log_pathname = _inst->get_log_pathname();
  }

  _inst = inst;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::call_play
//       Access: Private
//  Description: Starts the process remotely, as if the play button
//               had been clicked.  If the application has not yet
//               been validated, this pops up the validation dialog.
//
//               Only applicable if the application was in the ready
//               state, or the unauth state.  Returns true if the
//               application is now started, false otherwise.
//
//               This may be invoked from the unauth state only once.
//               If the user chooses not to authorize the plugin at
//               that time, it may not be invoked automatically again.
////////////////////////////////////////////////////////////////////
P3D_object *P3DMainObject::
call_play(P3D_object *params[], int num_params) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (_inst == NULL) {
    return inst_mgr->new_bool_object(false);
  }

  // I guess there's no harm in allowing JavaScript to call play(),
  // with or without explicit scripting authorization.
  nout << "play() called from JavaScript\n";

  if (!_inst->is_trusted()) {
    // Requires authorization.  We allow this only once; beyond that,
    // and you're only annoying the user.
    if (!_unauth_play) {
      _unauth_play = true;
      _inst->splash_button_clicked_main_thread();
    }

  } else if (!_inst->is_started()) {
    // We allow calling play() from a ready state without limit, but
    // probably only once will be necessary.
    _inst->splash_button_clicked_main_thread();
  }
   
  return inst_mgr->new_bool_object(_inst->is_started());
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::call_read_game_log
//       Access: Private
//  Description: Reads the entire logfile as a string, and returns it
//               to the calling JavaScript process.
////////////////////////////////////////////////////////////////////
P3D_object *P3DMainObject::
call_read_game_log(P3D_object *params[], int num_params) {
  if (_inst != NULL) {
    string log_pathname = _inst->get_log_pathname();
    return read_log(log_pathname, params, num_params);
  }

  if (!_game_log_pathname.empty()) {
    // The instance has already finished, but we saved its log
    // filename.
    return read_log(_game_log_pathname, params, num_params);
  }

  // No log available for us.
  nout << "read_game_log: error: game log name unknown" << "\n";
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  return inst_mgr->new_undefined_object();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::call_read_system_log
//       Access: Private
//  Description: As above, but reads the system log, the logfile for
//               the installation process.
////////////////////////////////////////////////////////////////////
P3D_object *P3DMainObject::
call_read_system_log(P3D_object *params[], int num_params) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  string log_pathname = inst_mgr->get_log_pathname();
  return read_log(log_pathname, params, num_params);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::call_read_log
//       Access: Private
//  Description: Reads a named logfile.  The filename must end
//               in ".log" and must not contain any slashes or colons
//               (it must be found within the log directory).
////////////////////////////////////////////////////////////////////
P3D_object *P3DMainObject::
call_read_log(P3D_object *params[], int num_params) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  if (num_params < 1) {
    nout << "read_log: error: not enough parameters" << "\n";
    return inst_mgr->new_undefined_object();
  }

  int size = P3D_OBJECT_GET_STRING(params[0], NULL, 0);
  char *buffer = new char[size];
  P3D_OBJECT_GET_STRING(params[0], buffer, size);
  string log_filename = string(buffer, size);
  delete[] buffer;

  if (log_filename.size() < 4 || log_filename.substr(log_filename.size() - 4) != string(".log")) {
    // Wrong filename extension.
    nout << "read_log: error: invalid filename" << "\n";
    return inst_mgr->new_undefined_object();
  }

  size_t slash = log_filename.find('/');
  if (slash != string::npos) {
    // No slashes allowed.
    nout << "read_log: error: invalid filename" << "\n";
    return inst_mgr->new_undefined_object();
  }

  slash = log_filename.find('\\');
  if (slash != string::npos) {
    // Nor backslashes.
    nout << "read_log: error: invalid filename" << "\n";
    return inst_mgr->new_undefined_object();
  }

  size_t colon = log_filename.find(':');
  if (colon != string::npos) {
    // Nor colons, for that matter.
    nout << "read_log: error: invalid filename" << "\n";
    return inst_mgr->new_undefined_object();
  }

  string log_pathname = inst_mgr->get_log_directory() + log_filename;
  P3D_object *result = read_log(log_pathname, params + 1, num_params - 1);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::read_log
//       Access: Private
//  Description: log-reader meta function that handles reading
//               previous log files in addition to the present one
////////////////////////////////////////////////////////////////////
P3D_object *P3DMainObject::
read_log(const string &log_pathname, P3D_object *params[], int num_params) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  string log_directory = inst_mgr->get_log_directory();
  ostringstream log_data;

  // Check the first parameter, if any--if given, it specifies the
  // last n bytes to retrieve.
  size_t tail_bytes = 0;
  if (num_params > 0) {
    tail_bytes = (size_t)max(P3D_OBJECT_GET_INT(params[0]), 0);
  }
  // Check the second parameter, if any--if given, it specifies the
  // first n bytes to retrieve.
  size_t head_bytes = 0;
  if (num_params > 1) {
    head_bytes = (size_t)max(P3D_OBJECT_GET_INT(params[1]), 0);
  }
  // Check the third parameter, if any--if given, it specifies the
  // last n bytes to retrieve from previous copies of this file.
  size_t tail_bytes_prev = 0;
  if (num_params > 2) {
    tail_bytes_prev = (size_t)max(P3D_OBJECT_GET_INT(params[2]), 0);
  }
  // Check the fourth parameter, if any--if given, it specifies the
  // first n bytes to retrieve from previous copies of this file.
  size_t head_bytes_prev = 0;
  if (num_params > 3) {
    head_bytes_prev = (size_t)max(P3D_OBJECT_GET_INT(params[3]), 0);
  }

  // Determine the base of the log file names
  nout << "log_pathname: " << log_pathname << "\n";
  string log_basename = log_pathname;
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
  string log_leafname_primary = log_basename;
  int dash = log_basename.rfind("-");
  if (dash != string::npos) {
    log_basename = log_basename.substr(0, dash+1);
  } else {
    int dotLog = log_basename.rfind(".log");
    if (dotLog != string::npos) {
      log_basename = log_basename.substr(0, dotLog);
      log_basename += "-";
    }
  }

  // Read matching files
  vector<string> all_logs;
  int log_matches_found = 0;
  string log_matching_pathname;
  inst_mgr->scan_directory(log_directory, all_logs);
  for (int i = (int)all_logs.size() - 1; i >= 0; --i) {
    if (all_logs[i] == log_leafname_primary ||
        (all_logs[i].find(log_basename) == 0 &&
         all_logs[i].size() > 4 &&
         all_logs[i].substr(all_logs[i].size() - 4) == string(".log"))) {
      log_matches_found++;
      log_matching_pathname = (log_directory + all_logs[i]);
      read_log_file(log_matching_pathname, tail_bytes, head_bytes, log_data);
      tail_bytes = tail_bytes_prev;
      head_bytes = head_bytes_prev;
    }
  }

  if (log_matches_found == 0) {
    nout << "read_log: warning: no matching file(s) on disk." << "\n";
  }

  string log_data_str = log_data.str();
  P3D_object *result = new P3DStringObject(log_data_str);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::read_log_file
//       Access: Private
//  Description: The generic log-reader function.
////////////////////////////////////////////////////////////////////
void P3DMainObject::
read_log_file(const string &log_pathname,
              size_t tail_bytes, size_t head_bytes,
              ostringstream &log_data) {

  // Get leaf name
  string log_leafname = log_pathname;
  size_t slash = log_leafname.rfind('/');
  if (slash != string::npos) {
    log_leafname = log_leafname.substr(slash + 1);
  }
#ifdef _WIN32
  slash = log_leafname.rfind('\\');
  if (slash != string::npos) {
    log_leafname = log_leafname.substr(slash + 1);
  }
#endif  // _WIN32

  // Render log file header to log_data
  log_data << "=======================================";
  log_data << "=======================================" << "\n";
  log_data << "== PandaLog-" << log_pathname << "\n";

  // load file
  ifstream log(log_pathname.c_str(), ios::in);
  if (!log) {
    log_data << "== PandaLog-" << "Error opening file";
    log_data << " " << "(" << log_leafname << ")" << "\n";
    return;
  }

  // Get the size of the file.
  log.seekg(0, ios::end);
  size_t file_size = (size_t)log.tellg();
  nout << "read_log: " << log_pathname << " is " << file_size
       << " bytes, tail_bytes = " << tail_bytes << ", head_bytes = "
       << head_bytes << "\n";

  // Early out if the file is empty
  if (file_size == (size_t)0) {
    log_data << "== PandaLog-" << "Empty File";
    log_data << " " << "(" << log_leafname << ")" << "\n";
    return;
  }

  // Check if we are getting the full file
  size_t full_bytes = 0;
  if (file_size <= head_bytes + tail_bytes) {
    // We will return the entire log.
    full_bytes = file_size;
    head_bytes = 0;
    tail_bytes = 0;
  }

  // Allocate a temp buffer to hold file data
  size_t buffer_bytes = max(max(full_bytes, head_bytes), tail_bytes) + 1;
  nout << "allocating " << buffer_bytes << " bytes to read at a time from file of size " << file_size << ".\n";
  char *buffer = new char[buffer_bytes];
  if (buffer == NULL) {
    log_data << "== PandaLog-" << "Error allocating buffer";
    log_data << " " << "(" << log_leafname << ")" << "\n";
    return;
  }

  // Render log data if full file is to be fetched
  if (full_bytes > 0) {
    log.seekg(0, ios::beg);
    log.read(buffer, full_bytes);
    streamsize read_bytes = log.gcount();
    assert(read_bytes < (streamsize)buffer_bytes);
    buffer[read_bytes] = '\0';
    log_data << "== PandaLog-" << "Full Start";
    log_data << " " << "(" << log_leafname << ")" << "\n";
    log_data << buffer;
    log_data << "== PandaLog-" << "Full End";
    log_data << " " << "(" << log_leafname << ")" << "\n";
  }

  // Render log data if head bytes are to be fetched
  if (head_bytes > 0) {
    log.seekg(0, ios::beg);
    log.read(buffer, head_bytes);
    streamsize read_bytes = log.gcount();
    assert(read_bytes < (streamsize)buffer_bytes);
    buffer[read_bytes] = '\0';
    log_data << "== PandaLog-" << "Head Start";
    log_data << " " << "(" << log_leafname << ")" << "\n";
    log_data << buffer << "\n";
    log_data << "== PandaLog-" << "Head End";
    log_data << " " << "(" << log_leafname << ")" << "\n";
  }

  // Render separator if head & tail bytes are to be fetched
  if ((head_bytes > 0) && (tail_bytes > 0)) {
    log_data << "== PandaLog-" << "truncated";
    log_data << " " << "(" << log_leafname << ")" << "\n";
  }

  // Render log data if tail bytes are to be fetched
  if (tail_bytes > 0) {
    log.seekg(file_size - tail_bytes, ios::beg);
    log.read(buffer, tail_bytes);
    streamsize read_bytes = log.gcount();
    assert(read_bytes < (streamsize)buffer_bytes);
    buffer[read_bytes] = '\0';
    log_data << "== PandaLog-" << "Tail Start";
    log_data << " " << "(" << log_leafname << ")" << "\n";
    log_data << buffer;
    log_data << "== PandaLog-" << "Tail End";
    log_data << " " << "(" << log_leafname << ")" << "\n";
  }

  // Render log file footer to log_data
  //log_data << "=======================================";
  //log_data << "=======================================" << "\n";

  // cleanup
  delete[] buffer;
  buffer = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMainObject::call_uninstall
//       Access: Private
//  Description: Implements the uninstall() plugin method, which
//               removes all Panda installed files for a particular
//               host, or referenced by a particular p3d file.
////////////////////////////////////////////////////////////////////
P3D_object *P3DMainObject::
call_uninstall(P3D_object *params[], int num_params) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  // Get the first parameter, the uninstall mode.
  string mode;
  if (num_params > 0) {
    int size = P3D_OBJECT_GET_STRING(params[0], NULL, 0);
    char *buffer = new char[size];
    P3D_OBJECT_GET_STRING(params[0], buffer, size);
    mode = string(buffer, size);
    delete[] buffer;
  }

  if (mode == "all") {
    nout << "uninstall all\n";
    inst_mgr->uninstall_all();
    return inst_mgr->new_bool_object(true);
  }

  if (_inst != NULL) {
    nout << "uninstall " << mode << " for " << _inst << "\n";
    bool success = false;
    if (mode == "host") {
      success = _inst->uninstall_host();
    } else {
      success = _inst->uninstall_packages();
    }
    return inst_mgr->new_bool_object(success);
  }

  nout << "couldn't uninstall; no instance.\n";
  return inst_mgr->new_bool_object(false);
}
