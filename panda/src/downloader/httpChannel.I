// Filename: httpChannel.I
// Created by:  drose (24Sep02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::is_valid
//       Access: Published
//  Description: Returns true if the last-requested document was
//               successfully retrieved and is ready to be read, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool HTTPChannel::
is_valid() const {
  return (!_source.is_null() && _state != S_failure && 
          (_status_code / 100) == 2);
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_url
//       Access: Published
//  Description: Returns the URL that was used to retrieve the
//               most recent document: whatever URL was last passed to
//               get_document() or get_header().  If a redirect has
//               transparently occurred, this will return the new,
//               redirected URL (the actual URL at which the document
//               was located).
////////////////////////////////////////////////////////////////////
INLINE const URLSpec &HTTPChannel::
get_url() const {
  return _url;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_http_version
//       Access: Published
//  Description: Returns the HTTP version number returned by the
//               server, as one of the HTTPClient enumerated types,
//               e.g. HTTPClient::HV_11.
////////////////////////////////////////////////////////////////////
INLINE HTTPClient::HTTPVersion HTTPChannel::
get_http_version() const {
  return _http_version;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_http_version_string
//       Access: Published
//  Description: Returns the HTTP version number returned by the
//               server, formatted as a string, e.g. "HTTP/1.1".
////////////////////////////////////////////////////////////////////
INLINE const string &HTTPChannel::
get_http_version_string() const {
  return _http_version_string;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_status_code
//       Access: Published
//  Description: Returns the HTML return code from the document
//               retrieval request.  This will be in the 200 range if
//               the document is successfully retrieved, or some other
//               value in the case of an error.
////////////////////////////////////////////////////////////////////
INLINE int HTTPChannel::
get_status_code() const {
  return _status_code;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_status_string
//       Access: Published
//  Description: Returns the string as returned by the server
//               describing the status code for humans.  This may or
//               may not be meaningful.
////////////////////////////////////////////////////////////////////
INLINE const string &HTTPChannel::
get_status_string() const {
  return _status_string;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_realm
//       Access: Published
//  Description: If the document failed to connect because of a 401
//               (Authorization required) or 407 (Proxy authorization
//               required) error, this method should return the
//               "realm" returned by the server in which the requested
//               document must be authenticated.  This string may be
//               presented to the user to request an associated
//               username and password (which then should be stored in
//               HTTPClient::set_username()).
//
//               If the document was retrieved successfully, or failed
//               to connect for some other reason, this string will be
//               empty.  If it is empty even in the presence of a 401
//               or 407 error, then the client code and the server do
//               not have a common scheme for exchanging passwords.
////////////////////////////////////////////////////////////////////
INLINE const string &HTTPChannel::
get_realm() const {
  return _realm;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_redirect
//       Access: Published
//  Description: If the document failed with a redirect code (300
//               series), this will generally contain the new URL the
//               server wants us to try.  In many cases, the client
//               will automatically follow redirects; if these are
//               succesful the client will return a successful code
//               and get_redirect() will return empty, but get_url()
//               will return the new, redirected URL.
////////////////////////////////////////////////////////////////////
INLINE const URLSpec &HTTPChannel::
get_redirect() const {
  return _redirect;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::set_persistent_connection
//       Access: Published
//  Description: Indicates whether the HTTPChannel should try to keep
//               the connection to the server open and reuse that
//               connection for multiple documents, or whether it
//               should close the connection and open a new one for
//               each request.  Set this true to keep the connections
//               around when possible, false to recycle them.
//
//               It makes most sense to set this false when the
//               HTTPChannel will be used only once to retrieve a
//               single document, true when you will be using the same
//               HTTPChannel object to retrieve multiple documents.
////////////////////////////////////////////////////////////////////
INLINE void HTTPChannel::
set_persistent_connection(bool persistent_connection) {
  _persistent_connection = persistent_connection;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_persistent_connection
//       Access: Published
//  Description: Returns whether the HTTPChannel should try to keep
//               the connection to the server open and reuse that
//               connection for multiple documents, or whether it
//               should close the connection and open a new one for
//               each request.  See set_persistent_connection().
////////////////////////////////////////////////////////////////////
INLINE bool HTTPChannel::
get_persistent_connection() const {
  return _persistent_connection;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_file_size
//       Access: Published
//  Description: Returns the size of the file, if it is known.
//               Returns 0 if the file size is not known.  
//
//               If the file is dynamically generated, the size may
//               not be available until a read has started
//               (e.g. open_read_file() has been called); and even
//               then it may increase as more of the file is read due
//               to the nature of HTTP/1.1 requests which can change
//               their minds midstream about how much data they're
//               sending you.
////////////////////////////////////////////////////////////////////
INLINE size_t HTTPChannel::
get_file_size() const {
  return _file_size;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::post_form
//       Access: Published
//  Description: Posts form data to a particular URL and retrieves the
//               response.
////////////////////////////////////////////////////////////////////
INLINE bool HTTPChannel::
post_form(const URLSpec &url, const string &body) {
  begin_request("POST", url, body, false);
  run();
  return is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_document
//       Access: Published
//  Description: Opens the named document for reading, if available.
//               Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool HTTPChannel::
get_document(const URLSpec &url) {
  begin_request("GET", url, string(), false);
  run();
  return is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::get_header
//       Access: Published
//  Description: Like get_document(), except only the header
//               associated with the document is retrieved.  This may
//               be used to test for existence of the document; it
//               might also return the size of the document (if the
//               server gives us this information).
////////////////////////////////////////////////////////////////////
INLINE bool HTTPChannel::
get_header(const URLSpec &url) {
  begin_request("HEAD", url, string(), false);
  run();
  return is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::request_post_form
//       Access: Published
//  Description: Posts form data to a particular URL and retrieves the
//               response, all using non-blocking I/O.  See
//               request_document() and post_form().
//
//               It is important to note that you *must* call run()
//               repeatedly after calling this method until run()
//               returns false, and you may not call any other
//               document posting or retrieving methods using the
//               HTTPChannel object in the interim, or your form data
//               may not get posted.
////////////////////////////////////////////////////////////////////
INLINE void HTTPChannel::
request_post_form(const URLSpec &url, const string &body) {
  begin_request("POST", url, body, true);
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::request_document
//       Access: Published
//  Description: Begins a non-blocking request to retrieve a given
//               document.  This method will return immediately, even
//               before a connection to the server has necessarily
//               been established; you must then call run() from time
//               to time until the return value of run() is false.
//               Then you may check is_valid() and get_status_code()
//               to determine the status of your request.
//
//               If a previous request had been pending, that request
//               is discarded.
////////////////////////////////////////////////////////////////////
INLINE void HTTPChannel::
request_document(const URLSpec &url) {
  begin_request("GET", url, string(), true);
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::request_header
//       Access: Published
//  Description: Begins a non-blocking request to retrieve a given
//               header.  See request_document() and get_header().
////////////////////////////////////////////////////////////////////
INLINE void HTTPChannel::
request_header(const URLSpec &url) {
  begin_request("HEAD", url, string(), true);
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPChannel::check_socket
//       Access: Private
//  Description: Checks whether the connection to the server has been
//               closed after a failed read.  If it has, issues a
//               warning and calls free_bio().
////////////////////////////////////////////////////////////////////
INLINE void HTTPChannel::
check_socket() {
  nassertv(!_source.is_null());
  if ((*_source)->is_closed()) {
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << "Lost connection to server unexpectedly during read.\n";
    }
    free_bio();
  }
}
