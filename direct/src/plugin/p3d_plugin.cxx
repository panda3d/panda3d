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

// Use a simple lock to protect the C-style API functions in this
// module from parallel access by multiple threads in the host.

bool initialized_lock = false;
LOCK _lock;

bool 
P3D_initialize() {
  if (!initialized_lock) {
    INIT_LOCK(_lock);
    initialized_lock = true;
  }
  ACQUIRE_LOCK(_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  bool result = inst_mgr->initialize();
  RELEASE_LOCK(_lock);
  return result;
}

void 
P3D_free_string(char *string) {
  ACQUIRE_LOCK(_lock);
  delete [] string;
  RELEASE_LOCK(_lock);
}

P3D_instance *
P3D_create_instance(P3D_request_ready_func *func,
                    const char *p3d_filename, 
                    P3D_window_type window_type,
                    int win_x, int win_y,
                    int win_width, int win_height,
                    P3D_window_handle parent_window,
                    const P3D_token tokens[], size_t num_tokens) {
  ACQUIRE_LOCK(_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *result = 
    inst_mgr->create_instance(func, p3d_filename, window_type, 
                              win_x, win_y, win_width, win_height,
                              parent_window, tokens, num_tokens);
  RELEASE_LOCK(_lock);
  return result;
}

void
P3D_instance_finish(P3D_instance *instance) {
  ACQUIRE_LOCK(_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->finish_instance((P3DInstance *)instance);
  RELEASE_LOCK(_lock);
}

bool
P3D_instance_has_property(P3D_instance *instance,
                          const char *property_name) {
  ACQUIRE_LOCK(_lock);
  bool result = ((P3DInstance *)instance)->has_property(property_name);
  RELEASE_LOCK(_lock);
  return result;
}

char *
P3D_instance_get_property(P3D_instance *instance,
                          const char *property_name) {
  ACQUIRE_LOCK(_lock);
  string value = ((P3DInstance *)instance)->get_property(property_name);

  char *result = new char[value.length() + 1];
  RELEASE_LOCK(_lock);

  memcpy(result, value.data(), value.length());
  result[value.length()] = '\0';
  return result;
}

void 
P3D_instance_set_property(P3D_instance *instance,
                          const char *property_name,
                          const char *value) {
  ACQUIRE_LOCK(_lock);
  ((P3DInstance *)instance)->set_property(property_name, value);
  RELEASE_LOCK(_lock);
}

P3D_request *
P3D_instance_get_request(P3D_instance *instance) {
  ACQUIRE_LOCK(_lock);
  P3D_request *result = ((P3DInstance *)instance)->get_request();
  RELEASE_LOCK(_lock);
  return result;
}

P3D_instance *
P3D_check_request(bool wait) {
  ACQUIRE_LOCK(_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_instance *inst = inst_mgr->check_request();

  if (inst != NULL || !wait) {
    RELEASE_LOCK(_lock);
    return inst;
  }
  
  // Now we have to block until a request is available.
  while (inst == NULL && inst_mgr->get_num_instances() != 0) {
    RELEASE_LOCK(_lock);
    inst_mgr->wait_request();
    ACQUIRE_LOCK(_lock);
    inst = inst_mgr->check_request();
  }

  RELEASE_LOCK(_lock);
  return inst;
}

void
P3D_request_finish(P3D_request *request, bool handled) {
  ACQUIRE_LOCK(_lock);
  if (request != (P3D_request *)NULL) {
    ((P3DInstance *)request->_instance)->finish_request(request, handled);
  }
  RELEASE_LOCK(_lock);
}

void
P3D_instance_feed_url_stream(P3D_instance *instance, int unique_id,
                             P3D_result_code result_code,
                             int http_status_code, 
                             size_t total_expected_data,
                             const unsigned char *this_data, 
                             size_t this_data_size) {
  ACQUIRE_LOCK(_lock);
  ((P3DInstance *)instance)->
    feed_url_stream(unique_id, result_code, http_status_code,
                    total_expected_data, this_data, this_data_size);
  RELEASE_LOCK(_lock);
}

