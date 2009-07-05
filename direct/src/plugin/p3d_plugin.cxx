// Filename: p3d_plugin.cxx
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

#include "p3d_plugin_common.h"
#include "p3dInstanceManager.h"
#include "p3dInstance.h"
#include "p3dWindowParams.h"
#include "p3dNoneObject.h"
#include "p3dBoolObject.h"
#include "p3dIntObject.h"
#include "p3dFloatObject.h"
#include "p3dStringObject.h"

#include <assert.h>

// Use a simple lock to protect the C-style API functions in this
// module from parallel access by multiple threads in the host.

bool initialized_lock = false;
LOCK _api_lock;

ofstream log;
string plugin_output_filename;
ostream *nout_stream;


bool 
P3D_initialize(int api_version, const char *output_filename) {
  if (api_version != P3D_API_VERSION) {
    // Can't accept an incompatible version.
    return false;
  }

  if (!initialized_lock) {
    INIT_LOCK(_api_lock);
    initialized_lock = true;
  }
  ACQUIRE_LOCK(_api_lock);

  plugin_output_filename = string();
  if (output_filename != NULL) {
    plugin_output_filename = output_filename;
  }
  nout_stream = &cerr;
  if (!plugin_output_filename.empty()) {
    log.open(plugin_output_filename.c_str(), ios::out | ios::trunc);
    if (log) {
      nout_stream = &log;
    }
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  bool result = inst_mgr->initialize();
  RELEASE_LOCK(_api_lock);
  return result;
}

void 
P3D_finalize() {
  P3DInstanceManager::delete_global_ptr();
}

P3D_instance *
P3D_new_instance(P3D_request_ready_func *func, void *user_data) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *result = inst_mgr->create_instance(func, user_data);
  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_instance_start(P3D_instance *instance, const char *p3d_filename, 
                   const P3D_token tokens[], size_t num_tokens) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  if (p3d_filename == NULL) {
    p3d_filename = "";
  }
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  bool result = inst_mgr->start_instance
    ((P3DInstance *)instance, p3d_filename, tokens, num_tokens);
  RELEASE_LOCK(_api_lock);
  return result;
}

void
P3D_instance_finish(P3D_instance *instance) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->finish_instance((P3DInstance *)instance);
  RELEASE_LOCK(_api_lock);
}

void
P3D_instance_setup_window(P3D_instance *instance,
                          P3D_window_type window_type,
                          int win_x, int win_y,
                          int win_width, int win_height,
                          P3D_window_handle parent_window) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  P3DWindowParams wparams(window_type, win_x, win_y,
                          win_width, win_height, parent_window);

  ACQUIRE_LOCK(_api_lock);
  ((P3DInstance *)instance)->set_wparams(wparams);
  RELEASE_LOCK(_api_lock);
}

P3D_class_definition *
P3D_make_class_definition() {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_class_definition *result = inst_mgr->make_class_definition();
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_none_object() {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3D_object *result = new P3DNoneObject();
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_bool_object(bool value) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3D_object *result = new P3DBoolObject(value);
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_int_object(int value) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3D_object *result = new P3DIntObject(value);
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_float_object(double value) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3D_object *result = new P3DFloatObject(value);
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_string_object(const char *str, int length) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3D_object *result = new P3DStringObject(string(str, length));
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_instance_get_script_object(P3D_instance *instance) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3D_object *result = ((P3DInstance *)instance)->get_script_object();
  
  RELEASE_LOCK(_api_lock);
  return result;
}

void
P3D_instance_set_script_object(P3D_instance *instance, 
                               P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  // TODO.
  
  RELEASE_LOCK(_api_lock);
}


P3D_request *
P3D_instance_get_request(P3D_instance *instance) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3D_request *result = ((P3DInstance *)instance)->get_request();
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_instance *
P3D_check_request(bool wait) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_instance *inst = inst_mgr->check_request();

  if (inst != NULL || !wait) {
    RELEASE_LOCK(_api_lock);
    return inst;
  }
  
  // Now we have to block until a request is available.
  while (inst == NULL && inst_mgr->get_num_instances() != 0) {
    RELEASE_LOCK(_api_lock);
    inst_mgr->wait_request();
    ACQUIRE_LOCK(_api_lock);
    inst = inst_mgr->check_request();
  }

  RELEASE_LOCK(_api_lock);
  return inst;
}

void
P3D_request_finish(P3D_request *request, bool handled) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  if (request != (P3D_request *)NULL) {
    ((P3DInstance *)request->_instance)->finish_request(request, handled);
  }
  RELEASE_LOCK(_api_lock);
}

bool
P3D_instance_feed_url_stream(P3D_instance *instance, int unique_id,
                             P3D_result_code result_code,
                             int http_status_code, 
                             size_t total_expected_data,
                             const void *this_data, 
                             size_t this_data_size) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  bool result = ((P3DInstance *)instance)->
    feed_url_stream(unique_id, result_code, http_status_code,
                    total_expected_data, 
                    (const unsigned char *)this_data, this_data_size);
  RELEASE_LOCK(_api_lock);
  return result;
}
