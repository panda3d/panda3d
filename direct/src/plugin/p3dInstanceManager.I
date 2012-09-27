// Filename: p3dInstanceManager.I
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


////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::is_initialized
//       Access: Public
//  Description: Returns true if the instance manager is successfully
//               initialized, false otherwise.
////////////////////////////////////////////////////////////////////
inline bool P3DInstanceManager::
is_initialized() const {
  return _is_initialized;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::reconsider_runtime_environment
//       Access: Public
//  Description: Recreates the runtime environment if a previous call
//               to uninstall_all() removed it.  Does nothing if the
//               runtime environment is already correctly set up.
////////////////////////////////////////////////////////////////////
inline void P3DInstanceManager::
reconsider_runtime_environment() {
  assert(_is_initialized);
  if (!_created_runtime_environment) {
    create_runtime_environment();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::verify_contents
//       Access: Public
//  Description: Returns the verify_contents setting.  When this is
//               set to P3D_VC_none, it indicates that we don't need
//               to contact the server to verify that a contents.xml
//               file is fresh before using it; we should just use it
//               as it is.
////////////////////////////////////////////////////////////////////
inline P3D_verify_contents P3DInstanceManager::
get_verify_contents() const {
  return _verify_contents;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::reset_verify_contents
//       Access: Public
//  Description: Resets the verify_contents flag to P3D_VC_normal, if
//               it is P3D_VC_none.  This should be done whenever we
//               discover anything needs to be downloaded.  At this
//               point, we might as well verify everything.
////////////////////////////////////////////////////////////////////
inline void P3DInstanceManager::
reset_verify_contents() {
  if (_verify_contents == P3D_VC_none) {
    _verify_contents = P3D_VC_normal;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_api_version
//       Access: Public
//  Description: Returns the api_version number which was passed to
//               P3D_initialize().  Client code may use this to
//               determine how to interpret parameters to various
//               functions whose interface may have changed over
//               different versions.
////////////////////////////////////////////////////////////////////
inline int P3DInstanceManager::
get_api_version() const {
  return _api_version;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_host_url
//       Access: Public
//  Description: Returns the standard host_url which the instances
//               should attempt to contact to download auxiliary
//               packages associated with the core API, such as the
//               p3dcert and images packages.  This is normally the
//               compiled-in PANDA_PACKAGE_HOST_URL, but it might be
//               set to something different by the -u parameter on the
//               panda3d executable.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_host_url() const {
  return _host_url;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_root_dir
//       Access: Public
//  Description: Returns the root directory into which all the P3D
//               runtime files are downloaded and installed.  This
//               must be a writable directory or nothing will work.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_root_dir() const {
  return _root_dir;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_platform
//       Access: Public
//  Description: Returns the string that corresponds to the platform
//               on which we are running.  This string will be used to
//               determine the appropriate packages to download.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_platform() const {
  return _platform;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_temp_directory
//       Access: Public
//  Description: Returns the pathname of the directory into which 
//               temporary files should be written.  This filename
//               will end with a slash, so that full pathnames may be
//               made by concatenting directly with this string.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_temp_directory() const {
  return _temp_directory;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_log_directory
//       Access: Public
//  Description: Returns the pathname of the directory into which all
//               log files should be written.  This filename will end
//               with a slash, so that full pathnames may be made by
//               concatenting directly with this string.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_log_directory() const {
  return _log_directory;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_log_pathname
//       Access: Public
//  Description: Returns the filename of the system log file; this
//               file is responsible for downloading and installing
//               updates, and launching applications.  This is
//               different from the session log file(s), which
//               represent the output from a particular Python
//               session.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_log_pathname() const {
  return _log_pathname;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_trusted_environment
//       Access: Public
//  Description: Returns the value of the trusted_environment flag
//               passed to the constructor.  If this is true, it means
//               the environment we are running in is trusted and the
//               p3d file is already vetted.  This means the current
//               working directory will remain unchanged, and the p3d
//               file will be run without checking its signature.
//
//               This should generally be true only when run by
//               panda3d.exe or panda3dw.exe, and not when run by the
//               web plugin.
////////////////////////////////////////////////////////////////////
inline bool P3DInstanceManager::
get_trusted_environment() const {
  return _trusted_environment;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_console_environment
//       Access: Public
//  Description: Returns the value of the console_environment flag
//               passed to the constructor.  If this is true, it means
//               we are running from a text-based console window, and
//               not from a desktop environment.
//
//               This should generally be true only when run by
//               panda3d.exe, and not when run by the web plugin or by
//               panda3dw.exe.
////////////////////////////////////////////////////////////////////
inline bool P3DInstanceManager::
get_console_environment() const {
  return _console_environment;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_num_supported_platforms
//       Access: Public
//  Description: Returns the number of different supported platforms
//               available in get_supported_platform().
////////////////////////////////////////////////////////////////////
inline int P3DInstanceManager::
get_num_supported_platforms() const {
  return (int)_supported_platforms.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_supported_platform
//       Access: Public
//  Description: Returns the nth supported platform, where 0 <= n <
//               get_num_supported_platforms().
//
//               A given runtime environment may support multiple
//               different platforms, e.g. win32 or win64, with the
//               restriction that all platform-specific packages
//               (beginning from panda3d), must be the same platform.
//
//               This function enumerates the different platforms that
//               the current runtime environment will support, in
//               order of preference--preferred platforms appear first
//               in the list.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_supported_platform(int n) const {
  return _supported_platforms.at(n);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_plugin_major_version
//       Access: Public
//  Description: Returns the plugin's reported major version number.
////////////////////////////////////////////////////////////////////
inline int P3DInstanceManager::
get_plugin_major_version() const {
  return _plugin_major_version;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_plugin_minor_version
//       Access: Public
//  Description: Returns the plugin's reported minor version number.
////////////////////////////////////////////////////////////////////
inline int P3DInstanceManager::
get_plugin_minor_version() const {
  return _plugin_minor_version;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_plugin_sequence_version
//       Access: Public
//  Description: Returns the plugin's reported sequence version number.
////////////////////////////////////////////////////////////////////
inline int P3DInstanceManager::
get_plugin_sequence_version() const {
  return _plugin_sequence_version;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_plugin_official_version
//       Access: Public
//  Description: Returns true if the plugin claims to be from an
//               "official" build, and the its version number is
//               authoritative; or false if it makes no such claim
//               (for instance, it was built by someone checking out
//               from cvs).
////////////////////////////////////////////////////////////////////
inline bool P3DInstanceManager::
get_plugin_official_version() const {
  return _plugin_official_version;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_plugin_distributor
//       Access: Public
//  Description: Returns the "distributor" reported by the plugin.
//               This should represent the entity that built and
//               hosted the plugin.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_plugin_distributor() const {
  return _plugin_distributor;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_coreapi_host_url
//       Access: Public
//  Description: Returns the host URL from which this Core API was
//               downloaded (according to the plugin).  This is for
//               reporting purposes only; see get_host_url() for the
//               URL to contact to actually download content.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_coreapi_host_url() const {
  return _coreapi_host_url;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_coreapi_timestamp
//       Access: Public
//  Description: Returns the timestamp associated with this Core API
//               DLL (according to the plugin).  This is the timestamp
//               shown in the contents.xml for this host, and is
//               usually the time at which the plugin was built.
////////////////////////////////////////////////////////////////////
inline time_t P3DInstanceManager::
get_coreapi_timestamp() const {
  return _coreapi_timestamp;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_coreapi_set_ver
//       Access: Public
//  Description: Returns the version number associated with the Core
//               API, if provided.  Some early versions of the Core
//               API, and some early versions of the plugin, did not
//               provide a number here.  If provided, this will be a
//               string of dot-separated integers.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_coreapi_set_ver() const {
  return _coreapi_set_ver;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_super_mirror
//       Access: Public
//  Description: Returns the "super mirror" URL.  See p3d_plugin.h.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_super_mirror() const {
  return _super_mirror_url;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_num_instances
//       Access: Public
//  Description: Returns the number of instances currently running
//               within the world.
////////////////////////////////////////////////////////////////////
inline int P3DInstanceManager::
get_num_instances() const {
  return _instances.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::new_undefined_object
//       Access: Public
//  Description: Returns the singleton "undefined" object, as a new
//               reference.
////////////////////////////////////////////////////////////////////
inline P3D_object *P3DInstanceManager::
new_undefined_object() {
  P3D_OBJECT_INCREF(_undefined_object);
  return _undefined_object;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::new_none_object
//       Access: Public
//  Description: Returns the singleton "none" object, as a new
//               reference.
////////////////////////////////////////////////////////////////////
inline P3D_object *P3DInstanceManager::
new_none_object() {
  P3D_OBJECT_INCREF(_none_object);
  return _none_object;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::new_bool_object
//       Access: Public
//  Description: Returns the singleton "true" or "false" object, as a
//               new reference.
////////////////////////////////////////////////////////////////////
inline P3D_object *P3DInstanceManager::
new_bool_object(bool value) {
  P3D_object *obj = (value) ? _true_object : _false_object;
  P3D_OBJECT_INCREF(obj);
  return obj;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::encode_hexdigit
//       Access: Public
//  Description: Returns the hex digit corresponding to the
//               indicated integer value.
////////////////////////////////////////////////////////////////////
inline char P3DInstanceManager::
encode_hexdigit(int c) {
  if (c >= 10) {
    return c - 10 + 'a';
  }
  return c + '0';
}
