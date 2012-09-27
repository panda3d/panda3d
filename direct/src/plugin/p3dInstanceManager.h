// Filename: p3dInstanceManager.h
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

#ifndef P3DINSTANCEMANAGER_H
#define P3DINSTANCEMANAGER_H

#include "p3d_plugin_common.h"
#include "p3dConditionVar.h"

#include <set>
#include <map>
#include <vector>

#ifndef _WIN32
#include <signal.h>
#endif

#define OPENSSL_NO_KRB5
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/md5.h"

class P3DInstance;
class P3DSession;
class P3DAuthSession;
class P3DHost;
class P3DPackage;
class FileSpec;
class TiXmlElement;

////////////////////////////////////////////////////////////////////
//       Class : P3DInstanceManager
// Description : This global class manages the set of instances in the
//               universe.
////////////////////////////////////////////////////////////////////
class P3DInstanceManager {
private:
  P3DInstanceManager();
  ~P3DInstanceManager();

public:
  bool initialize(int api_version, const string &contents_filename,
                  const string &host_url,
                  P3D_verify_contents verify_contents,
                  const string &platform,
                  const string &log_directory,
                  const string &log_basename,
                  bool trusted_environment,
                  bool console_environment,
                  const string &root_dir = "",
                  const string &host_dir = "");

  inline bool is_initialized() const;
  inline void reconsider_runtime_environment();
  inline P3D_verify_contents get_verify_contents() const;
  inline void reset_verify_contents();

  inline int get_api_version() const;
  inline const string &get_host_url() const;
  inline const string &get_root_dir() const;
  inline const string &get_platform() const;
  inline const string &get_temp_directory() const;
  inline const string &get_log_directory() const;
  inline const string &get_log_pathname() const;
  inline bool get_trusted_environment() const;
  inline bool get_console_environment() const;

  inline int get_num_supported_platforms() const;
  inline const string &get_supported_platform(int n) const;

  void set_plugin_version(int major, int minor, int sequence,
                          bool official, const string &distributor,
                          const string &coreapi_host_url,
                          time_t coreapi_timestamp,
                          const string &coreapi_set_ver);
  inline int get_plugin_major_version() const;
  inline int get_plugin_minor_version() const;
  inline int get_plugin_sequence_version() const;
  inline bool get_plugin_official_version() const;
  inline const string &get_plugin_distributor() const;
  inline const string &get_coreapi_host_url() const;
  inline time_t get_coreapi_timestamp() const;
  inline const string &get_coreapi_set_ver() const;

  void set_super_mirror(const string &super_mirror_url);
  inline const string &get_super_mirror() const;

  P3DInstance *
  create_instance(P3D_request_ready_func *func, 
                  const P3D_token tokens[], size_t num_tokens, 
                  int argc, const char *argv[], void *user_data);

  bool set_p3d_filename(P3DInstance *inst, bool is_local,
                        const string &p3d_filename, const int &p3d_offset);
  int make_p3d_stream(P3DInstance *inst, const string &p3d_url);
  bool start_instance(P3DInstance *inst);
  void finish_instance(P3DInstance *inst);
  P3DAuthSession *authorize_instance(P3DInstance *inst);

  P3DInstance *validate_instance(P3D_instance *instance);

  P3DInstance *check_request();
  void wait_request(double timeout);

  P3DHost *get_host(const string &host_url);
  void forget_host(P3DHost *host);

  inline int get_num_instances() const;

  int get_unique_id();
  void signal_request_ready(P3DInstance *inst);

  P3D_class_definition *make_class_definition() const;

  inline P3D_object *new_undefined_object();
  inline P3D_object *new_none_object();
  inline P3D_object *new_bool_object(bool value);

  string make_temp_filename(const string &extension);
  void release_temp_filename(const string &filename);

  bool find_cert(X509 *cert);
  void read_certlist(P3DPackage *package);
  string get_cert_dir(X509 *cert);
  static string cert_to_der(X509 *cert);

  void uninstall_all();
  
  static P3DInstanceManager *get_global_ptr();
  static void delete_global_ptr();

  static inline char encode_hexdigit(int c);
  static bool scan_directory(const string &dirname, vector<string> &contents);
  static bool scan_directory_recursively(const string &dirname, 
                                         vector<string> &filename_contents,
                                         vector<string> &dirname_contents,
                                         const string &prefix = "");
  static void delete_directory_recursively(const string &root_dir);
  static bool remove_file_from_list(vector<string> &contents, const string &filename);

  static void append_safe_dir(string &root, const string &basename);

private:
  void create_runtime_environment();
  static void append_safe_dir_component(string &root, const string &component);

private:
  // The notify thread.  This thread runs only for the purpose of
  // generating asynchronous notifications of requests, to callers who
  // ask for it.
  THREAD_CALLBACK_DECLARATION(P3DInstanceManager, nt_thread_run);
  void nt_thread_run();

#ifdef _WIN32
  static bool supports_win64();
#endif  // _WIN32

private:
  bool _is_initialized;
  bool _created_runtime_environment;
  int _api_version;
  string _host_url;
  string _root_dir;
  string _host_dir;
  string _certs_dir;
  P3D_verify_contents _verify_contents;
  string _platform;
  string _log_directory;
  string _log_basename;
  string _log_pathname;
  string _temp_directory;
  bool _trusted_environment;
  bool _console_environment;
  int _plugin_major_version;
  int _plugin_minor_version;
  int _plugin_sequence_version;
  bool _plugin_official_version;
  string _plugin_distributor;
  string _coreapi_host_url;
  time_t _coreapi_timestamp;
  string _coreapi_set_ver;
  string _super_mirror_url;

  typedef vector<string> SupportedPlatforms;
  SupportedPlatforms _supported_platforms;

  P3D_object *_undefined_object;
  P3D_object *_none_object;
  P3D_object *_true_object;
  P3D_object *_false_object;

  typedef set<string> ApprovedCerts;
  ApprovedCerts _approved_certs;  

  typedef set<P3DInstance *> Instances;
  Instances _instances;
  P3DAuthSession *_auth_session;

  typedef map<string, P3DSession *> Sessions;
  Sessions _sessions;

  typedef map<string, P3DHost *> Hosts;
  Hosts _hosts;

  typedef set<string> TempFilenames;
  TempFilenames _temp_filenames;
  int _next_temp_filename_counter;

  int _unique_id;

  // This condition var is waited on the main thread and signaled in a
  // sub-thread when new request notices arrive.
  P3DConditionVar _request_ready;

  // We may need a thread to send async request notices to callers.
  bool _notify_thread_continue;
  bool _started_notify_thread;
  THREAD _notify_thread;
  // This queue of instances that need to send notifications is
  // protected by _notify_ready's mutex.
  typedef vector<P3DInstance *> NotifyInstances;
  NotifyInstances _notify_instances;
  P3DConditionVar _notify_ready;

#ifndef _WIN32
  struct sigaction _old_sigpipe;
#endif

  static P3DInstanceManager *_global_ptr;
};

#include "p3dInstanceManager.I"

#endif
