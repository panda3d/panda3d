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

/* This file defines the C-level API to Panda's core plugin API.  This
   API is intended to provide basic functionality for loading and
   running Panda's .p3d files, particularly within a browser.

   This core API is intended to be loaded and run within a browser, as
   a standalone DLL.  It will in turn be responsible for fetching and
   installing the appropriate version of Panda and Python, as well as
   any required supporting libraries.

   Note that this code defines only the interface between the actual
   browser plugin and the Panda code.  It contains no code to directly
   interface with any browser.  The actual plugin itself will be a
   separate piece of code, written in ActiveX or NPAPI or whatever API
   is required for a given browser; and this code will be designed to
   download and link with this DLL.

   The browser or launching application will be referred to as the
   "host" in this documentation.  The host should load this core API
   DLL only once, but may then use it to create multiple simultaneous
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
   functions themselves, allowing the core API library to be loaded
   via an explicit LoadLibrary() or equivalent call. */


/* This symbol serves to validate that runtime and compile-time
libraries match.  It should be passed to P3D_initialize() (below).
This number will be incremented whenever there are changes to any of
the interface specifications defined in this header file. */
#define P3D_API_VERSION 3

/************************ GLOBAL FUNCTIONS **************************/

/* The following interfaces are global to the core API space, as
   opposed to being specific to a particular instance. */

/* This function should be called immediately after the core API is
   loaded.  You should pass P3D_API_VERSION as the first parameter, so
   the DLL can verify that it has been built with the same version of
   the API as the host.

   The output_filename is usually NULL, but if you put a filename
   here, it will be used as the log file for the output from the core
   API.  This is useful for debugging, particularly when running
   within a browser that squelches stderr.

   This function returns true if the core API is valid and uses a
   compatible API, false otherwise.  If it returns false, the host
   should not call any more functions in this API, and should
   immediately unload the DLL and (if possible) download a new one. */
typedef bool 
P3D_initialize_func(int api_version, const char *output_filename);

/* This function should be called to unload the core API.  It will
   release all internally-allocated memory and return the core API to
   its initial state. */
typedef void
P3D_finalize_func();

/********************** INSTANCE MANAGEMENT **************************/

/* The following interfaces define the API to manage individual
   Panda3D instances.  Each instance can display a separate 3-D
   graphics window simultaneously on the host or on the desktop.  The
   instances operate generally independently of each other. */

/* This structure defines the handle to a single instance.  The host
   may access any members appearing here. */
typedef struct {
  /* true if the host should call P3D_instance_get_request().*/
  bool _request_pending;

  /* an opaque pointer the host may use to store private data that the
     core API does not interpret.  This pointer can be directly set, or
     it can be initialized in the P3D_new_instance() call. */
  void *_user_data;

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
  /* Embedded: the Panda window is embedded within the host window.
     This is the normal kind of window for an object embedded within a
     browser page.  Pass a valid window handle in for parent_window,
     and valid coordinates on the parent window for win_x, win_y,
     win_width, win_height. */
  P3D_WT_embedded,

  /* Toplevel: the Panda window is a toplevel window on the user's
     desktop.  Pass valid desktop coordinates in for win_x, win_y,
     win_width, and win_height.  If all of these are zero, the core
     API will create a window wherever it sees fit. */
  P3D_WT_toplevel,

  /* Fullscreen: the Panda window is a fullscreen window, completely
     overlaying the entire screen and changing the desktop resolution.
     Pass a valid desktop size in for win_width and win_height (win_x
     and win_y are ignored).  If win_width and win_height are zero,
     the core API will create a fullscreen window of its own preferred
     size. */
  P3D_WT_fullscreen,

  /* Hidden: there is no window at all for the instance. */
  P3D_WT_hidden,

} P3D_window_type;


/* This function pointer must be passed to P3D_new_instance(), below.
   The host must pass in a pointer to a valid function in the host's
   address space, or NULL.  If not NULL, this function will be called
   asynchronously by the core API when it needs to make a request from
   the host.  After this notification has been received, the host
   should call P3D_instance_get_request() (at its convenience) to
   retrieve the actual Panda request.  If the host passes NULL for
   this function pointer, asynchronous notifications will not be
   provided, and the host must be responsible for calling
   P3D_instance_get_request() from time to time. */

/* Note that, unlike the other func typedefs in this header file, this
   declaration is not naming a function within the core API itself.
   Instead, it is a typedef for a function pointer that must be
   supplied by the host. */

typedef void
P3D_request_ready_func(P3D_instance *instance);

/* This structure is used to represent a single keyword/value pair
   that appears within the embed syntax on the HTML page.  An array of
   these values is passed to the P3D instance to represent all of the
   additional keywords that may appear within this syntax; it is up to
   the Panda instance to interpret these additional keywords
   correctly. */
typedef struct {
  const char *_keyword;
  const char *_value;
} P3D_token;

/* This function creates a new Panda3D instance.  

   The user_data pointer is any arbitrary pointer value; it will be
   copied into the _user_data member of the new P3D_instance object.
   This pointer is intended for the host to use to store private data
   associated with each instance; the core API will not do anything with
   this data.

 */

typedef P3D_instance *
P3D_new_instance_func(P3D_request_ready_func *func, void *user_data);

/* This function should be called at some point after
   P3D_new_instance(); it actually starts the instance running.
   Before this call, the instance will be in an indeterminate state.

   For p3d_filename pass the name of a file on disk that contains the
   contents of the p3d file that should be launched within the
   instance.  If this is empty or NULL, the "src" token (below) will
   be downloaded instead.

   For tokens, pass an array of P3D_token elements (above), which
   correspond to the user-supplied keyword/value pairs that may appear
   in the embed token within the HTML syntax; the host is responsible
   for allocating this array, and for deallocating it after this call
   (the core API will make its own copy of the array).

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

   The return value is true on success, false on failure. */
typedef bool
P3D_instance_start_func(P3D_instance *instance, const char *p3d_filename, 
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
   Panda instance via JavaScript or related interfaces on the browser. */

/* We require an "object" that contains some number of possible
   different interfaces.  An "object" might be a simple primitive like
   an int, float, or string; or it might be a class object with
   methods and properties.  Instances of P3D_object are passed around
   as parameters into and return values from functions.
  
   To implement a P3D_object, we need to first define a class
   definition, which is a table of methods.  Most classes are defined
   internally by the core API, but the host must define at least one
   class type as well, which provides callbacks into host-provided
   objects.

   These function types define the methods available on a class.
   These are function type declarations only; they do not correspond
   to named functions within the core API DLL.  Instead, the function
   pointers themselves are stored within the P3D_class_definition
   structure, below. */

/* A forward declaration of P3D_object. */
typedef struct _P3D_object P3D_object;

/* A list of fundamental object types. */
typedef enum {
  P3D_OT_null,
  P3D_OT_none,
  P3D_OT_bool,
  P3D_OT_int,
  P3D_OT_float,
  P3D_OT_string,
  P3D_OT_object,
} P3D_object_type;

/* This method is called to deallocate the object and all of its
   internal structures. */
typedef void
P3D_object_finish_method(P3D_object *object);

/* Returns a new copy of the object.  The caller receives ownership of
   the new object and must eventually pass its pointer to
   P3D_OBJECT_FINISH() to delete it, or into some other call that
   transfers ownership. */
typedef P3D_object *
P3D_object_copy_method(const P3D_object *object);

/* Returns the fundamental type of the object.  This should be treated
   as a hint to suggest how the object can most accurately be
   represented; it does not limit the actual interfaces available to
   an object.  For instance, you may call P3D_OBJECT_GET_PROPERTY()
   even if the object's type is not "object". */
typedef P3D_object_type
P3D_object_get_type_method(const P3D_object *object);

/* Each of the following methods returns the object's value expressed
   as the corresponding type.  If the object is not precisely that
   type, a coercion is performed. */

/* Return the object as a bool. */
typedef bool
P3D_object_get_bool_method(const P3D_object *object);

/* Return the object as an integer. */
typedef int
P3D_object_get_int_method(const P3D_object *object);

/* Return the object as a floating-point number. */
typedef double
P3D_object_get_float_method(const P3D_object *object);

/* Get the object as a string.  This method copies the string into the
   provided buffer, and returns the actual length of the internal
   string (not counting any terminating null character).  If the
   return value is larger than buffer_length, the string has been
   truncated.  If it is equal, there is no null character written to
   the buffer (like strncpy). */
typedef int
P3D_object_get_string_method(const P3D_object *object, 
                             char *buffer, int buffer_length);

/* As above, but instead of the literal object data, returns a
   user-friendly reprensentation of the object as a string.  For
   instance, a string object returns the string itself to
   P3D_OBJECT_GET_STRING(), but returns the string with quotation
   marks and escape characters from P3D_OBJECT_GET_REPR().
   Mechanically, this function works the same way as get_string(). */
typedef int
P3D_object_get_repr_method(const P3D_object *object, 
                           char *buffer, int buffer_length);

/* Looks up a property on the object by name, i.e. a data member or a
   method.  The return value is a newly-allocated P3D_object if the
   property exists, or NULL if it does not.  If it is not NULL,
   ownership of the return value is transferred to the caller, who
   will be responsible for deleting it later. */
typedef P3D_object *
P3D_object_get_property_method(const P3D_object *object, const char *property);

/* Changes the value at the indicated property.  Any existing object
   already at the corresponding property is deleted.  If the value
   object pointer is NULL, the property is deleted.  Returns true on
   success, false on failure.  The caller must have ownership of the
   value object before the call; after the call, ownership of the
   value object is transferred to this object. */
typedef bool
P3D_object_set_property_method(P3D_object *object, const char *property,
                               P3D_object *value);

/* Invokes a named method on the object.  If method_name is empty or
   NULL, invokes the object itself as a function.  You must pass an
   array of P3D_objects as the list of parameters.  The ownership of
   each of the parameters in this array (but not of the array pointer
   itself) is passed into this call; the objects will be deleted when
   the call is completed.

   The return value is a newly-allocated P3D_object on success, or
   NULL on failure.  Ownership of the return value is transferred to
   the caller. */
typedef P3D_object *
P3D_object_call_method(const P3D_object *object, const char *method_name,
                       P3D_object *params[], int num_params);

/* This defines the class structure that implements all of the above
   methods. */
typedef struct _P3D_class_definition {
  P3D_object_finish_method *_finish;
  P3D_object_copy_method *_copy;

  P3D_object_get_type_method *_get_type;
  P3D_object_get_bool_method *_get_bool;
  P3D_object_get_int_method *_get_int;
  P3D_object_get_float_method *_get_float;
  P3D_object_get_string_method *_get_string;
  P3D_object_get_repr_method *_get_repr;

  P3D_object_get_property_method *_get_property;
  P3D_object_set_property_method *_set_property;

  P3D_object_call_method *_call;

} P3D_class_definition;

/* And this structure defines the actual instances of P3D_object. */
struct _P3D_object {
  const P3D_class_definition *_class;

  /* Additional opaque data may be stored here. */
};

/* These macros are defined for the convenience of invoking any of the
   above method functions on an object. */

#define P3D_OBJECT_FINISH(object) ((object)->_class->_finish((object)))
#define P3D_OBJECT_COPY(object) ((object)->_class->_copy((object)))

#define P3D_OBJECT_GET_TYPE(object) ((object)->_class->_get_type((object)))
#define P3D_OBJECT_GET_BOOL(object) ((object)->_class->_get_bool((object)))
#define P3D_OBJECT_GET_INT(object) ((object)->_class->_get_int((object)))
#define P3D_OBJECT_GET_FLOAT(object) ((object)->_class->_get_float((object)))
#define P3D_OBJECT_GET_STRING(object, buffer, buffer_size) ((object)->_class->_get_string((object), (buffer), (buffer_size)))
#define P3D_OBJECT_GET_REPR(object, buffer, buffer_size) ((object)->_class->_get_repr((object), (buffer), (buffer_size)))

#define P3D_OBJECT_GET_PROPERTY(object, property) ((object)->_class->_get_property((object), (property)))
#define P3D_OBJECT_SET_PROPERTY(object, property, value) ((object)->_class->_set_property((object), (property), (value)))

#define P3D_OBJECT_CALL(object, method_name, params, num_params) ((object)->_class->_call((object), (method_name), (params), (num_params)))


/* The following function types are once again meant to define
   actual function pointers to be found within the core API DLL. */

/* Returns a newly-allocated P3D_class_definition object, filled with
   generic function pointers that have reasonable default behavior for
   all methods.  The host should use this function to get a clean
   P3D_class_definition object before calling
   P3D_instance_set_browser_script_object() (see below).  Note that this
   pointer will automatically be freed when P3D_finalize() is
   called. */
typedef P3D_class_definition *
P3D_make_class_definition_func();

/* Allocates a new P3D_object of type null.  This value has no
   particular value and corresponds a NULL pointer in C or JavaScript.
   It has no Python equivalent (but we map it to an explicit Null
   instance in runp3d.py). */
typedef P3D_object *
P3D_new_null_object_func();

/* Allocates a new P3D_object of type none.  This value has no
   particular value and corresponds to Python's None type or C's void
   type. */
typedef P3D_object *
P3D_new_none_object_func();

/* Allocates a new P3D_object of type bool. */
typedef P3D_object *
P3D_new_bool_object_func(bool value);

/* Allocates a new P3D_object of type int. */
typedef P3D_object *
P3D_new_int_object_func(int value);

/* Allocates a new P3D_object of type float. */
typedef P3D_object *
P3D_new_float_object_func(double value);

/* Allocates a new P3D_object of type string.  The supplied string is
   copied into the object and stored internally. */
typedef P3D_object *
P3D_new_string_object_func(const char *string, int length);

/* Returns a pointer to the top-level scriptable object of the
   instance.  Scripts running on the host may use this object to
   communicate with the instance, by using the above methods to set or
   query properties, and/or call methods, on the instance. 

   The return value from this function is a newly-allocated object;
   ownership of the object is passed to the caller, who should be
   responsible for deleting it eventually. */
typedef P3D_object *
P3D_instance_get_panda_script_object_func(P3D_instance *instance);

/* The inverse functionality: this supplies an object pointer to the
   instance to allow the Panda instance to control the browser.  In
   order to enable browser scriptability, the host must call this
   method shortly after creating the instance, preferably before
   calling P3D_instance_start().

   The object parameter must have been created by the host.  It will
   have a custom P3D_class_definition pointer, which also must have
   been created by the host.  The best way to create an appropriate
   class definition is call P3D_make_class_definition(), and then
   replace the function pointers for at least _finish, _copy,
   _get_property, _set_property, and _call.  Set these pointers to the
   host's own functions that make the appropriate changes in the DOM,
   or invoke the appropriate JavaScript functions.

   If this function is never called, the instance will not be able to
   make outcalls to the DOM or to JavaScript, but scripts may still be
   able to control the instance via P3D_instance_get_panda_script_object(),
   above. 

   Ownership of the object is passed into the instance.  The caller
   must have freshly allocated the object, and should no longer store
   or delete it.  The instance will eventually delete it by calling
   its _finish method. */
typedef void
P3D_instance_set_browser_script_object_func(P3D_instance *instance, 
                                            P3D_object *object);


/********************** REQUEST HANDLING **************************/

/* The core API may occasionally have an asynchronous request to pass
   up to the host.  The following structures implement this interface.
   The design is intended to support single-threaded as well as
   multi-threaded implementations in the host; there is only the one
   callback function, P3D_request_ready (above), which may be called
   asynchronously by the core API.  The host should be careful that
   this callback function is protected from mutual access.  The
   callback function implementation may be as simple as setting a flag
   that the host will later check within its main processing loop.

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
  P3D_RT_get_url,
  P3D_RT_post_url,
  P3D_RT_notify,
  P3D_RT_script,
} P3D_request_type;

/* Structures corresponding to the request types in the above enum. */

/* A stop request.  The instance would like to stop itself.  No
   additional data is required.  The host should respond by calling
   P3D_instance_finish(). */
typedef struct {
} P3D_request_stop;

/* A get_url request.  The core API would like to retrieve data for a
   particular URL.  The core API is responsible for supplying a valid
   URL string, and a unique integer ID.  The unique ID is needed to
   feed the results of the URL back to the core API.  If possible, the
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

/* A general notification.  This is just a message of some event
   having occurred within the Panda3D instance.  It may be safely
   ignored.
*/
typedef struct {
  const char *_message;
} P3D_request_notify;

/* A script object request.  This is used to call out into the
   browser_script_object (above).  This request is handled internally
   by the core API, and may safely be ignored by the host.
*/
typedef enum {
  P3D_SO_get_property,
  P3D_SO_set_property,
  P3D_SO_del_property,
  P3D_SO_call,
} P3D_script_operation;
typedef struct {
  P3D_object *_object;
  P3D_script_operation _op;
  const char *_property_name;
  P3D_object **_values;
  int _num_values;
  int _unique_id;
} P3D_request_script;

/* This is the overall structure that represents a single request.  It
   is returned by P3D_instance_get_request(). */
typedef struct {
  P3D_instance *_instance;
  P3D_request_type _request_type;
  union {
    P3D_request_stop _stop;
    P3D_request_get_url _get_url;
    P3D_request_post_url _post_url;
    P3D_request_notify _notify;
    P3D_request_script _script;
  } _request;
} P3D_request;

/* After a call to P3D_request_ready(), or from time to time in
   general, the host should call this function to see if there are any
   pending requests from the core API.  The function will return a
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
   function from time to time to feed that data to the core API.

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
   is appended together by the core API to define the total set of
   data retrieved from the URL.  For a particular call to
   feed_url_stream, this may contain no data at all
   (e.g. this_data_size may be 0).

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
                                  const void *this_data, 
                                  size_t this_data_size);

#ifdef P3D_FUNCTION_PROTOTYPES

/* Define all of the actual prototypes for the above functions. */
EXPCL_P3D_PLUGIN P3D_initialize_func P3D_initialize;
EXPCL_P3D_PLUGIN P3D_finalize_func P3D_finalize;

EXPCL_P3D_PLUGIN P3D_new_instance_func P3D_new_instance;
EXPCL_P3D_PLUGIN P3D_instance_start_func P3D_instance_start;
EXPCL_P3D_PLUGIN P3D_instance_finish_func P3D_instance_finish;
EXPCL_P3D_PLUGIN P3D_instance_setup_window_func P3D_instance_setup_window;

EXPCL_P3D_PLUGIN P3D_make_class_definition_func P3D_make_class_definition;
EXPCL_P3D_PLUGIN P3D_new_null_object_func P3D_new_null_object;
EXPCL_P3D_PLUGIN P3D_new_none_object_func P3D_new_none_object;
EXPCL_P3D_PLUGIN P3D_new_bool_object_func P3D_new_bool_object;
EXPCL_P3D_PLUGIN P3D_new_int_object_func P3D_new_int_object;
EXPCL_P3D_PLUGIN P3D_new_float_object_func P3D_new_float_object;
EXPCL_P3D_PLUGIN P3D_new_string_object_func P3D_new_string_object;
EXPCL_P3D_PLUGIN P3D_instance_get_panda_script_object_func P3D_instance_get_panda_script_object;
EXPCL_P3D_PLUGIN P3D_instance_set_browser_script_object_func P3D_instance_set_browser_script_object;

EXPCL_P3D_PLUGIN P3D_instance_get_request_func P3D_instance_get_request;
EXPCL_P3D_PLUGIN P3D_check_request_func P3D_check_request;
EXPCL_P3D_PLUGIN P3D_request_finish_func P3D_request_finish;
EXPCL_P3D_PLUGIN P3D_instance_feed_url_stream_func P3D_instance_feed_url_stream;

#endif  /* P3D_FUNCTION_PROTOTYPES */

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif  /* P3D_PLUGIN_H */


