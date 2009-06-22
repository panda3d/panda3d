/* Filename: p3d_plugin.h
 * Created by:  drose (28May09)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef P3D_PLUGIN_H
#define P3D_PLUGIN_H

/* This file defines the C-level API to Panda's plugin system.  This
   API is intended to provide basic functionality for loading and
   running Panda's .p3d files, particularly within a browser. 

   This plugin code is intended to be loaded and run as a standalone
   DLL.  It will in turn be responsible for fetching and installing
   the appropriate version of Panda and Python, as well as any
   required supporting libraries.

   Note that this code defines only the interface between the actual
   browser plugin and the Panda code.  The actual plugin itself will
   be a separate piece of code, written in ActiveX or NPIP or whatever
   API is required for a given browser, which is designed to download
   and link with this layer.

   The browser or launching application will be referred to as the
   "host" in this documentation.  The host should load this plugin dll
   only once, but may then use it to create multiple simultaneous
   different instances of Panda windows.

   Filenames passed through this interface are in native OS-specific
   form, e.g. with a leading drive letter and backslashes, not in
   Panda's Unix-like form (except on Unix-based OSes, of course).
*/

#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>

#ifdef BUILDING_P3D_PLUGIN
#define EXPCL_P3D_PLUGIN __declspec(dllexport)
#else
#define EXPCL_P3D_PLUGIN __declspec(dllimport)
#endif

#else  /* _WIN32 */
#define EXPCL_P3D_PLUGIN

#endif  /* _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

/* In the following function prototypes, all functions are declared
   initially as typedefs only, and then the actual function references
   are finally declared at the end of this file, but only if
   P3D_PLUGIN_PROTOTYPES is defined.  This is intended to allow
   including this file without building an implicit reference to the
   functions themselves, allowing the plugin library to be loaded via
   an explicit LoadLibrary() or equivalent call. */


/* This symbol serves to validate that runtime and compile-time
libraries match.  It should be passed to P3D_initialize() (below).
This number will be incremented whenever there are changes to any of
the interface specifications defined in this header file. */
#define P3D_API_VERSION 2

/************************ GLOBAL FUNCTIONS **************************/

/* The following interfaces are global to the plugin space, as opposed
   to being specific to a particular instance. */

/* This function should be called immediately after the plugin is
   loaded.  You should pass P3D_API_VERSION as the first parameter, so
   the dll can verify that it has been built with the same version of
   the API as the host.

   The output_filename is usually NULL, but if you put a filename
   here, it will be used as the log file for the output from the
   plugin.  This is useful for debugging, particularly when running
   within a browser that squelches stderr.

   This function returns true if the plugin is valid and uses a
   compatible API, false otherwise.  If it returns false, the host
   should not call any more functions in this API, and should
   immediately unload the DLL and (if possible) download a new one. */
typedef bool 
P3D_initialize_func(int api_version, const char *output_filename);

/* This function frees a pointer returned by
   P3D_instance_get_property(), or another similar function that
   returns a dynamically-allocated string. */
typedef void 
P3D_free_string_func(char *string);

/********************** INSTANCE MANAGEMENT **************************/

/* The following interfaces define the API to manage individual
   Panda3D instances.  Each instance can display a separate 3-D
   graphics window simultaneously on the host or on the desktop.  The
   instances operate generally independently of each other. */

/* This structure defines the handle to a single instance.  The host
   may access the _request_pending member, which will be true if the
   host should call P3D_instance_get_request(). */
typedef struct {
  bool _request_pending;

  /* Additional opaque data may be stored here. */
} P3D_instance;

/* This structure abstracts out the various window handle types for
   the different platforms. */
typedef struct {
#ifdef _WIN32
  HWND _hwnd;
#endif
#ifdef __APPLE__
  void *_nswindow;
#endif
} P3D_window_handle;

/* This enum lists the different kinds of window types that may be
   requested for the instance.  These define the way that the instance
   will create its main Panda3D window.  The instance will treat this
   as a request only; it is always free to create whatever kind of
   window it likes. */
typedef enum {
  /* Embedded: the plugin window is embedded within the host window.
     This is the normal kind of window for an object embedded within a
     browser page.  Pass a valid window handle in for parent_window,
     and valid coordinates on the parent window for win_x, win_y,
     win_width, win_height. */
  P3D_WT_embedded,

  /* Toplevel: the plugin window is a toplevel window on the user's
     desktop.  Pass valid desktop coordinates in for win_x, win_y,
     win_width, and win_height.  If all of these are zero, the plugin
     will create a window wherever it sees fit. */
  P3D_WT_toplevel,

  /* Fullscreen: the plugin window is a fullscreen window, completely
     overlaying the entire screen and changing the desktop resolution.
     Pass a valid desktop size in for win_width and win_height (win_x
     and win_y are ignored).  If win_width and win_height are zero,
     the plugin will create a fullscreen window of its own preferred
     size. */
  P3D_WT_fullscreen,

  /* Hidden: there is no window at all for the plugin. */
  P3D_WT_hidden,

} P3D_window_type;


/* This function pointer must be passed to P3D_create_instance(),
   below.  The host must pass in a pointer to a valid function in the
   host's address space, or NULL.  If not NULL, this function will be
   called asynchronously by the plugin when the plugin needs to make a
   request from the host.  After this notification has been received,
   the host should call P3D_instance_get_request() (at its
   convenience) to retrieve the actual plugin request.  If the host
   passes NULL for this function pointer, asynchronous notifications
   will not be provided, and the host must be responsible for calling
   P3D_instance_get_request() from time to time. */

/* Note that, unlike the other func typedefs in this header file, this
   declaration is not naming a function within the plugin itself.
   Instead, it is a typedef for a function pointer that must be
   supplied by the host. */

typedef void
P3D_request_ready_func(P3D_instance *instance);

/* This structure is used to represent a single keyword/value pair
   that appears within the embed syntax on the HTML page.  An array of
   these values is passed to the P3D instance to represent all of the
   additional keywords that may appear within this syntax; it is up to
   the plugin to interpret these additional keywords correctly. */
typedef struct {
  const char *_keyword;
  const char *_value;
} P3D_token;

/* This function creates a new Panda3D instance.  

   For p3d_filename pass the name of a file on disk that contains the
   contents of the p3d file that should be launched within the
   instance.  If this is empty or NULL, the "src" token (below) will
   be downloaded instead.

   For tokens, pass an array of P3D_token elements (above), which
   correspond to the user-supplied keyword/value pairs that may appear
   in the embed token within the HTML syntax; the host is responsible
   for allocating this array, and for deallocating it after this call
   (the plugin will make its own copy of the array).

   Most tokens are implemented by the application and are undefined at
   the system level.  However, two tokens in particular are
   system-defined:

     "src" : names a URL that will be loaded for the contents of the
       p3d file, if p3d_filename is empty or NULL.

     "output_filename" : names a file to create on disk which contains
       the console output from the application.  This may be useful in
       debugging.  If this is omitted, or an empty string, the console
       output is written to the standard error output, which may be
       NULL on a gui application.

 */

typedef P3D_instance *
P3D_create_instance_func(P3D_request_ready_func *func,
                         const char *p3d_filename, 
                         const P3D_token tokens[], size_t num_tokens);


/* Call this function to interrupt a particular instance and stop it
   from rendering, for instance when the user navigates away from the
   page containing it.  After calling this function, you should not
   reference the P3D_instance pointer again. */
typedef void 
P3D_instance_finish_func(P3D_instance *instance);

/* Call this function after creating an instance in order to set its
   window size and placement.  You must call this at least once in
   order to actually manifest the instance onscreen.  This may also be
   called to reposition a window after its initial placement (e.g. if
   the browser window has changed width and needs reformatting). */
typedef void
P3D_instance_setup_window_func(P3D_instance *instance,
                               P3D_window_type window_type,
                               int win_x, int win_y,
                               int win_width, int win_height,
                               P3D_window_handle parent_window);


/********************** SCRIPTING SUPPORT **************************/

/* The following interfaces are provided to support controlling the
   plugin via JavaScript or related interfaces on the browser. */

/* Call this function to query whether the instance has a property of
   the indicated name.  If this returns true, you may then query
   P3D_instance_get_property() or P3D_instance_set_property(). */
typedef bool
P3D_instance_has_property_func(P3D_instance *instance,
                               const char *property_name);

/* Call this function to query the value of the indicated property.
   It is an error to call this if the property does not exist; call
   P3D_instance_has_property() first to ensure this is so.  The return
   value has been dynamically allocated and should be passed to
   P3D_free_string() when it is no longer needed. */
typedef char *
P3D_instance_get_property_func(P3D_instance *instance,
                               const char *property_name);

/* Call this function to set the value of the indicated property. */
typedef void 
P3D_instance_set_property_func(P3D_instance *instance,
                               const char *property_name,
                               const char *value);

/********************** REQUEST HANDLING **************************/

/* The plugin may occasionally have an asynchronous request to pass up
   to the host.  The following structures implement this interface.
   The design is intended to support single-threaded as well as
   multi-threaded implementations in the host; there is only the one
   callback function, P3D_request_ready (above), which may be called
   asynchronously by the plugin.  The host should be careful that this
   callback function is protected from mutual access.  The callback
   function implementation may be as simple as setting a flag that the
   host will later check within its main processing loop.

   Once P3D_request_ready() has been received, the host should call
   P3D_instance_get_request() to query the nature of the request.
   This call may be made synchronously, i.e. within the host's main
   processing loop.  After each request is serviced, the host should
   release the request via P3D_request_finish() and then call
   P3D_instance_get_request() again until that function returns NULL.

   The requests themselves are implemented via a hierarchy of structs.
   Each request is stored in a different kind of struct, allowing the
   different requests to store a variety of data.  An enumerated value
   indicates the particular request type retrieved. */

/* This represents the type of a request returned by
   P3D_instance_get_request.  More types may be added later. */
typedef enum {
  P3D_RT_stop,
  P3D_RT_unused, //  P3D_RT_new_config_xml,
  P3D_RT_get_url,
  P3D_RT_post_url,
} P3D_request_type;

/* Structures corresponding to the request types in the above enum. */

/* A stop request.  The instance would like to stop itself.  No
   additional data is required.  The host should respond by calling
   P3D_instance_finish(). */
typedef struct {
} P3D_request_stop;

/* A get_url request.  The plugin would like to retrieve data for a
   particular URL.  The plugin is responsible for supplying a valid
   URL string, and a unique integer ID.  The unique ID is needed to
   feed the results of the URL back to the plugin.  If possible, the
   host should be prepared to handle multiple get_url requests in
   parallel, but it is allowed to handle them all one at a time if
   necessary.  As data comes in from the url, the host should call
   P3D_instance_feed_url_stream(). 
*/
typedef struct {
  const char *_url;
  int _unique_id;
} P3D_request_get_url;

/* A post_url request.  Similar to get_url, but additional data is to
   be sent via POST to the indicated URL.  The result of the POST is
   returned in a mechanism similar to get_url.
*/
typedef struct {
  const char *_url;
  const char *_post_data;
  size_t _post_data_size;
  int _unique_id;
} P3D_request_post_url;

/* This is the overall structure that represents a single request.  It
   is returned by P3D_instance_get_request(). */
typedef struct {
  P3D_instance *_instance;
  P3D_request_type _request_type;
  union {
    P3D_request_stop _stop;
    P3D_request_get_url _get_url;
    P3D_request_post_url _post_url;
  } _request;
} P3D_request;

/* After a call to P3D_request_ready(), or from time to time in
   general, the host should call this function to see if there are any
   pending requests from the plugin.  The function will return a
   freshly-allocated request if there is a request ready, or NULL if
   there are no requests.  After a receipt of P3D_request_ready(),
   the host should call this function repeatedly until it returns NULL
   (there might be multiple requests for a single receipt of
   P3D_request_ready()).  Each request should be processed, then
   released via P3D_request_finish(). */
typedef P3D_request *
P3D_instance_get_request_func(P3D_instance *instance);

/* This method may also be used to test whether a request is ready.
   If any open instance has a pending request, this function will
   return a pointer to one of them (which you may then pass to
   P3D_instance_get_request_func).  If no instances have a pending
   request, this function will return NULL.  If wait is true, this
   function will never return NULL unless there are no instances open;
   instead, it will wait indefinitely until there is a request
   available.

   Note that, due to race conditions, it is possible for this function
   to return a P3D_instance that does not in fact have any requests
   pending (another thread may have checked the request first).  You
   should always verify that the return value of
   P3D_instance_get_request() is not NULL. */
typedef P3D_instance *
P3D_check_request_func(bool wait);

/* A request retrieved by P3D_instance_get_request() should eventually
   be passed here, after it has been handled, to deallocate its
   resources and prevent a memory leak.  The 'handled' flag should be
   passed true if the host has handled the request, or false if it has
   ignored it (e.g. because it does not implement support for this
   particular request type).  After calling this function, you should
   not reference the P3D_request pointer again. */
typedef void
P3D_request_finish_func(P3D_request *request, bool handled);

/* This code is passed to P3D_instance_feed_url_stream, below, as data
   is retrieved from the URL. */
typedef enum {
  /* in progress: the query is still in progress, and another call
     will be made in the future. */
  P3D_RC_in_progress,

  /* done: the query is done, and all data has been retrieved without
     error.  This call represents the last of the data. */
  P3D_RC_done,

  /* generic_error: some error other than an HTTP error has occurred,
     for instance, lack of connection to the server, or malformed URL.
     No more data will be forthcoming. */
  P3D_RC_generic_error,

  /* An HTTP error has occurred, for instance 404.  The particular
     status code will be supplied in the http_status_code parameter.
     There may or may not be data associated with this error as well.
     However, no more data will be delivered after this call. */
  P3D_RC_http_error,
} P3D_result_code;

/* This function is used by the host to handle a get_url request,
   above.  As it retrieves data from the URL, it should call this
   function from time to time to feed that data to the plugin.

   instance and unique_id are from the original get_url() request.

   result_code and http_status_code indicates the current status of
   the request, as described above; the call will be made again in the
   future if its result_code is P3D_RC_in_progress.

   total_expected_data represents the host's best guess at the total
   amount of data that will be retrieved.  It is acceptable if this
   guess doesn't match the actual data received at all.  Set it to 0
   if the host has no idea.  This value may change from one call to
   the next.

   this_data and this_data_size describe the most recent block of data
   retrieved from the URL.  Each chunk of data passed to this function
   is appended together by the plugin to define the total set of data
   retrieved from the URL.  For a particular call to feed_url_stream,
   this may contain no data at all (e.g. this_data_size may be 0).

   The return value of this function is true if there are no problems
   and the download should continue, false if there was an error
   accepting the data and the host should abort.  If this function
   returns false on a P3D_RC_in_progress, there is no need to call
   the function again with any future updates.
 */
typedef bool
P3D_instance_feed_url_stream_func(P3D_instance *instance, int unique_id,
                                  P3D_result_code result_code,
                                  int http_status_code, 
                                  size_t total_expected_data,
                                  const unsigned char *this_data, 
                                  size_t this_data_size);


#ifdef P3D_FUNCTION_PROTOTYPES

/* Define all of the actual prototypes for the above functions. */
EXPCL_P3D_PLUGIN P3D_initialize_func P3D_initialize;
EXPCL_P3D_PLUGIN P3D_free_string_func P3D_free_string;
EXPCL_P3D_PLUGIN P3D_create_instance_func P3D_create_instance;
EXPCL_P3D_PLUGIN P3D_instance_finish_func P3D_instance_finish;
EXPCL_P3D_PLUGIN P3D_instance_setup_window_func P3D_instance_setup_window;
EXPCL_P3D_PLUGIN P3D_instance_has_property_func P3D_instance_has_property;
EXPCL_P3D_PLUGIN P3D_instance_get_property_func P3D_instance_get_property;
EXPCL_P3D_PLUGIN P3D_instance_set_property_func P3D_instance_set_property;
EXPCL_P3D_PLUGIN P3D_instance_get_request_func P3D_instance_get_request;
EXPCL_P3D_PLUGIN P3D_check_request_func P3D_check_request;
EXPCL_P3D_PLUGIN P3D_request_finish_func P3D_request_finish;
EXPCL_P3D_PLUGIN P3D_instance_feed_url_stream_func P3D_instance_feed_url_stream;
#endif  /* P3D_FUNCTION_PROTOTYPES */

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif  /* P3D_PLUGIN_H */


