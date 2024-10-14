/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpChannel_emscripten.I
 * @author rdb
 * @date 2021-02-10
 */

/**
 * Returns the HTTPClient object that owns this channel.
 */
INLINE HTTPClient *HTTPChannel::
get_client() const {
  return _client;
}

/**
 * Returns true if the last-requested document was successfully retrieved and
 * is ready to be read, false otherwise.
 */
INLINE bool HTTPChannel::
is_valid() const {
  return get_status_code() / 100 == 2;
}

/**
 * Returns the URL that was used to retrieve the most recent document:
 * whatever URL was last passed to get_document() or get_header().  If a
 * redirect has transparently occurred, this will return the new, redirected
 * URL (the actual URL at which the document was located).
 */
INLINE const URLSpec &HTTPChannel::
get_url() const {
  return _document_spec.get_url();
}

/**
 * Returns the DocumentSpec associated with the most recent document.  This
 * includes its actual URL (following redirects) along with the identity tag
 * and last-modified date, if supplied by the server.
 *
 * This structure may be saved and used to retrieve the same version of the
 * document later, or to conditionally retrieve a newer version if it is
 * available.
 */
INLINE const DocumentSpec &HTTPChannel::
get_document_spec() const {
  return _document_spec;
}

/**
 * Returns the HTML return code from the document retrieval request.  This
 * will be in the 200 range if the document is successfully retrieved, or some
 * other value in the case of an error.
 *
 * Some proxy errors during an https-over-proxy request would return the same
 * status code as a different error that occurred on the host server.  To
 * differentiate these cases, status codes that are returned by the proxy
 * during the CONNECT phase (except code 407) are incremented by 1000.
 */
INLINE int HTTPChannel::
get_status_code() const {
  return _status_entry._status_code;
}

/**
 * Returns the string as returned by the server describing the status code for
 * humans.  This may or may not be meaningful.
 */
INLINE std::string HTTPChannel::
get_status_string() const {
  return _status_entry._status_string;
}

/**
 * If the document failed to connect because of a 401 (Authorization
 * required), this method will return the "realm" returned by the server in
 * which the requested document must be authenticated.  This string may be
 * presented to the user to request an associated username and password (which
 * then should be stored in HTTPClient::set_username()).
 */
INLINE const std::string &HTTPChannel::
get_www_realm() const {
  return _www_realm;
}

/**
 * Specifies the Content-Type header, useful for applications that require
 * different types of content, such as JSON.
 */
INLINE void HTTPChannel::
set_content_type(std::string content_type) {
  _content_type = content_type;
}

/**
 * Returns the value of the Content-Type header.
 */
INLINE std::string HTTPChannel::
get_content_type() const {
  return _content_type;
}

/**
 * This may be called immediately after a call to get_document() or some
 * related function to specify the expected size of the document we are
 * retrieving, if we happen to know.  This is used as the return value to
 * get_file_size() only in the case that the server does not tell us the
 * actual file size.
 */
INLINE void HTTPChannel::
set_expected_file_size(size_t file_size) {
  _expected_file_size = file_size;
  _got_expected_file_size = true;
}

/**
 * Returns true if the size of the file we are currently retrieving was told
 * us by the server and thus is reliably known, or false if the size reported
 * by get_file_size() represents an educated guess (possibly as set by
 * set_expected_file_size(), or as inferred from a chunked transfer encoding
 * in progress).
 */
INLINE bool HTTPChannel::
is_file_size_known() const {
  return _got_file_size;
}

/**
 * Returns the first byte of the file requested by the request.  This will
 * normally be 0 to indicate that the file is being requested from the
 * beginning, but if the file was requested via a get_subdocument() call, this
 * will contain the first_byte parameter from that call.
 */
INLINE size_t HTTPChannel::
get_first_byte_requested() const {
  return _first_byte_requested;
}

/**
 * Returns the last byte of the file requested by the request.  This will
 * normally be 0 to indicate that the file is being requested to its last
 * byte, but if the file was requested via a get_subdocument() call, this will
 * contain the last_byte parameter from that call.
 */
INLINE size_t HTTPChannel::
get_last_byte_requested() const {
  return _last_byte_requested;
}

/**
 * Returns the first byte of the file (that will be) delivered by the server
 * in response to the current request.  Normally, this is the same as
 * get_first_byte_requested(), but some servers will ignore a subdocument
 * request and always return the whole file, in which case this value will be
 * 0, regardless of what was requested to get_subdocument().
 */
INLINE size_t HTTPChannel::
get_first_byte_delivered() const {
  return _first_byte_delivered;
}

/**
 * Returns the last byte of the file (that will be) delivered by the server in
 * response to the current request.  Normally, this is the same as
 * get_last_byte_requested(), but some servers will ignore a subdocument
 * request and always return the whole file, in which case this value will be
 * 0, regardless of what was requested to get_subdocument().
 */
INLINE size_t HTTPChannel::
get_last_byte_delivered() const {
  return _last_byte_delivered;
}

/**
 * Stops whatever file transaction is currently in progress, closes the
 * connection, and resets to begin anew.  You shouldn't ever need to call
 * this, since the channel should be able to reset itself cleanly between
 * requests, but it is provided in case you are an especially nervous type.
 *
 * Don't call this after every request unless you set
 * set_persistent_connection() to false, since calling reset() rudely closes
 * the connection regardless of whether we have told the server we intend to
 * keep it open or not.
 */
INLINE void HTTPChannel::
reset() {
  reset_for_new_request();
  _status_list.clear();
}

/**
 * Preserves the previous status code (presumably a failure) from the previous
 * connection attempt.  If the subsequent connection attempt also fails, the
 * returned status code will be the better of the previous code and the
 * current code.
 *
 * This can be called to daisy-chain subsequent attempts to download the same
 * document from different servers.  After all servers have been attempted,
 * the final status code will reflect the attempt that most nearly succeeded.
 */
INLINE void HTTPChannel::
preserve_status() {
  _status_list.push_back(_status_entry);
}

/**
 * Resets the extra headers that were previously added via calls to
 * send_extra_header().
 */
INLINE void HTTPChannel::
clear_extra_headers() {
  _send_extra_headers.clear();
}

/**
 * Specifies an additional key: value pair that is added into the header sent
 * to the server with the next request.  This is passed along with no
 * interpretation by the HTTPChannel code.  You may call this repeatedly to
 * append multiple headers.
 *
 * This is persistent for one request only; it must be set again for each new
 * request.
 */
INLINE void HTTPChannel::
send_extra_header(const std::string &key, const std::string &value) {
  _send_extra_headers.push_back(std::make_pair(key, value));
}

/**
 * Opens the named document for reading, if available.  Returns true if
 * successful, false otherwise.
 */
INLINE bool HTTPChannel::
get_document(const DocumentSpec &url) {
  if (!begin_request(HTTPEnum::M_get, url, std::string(), false, 0, 0)) {
    return false;
  }
  while (run()) {
  }
  return is_valid();
}

/**
 * Retrieves only the specified byte range of the indicated document.  If
 * last_byte is 0, it stands for the last byte of the document.  When a
 * subdocument is requested, get_file_size() and get_bytes_downloaded() will
 * report the number of bytes of the subdocument, not of the complete
 * document.
 */
INLINE bool HTTPChannel::
get_subdocument(const DocumentSpec &url, size_t first_byte, size_t last_byte) {
  if (!begin_request(HTTPEnum::M_get, url, std::string(), false, first_byte, last_byte)) {
    return false;
  }
  while (run()) {
  }
  return is_valid();
}

/**
 * Like get_document(), except only the header associated with the document is
 * retrieved.  This may be used to test for existence of the document; it
 * might also return the size of the document (if the server gives us this
 * information).
 */
INLINE bool HTTPChannel::
get_header(const DocumentSpec &url) {
  if (!begin_request(HTTPEnum::M_head, url, std::string(), false, 0, 0)) {
    return false;
  }
  while (run()) {
  }
  return is_valid();
}

/**
 * Posts form data to a particular URL and retrieves the response.
 */
INLINE bool HTTPChannel::
post_form(const DocumentSpec &url, const std::string &body) {
  if (!begin_request(HTTPEnum::M_post, url, body, false, 0, 0)) {
    return false;
  }
  while (run()) {
  }
  return is_valid();
}

/**
 * Uploads the indicated body to the server to replace the indicated URL, if
 * the server allows this.
 */
INLINE bool HTTPChannel::
put_document(const DocumentSpec &url, const std::string &body) {
  if (!begin_request(HTTPEnum::M_put, url, body, false, 0, 0)) {
    return false;
  }
  while (run()) {
  }
  return is_valid();
}

/**
 * Requests the server to remove the indicated URL.
 */
INLINE bool HTTPChannel::
delete_document(const DocumentSpec &url) {
  if (!begin_request(HTTPEnum::M_delete, url, std::string(), false, 0, 0)) {
    return false;
  }
  while (run()) {
  }
  return is_valid();
}

/**
 * Sends an OPTIONS message to the server, which should query the available
 * options, possibly in relation to a specified URL.
 */
INLINE bool HTTPChannel::
get_options(const DocumentSpec &url) {
  if (!begin_request(HTTPEnum::M_options, url, std::string(), false, 0, 0)) {
    return false;
  }
  while (run()) {
  }
  return is_valid();
}

/**
 * Begins a non-blocking request to retrieve a given document.  This method
 * will return immediately, even before a connection to the server has
 * necessarily been established; you must then call run() from time to time
 * until the return value of run() is false.  Then you may check is_valid()
 * and get_status_code() to determine the status of your request.
 *
 * If a previous request had been pending, that request is discarded.
 */
INLINE void HTTPChannel::
begin_get_document(const DocumentSpec &url) {
  begin_request(HTTPEnum::M_get, url, std::string(), true, 0, 0);
}

/**
 * Begins a non-blocking request to retrieve only the specified byte range of
 * the indicated document.  If last_byte is 0, it stands for the last byte of
 * the document.  When a subdocument is requested, get_file_size() and
 * get_bytes_downloaded() will report the number of bytes of the subdocument,
 * not of the complete document.
 */
INLINE void HTTPChannel::
begin_get_subdocument(const DocumentSpec &url, size_t first_byte,
                      size_t last_byte) {
  begin_request(HTTPEnum::M_get, url, std::string(), true, first_byte, last_byte);
}

/**
 * Begins a non-blocking request to retrieve a given header.  See
 * begin_get_document() and get_header().
 */
INLINE void HTTPChannel::
begin_get_header(const DocumentSpec &url) {
  begin_request(HTTPEnum::M_head, url, std::string(), true, 0, 0);
}

/**
 * Posts form data to a particular URL and retrieves the response, all using
 * non-blocking I/O.  See begin_get_document() and post_form().
 *
 * It is important to note that you *must* call run() repeatedly after calling
 * this method until run() returns false, and you may not call any other
 * document posting or retrieving methods using the HTTPChannel object in the
 * interim, or your form data may not get posted.
 */
INLINE void HTTPChannel::
begin_post_form(const DocumentSpec &url, const std::string &body) {
  begin_request(HTTPEnum::M_post, url, body, true, 0, 0);
}

/**
 * Returns the number of bytes downloaded during the last (or current)
 * download_to_file() or download_to_ram operation().  This can be used in
 * conjunction with get_file_size() to report the percent complete (but be
 * careful, since get_file_size() may return 0 if the server has not told us
 * the size of the file).
 */
INLINE size_t HTTPChannel::
get_bytes_downloaded() const {
  return _bytes_downloaded;
}

/**
 * When download throttling is in effect (set_download_throttle() has been set
 * to true) and non-blocking I/O methods (like begin_get_document()) are used,
 * this returns the number of bytes "requested" from the server so far: that
 * is, the theoretical maximum value for get_bytes_downloaded(), if the server
 * has been keeping up with our demand.
 *
 * If this number is less than get_bytes_downloaded(), then the server has not
 * been supplying bytes fast enough to meet our own download throttle rate.
 *
 * When download throttling is not in effect, or when the blocking I/O methods
 * (like get_document(), etc.) are used, this returns 0.
 */
INLINE size_t HTTPChannel::
get_bytes_requested() const {
  return _bytes_requested;
}

/**
 * Returns true when a download_to() or download_to_ram() has executed and the
 * file has been fully downloaded.  If this still returns false after
 * processing has completed, there was an error in transmission.
 *
 * Note that simply testing is_download_complete() does not prove that the
 * requested document was successfully retrieved--you might have just
 * downloaded the "404 not found" stub (for instance) that a server would
 * provide in response to some error condition.  You should also check
 * is_valid() to prove that the file you expected has been successfully
 * retrieved.
 */
INLINE bool HTTPChannel::
is_download_complete() const {
  if (_download_dest != DD_none) {
    return get_state() == S_done;
  }
  return false;
}

/**
 *
 */
INLINE HTTPChannel::StatusEntry::
StatusEntry() {
  _status_code = SC_incomplete;
}
