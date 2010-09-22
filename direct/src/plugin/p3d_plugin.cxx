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
#include "p3d_plugin_config.h"
#include "p3dInstanceManager.h"
#include "p3dInstance.h"
#include "p3dWindowParams.h"
#include "p3dUndefinedObject.h"
#include "p3dNoneObject.h"
#include "p3dBoolObject.h"
#include "p3dIntObject.h"
#include "p3dFloatObject.h"
#include "p3dStringObject.h"

#include <assert.h>
#include <math.h>

// Use a simple lock to protect the C-style API functions in this
// module from parallel access by multiple threads in the host.

bool initialized_lock = false;
LOCK _api_lock;

bool 
P3D_initialize(int api_version, const char *contents_filename,
               const char *host_url, P3D_verify_contents verify_contents,
               const char *platform, const char *log_directory,
               const char *log_basename, bool trusted_environment,
               bool console_environment,
               const char *root_dir, const char *host_dir) {
  if (api_version < 10 || api_version > P3D_API_VERSION) {
    // Can't accept an incompatible version.
    return false;
  }

  if (api_version < 13) {
    // Prior to version 13, verify_contents was a bool.  Convert
    // "true" to P3D_VC_normal and "false" to P3D_VC_none.
    if ((int)verify_contents != 0) {
      verify_contents = P3D_VC_normal;
    } else {
      verify_contents = P3D_VC_none;
    }
  }

  if (!initialized_lock) {
    INIT_LOCK(_api_lock);
    initialized_lock = true;
  }
  ACQUIRE_LOCK(_api_lock);

  if (contents_filename == NULL){ 
    contents_filename = "";
  }

  if (host_url == NULL){ 
    host_url = "";
  }

  if (platform == NULL) {
    platform = "";
  }

  if (log_directory == NULL) {
    log_directory = "";
  }

  if (log_basename == NULL) {
    log_basename = "";
  }

  if (api_version < 12 || root_dir == NULL) {
    root_dir = "";
  }

  if (api_version < 16 || host_dir == NULL) {
    host_dir = "";
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  bool result = inst_mgr->initialize(api_version, contents_filename, host_url,
                                     verify_contents, platform,
                                     log_directory, log_basename,
                                     trusted_environment, console_environment,
                                     root_dir, host_dir);
  RELEASE_LOCK(_api_lock);
  return result;
}

void 
P3D_finalize() {
  nout << "P3D_finalize called\n";
  P3DInstanceManager::delete_global_ptr();
}

void
P3D_set_plugin_version(int major, int minor, int sequence,
                       bool official, const char *distributor,
                       const char *coreapi_host_url,
                       const char *coreapi_timestamp_str,
                       const char *coreapi_set_ver) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  if (distributor == NULL) {
    distributor = "";
  }
  if (coreapi_host_url == NULL) {
    coreapi_host_url = "";
  }

  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  time_t coreapi_timestamp = 0;
  if (inst_mgr->get_api_version() < 15) {
    // Before version 15, this was passed as a time_t.  
    coreapi_timestamp = (time_t)coreapi_timestamp_str;
  } else {
    // Passing a time_t causes problems with disagreements about word
    // size, so since version 15 we pass it as a string.
    coreapi_timestamp = strtoul(coreapi_timestamp_str, NULL, 10);
  }

  if (inst_mgr->get_api_version() < 14 || coreapi_set_ver == NULL) {
    // Prior to version 14 this parameter was absent.
    coreapi_set_ver = "";
  }

  inst_mgr->set_plugin_version(major, minor, sequence, official, distributor,
                               coreapi_host_url, coreapi_timestamp,
                               coreapi_set_ver);
  RELEASE_LOCK(_api_lock);
}

void
P3D_set_super_mirror(const char *super_mirror_url) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  if (super_mirror_url == NULL) {
    super_mirror_url = "";
  }

  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->set_super_mirror(super_mirror_url);
  RELEASE_LOCK(_api_lock);
}

P3D_instance *
P3D_new_instance(P3D_request_ready_func *func, 
                 const P3D_token tokens[], size_t num_tokens,
                 int argc, const char *argv[], void *user_data) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *result = inst_mgr->create_instance(func, tokens, num_tokens, 
                                                  argc, argv, user_data);
  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_instance_start(P3D_instance *instance, bool is_local, 
                   const char *p3d_filename, int p3d_offset) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  if (p3d_filename == NULL) {
    p3d_filename = "";
  }
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (inst_mgr->get_api_version() < 11) {
    // Prior to version 11, there was no p3d_offset parameter.  So, we
    // default it to 0.
    p3d_offset = 0;
  }

  P3DInstance *inst = inst_mgr->validate_instance(instance);
  bool result = false;
  if (inst != NULL) {
    // We don't actually start it immediately; the instance will have
    // to download the p3d url and read it, reading the python
    // version, before it can start.
    result = inst_mgr->set_p3d_filename(inst, is_local,
                                        p3d_filename, p3d_offset);
  }
  RELEASE_LOCK(_api_lock);
  return result;
}

int
P3D_instance_start_stream(P3D_instance *instance, const char *p3d_url) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  if (p3d_url == NULL) {
    p3d_url = "";
  }
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  int result = -1;
  if (inst != NULL) {
    result = inst_mgr->make_p3d_stream(inst, p3d_url);
  }
  RELEASE_LOCK(_api_lock);
  return result;
}

void
P3D_instance_finish(P3D_instance *instance) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  if (inst != NULL) {
    inst_mgr->finish_instance(inst);
  }
  RELEASE_LOCK(_api_lock);
}

void
P3D_instance_setup_window(P3D_instance *instance,
                          P3D_window_type window_type,
                          int win_x, int win_y,
                          int win_width, int win_height,
                          const P3D_window_handle *parent_window) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  P3DWindowParams wparams(window_type, win_x, win_y,
                          win_width, win_height, *parent_window);

  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  if (inst != NULL) {
    inst->set_wparams(wparams);
  }
  RELEASE_LOCK(_api_lock);
}

P3D_object_type
P3D_object_get_type(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3D_object_type result = P3D_OBJECT_GET_TYPE(object);
  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_object_get_bool(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  bool result = P3D_OBJECT_GET_BOOL(object);
  RELEASE_LOCK(_api_lock);
  return result;
}

int
P3D_object_get_int(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  int result = P3D_OBJECT_GET_INT(object);
  RELEASE_LOCK(_api_lock);
  return result;
}

double
P3D_object_get_float(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  double result = P3D_OBJECT_GET_FLOAT(object);
  RELEASE_LOCK(_api_lock);
  return result;
}

int
P3D_object_get_string(P3D_object *object, char *buffer, int buffer_size) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  int result = P3D_OBJECT_GET_STRING(object, buffer, buffer_size);
  RELEASE_LOCK(_api_lock);
  return result;
}

int
P3D_object_get_repr(P3D_object *object, char *buffer, int buffer_size) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  int result = P3D_OBJECT_GET_REPR(object, buffer, buffer_size);
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_object_get_property(P3D_object *object, const char *property) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3D_object *result = P3D_OBJECT_GET_PROPERTY(object, property);
  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_object_set_property(P3D_object *object, const char *property, 
                        bool needs_response, P3D_object *value) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  bool result = P3D_OBJECT_SET_PROPERTY(object, property, needs_response, value);
  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_object_has_method(P3D_object *object, const char *method_name) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  bool result = P3D_OBJECT_HAS_METHOD(object, method_name);
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_object_call(P3D_object *object, const char *method_name, 
                bool needs_response,
                P3D_object *params[], int num_params) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3D_object *result = P3D_OBJECT_CALL(object, method_name, needs_response,
                                       params, num_params);
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_object_eval(P3D_object *object, const char *expression) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3D_object *result = P3D_OBJECT_EVAL(object, expression);
  RELEASE_LOCK(_api_lock);
  return result;
}


void 
P3D_object_incref(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  if (object != NULL) {
    ACQUIRE_LOCK(_api_lock);
    P3D_OBJECT_INCREF(object);
    RELEASE_LOCK(_api_lock);
  }
}

void 
P3D_object_decref(P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  if (object != NULL) {
    ACQUIRE_LOCK(_api_lock);
    P3D_OBJECT_DECREF(object);
    RELEASE_LOCK(_api_lock);
  }
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
P3D_new_undefined_object() {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_object *result = inst_mgr->new_undefined_object();
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_none_object() {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_object *result = inst_mgr->new_none_object();
  
  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_object *
P3D_new_bool_object(bool value) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_object *result = inst_mgr->new_bool_object(value);
  
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
P3D_instance_get_panda_script_object(P3D_instance *instance) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  P3D_object *result = NULL;
  if (inst != NULL) {
    result = inst->get_panda_script_object();
  }
  
  RELEASE_LOCK(_api_lock);
  return result;
}

void
P3D_instance_set_browser_script_object(P3D_instance *instance, 
                                       P3D_object *object) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  if (inst != NULL) {
    inst->set_browser_script_object(object);
  }
  
  RELEASE_LOCK(_api_lock);
}


P3D_request *
P3D_instance_get_request(P3D_instance *instance) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  P3D_request *result = NULL;
  if (inst != NULL) {
    result = inst->get_request();
  }

  RELEASE_LOCK(_api_lock);
  return result;
}

P3D_instance *
P3D_check_request(double timeout) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3D_instance *inst = inst_mgr->check_request();

  if (inst != NULL || timeout <= 0.0) {
    RELEASE_LOCK(_api_lock);
    return inst;
  }

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

  // Now we have to block until a request is available.
  RELEASE_LOCK(_api_lock);
  inst_mgr->wait_request(timeout);
  ACQUIRE_LOCK(_api_lock);
  inst = inst_mgr->check_request();

  while (inst == NULL && inst_mgr->get_num_instances() != 0) {
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

    RELEASE_LOCK(_api_lock);
    inst_mgr->wait_request(timeout);
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
    P3DInstance::finish_request(request, handled);
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

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  bool result = false;
  if (inst != NULL) {
    result = inst->
      feed_url_stream(unique_id, result_code, http_status_code,
                      total_expected_data, 
                      (const unsigned char *)this_data, this_data_size);
  }

  RELEASE_LOCK(_api_lock);
  return result;
}

bool
P3D_instance_handle_event(P3D_instance *instance, 
                          const P3D_event_data *event) {
  assert(P3DInstanceManager::get_global_ptr()->is_initialized());
  ACQUIRE_LOCK(_api_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  P3DInstance *inst = inst_mgr->validate_instance(instance);
  bool result = false;
  if (inst != NULL) {
    result = inst->handle_event(*event);
  }

  RELEASE_LOCK(_api_lock);
  return result;
}
