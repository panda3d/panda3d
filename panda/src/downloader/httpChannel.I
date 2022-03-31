/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpChannel.I
 * @author drose
 * @date 2002-09-24
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
  return (_state != S_failure && (get_status_code() / 100) == 2 &&
          (_server_response_has_no_body || !_source.is_null()));
}

/**
 * Returns true if a connection has been established to the named server in a
 * previous call to connect_to() or begin_connect_to(), false otherwise.
 */
INLINE bool HTTPChannel::
is_connection_ready() const {
  return (!_source.is_null() && _state == S_ready);
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
 * Returns the HTTP version number returned by the server, as one of the
 * HTTPClient enumerated types, e.g.  HTTPClient::HV_11.
 */
INLINE HTTPEnum::HTTPVersion HTTPChannel::
get_http_version() const {
  return _http_version;
}

/**
 * Returns the HTTP version number returned by the server, formatted as a
 * string, e.g.  "HTTP/1.1".
 */
INLINE const std::string &HTTPChannel::
get_http_version_string() const {
  return _http_version_string;
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
 * If the document failed to connect because of a 407 (Proxy authorization
 * required), this method will return the "realm" returned by the proxy.  This
 * string may be presented to the user to request an associated username and
 * password (which then should be stored in HTTPClient::set_username()).
 */
INLINE const std::string &HTTPChannel::
get_proxy_realm() const {
  return _proxy_realm;
}

/**
 * If the document failed with a redirect code (300 series), this will
 * generally contain the new URL the server wants us to try.  In many cases,
 * the client will automatically follow redirects; if these are successful the
 * client will return a successful code and get_redirect() will return empty,
 * but get_url() will return the new, redirected URL.
 */
INLINE const URLSpec &HTTPChannel::
get_redirect() const {
  return _redirect;
}

/**
 * If the document automatically followed one or more redirects, this will
 * return the number of redirects that were automatically followed.  Use
 * get_redirect_step() to retrieve each URL in sequence.
 */
INLINE int HTTPChannel::
get_num_redirect_steps() const {
  return _redirect_trail.size();
}

/**
 * Use in conjunction with get_num_redirect_steps() to extract the chain of
 * URL's that the channel was automatically redirected through to arrive at
 * the final document.
 */
INLINE const URLSpec &HTTPChannel::
get_redirect_step(int n) const {
  nassertr(n >= 0 && n < (int)_redirect_trail.size(), _redirect_trail[0]);
  return _redirect_trail[n];
}

/**
 * Indicates whether the HTTPChannel should try to keep the connection to the
 * server open and reuse that connection for multiple documents, or whether it
 * should close the connection and open a new one for each request.  Set this
 * true to keep the connections around when possible, false to recycle them.
 *
 * It makes most sense to set this false when the HTTPChannel will be used
 * only once to retrieve a single document, true when you will be using the
 * same HTTPChannel object to retrieve multiple documents.
 */
INLINE void HTTPChannel::
set_persistent_connection(bool persistent_connection) {
  _persistent_connection = persistent_connection;
}

/**
 * Returns whether the HTTPChannel should try to keep the connection to the
 * server open and reuse that connection for multiple documents, or whether it
 * should close the connection and open a new one for each request.  See
 * set_persistent_connection().
 */
INLINE bool HTTPChannel::
get_persistent_connection() const {
  return _persistent_connection;
}

/**
 * If this is true (the normal case), the HTTPClient will be consulted for
 * information about the proxy to be used for each connection via this
 * HTTPChannel.  If this has been set to false by the user, then all
 * connections will be made directly, regardless of the proxy settings
 * indicated on the HTTPClient.
 */
INLINE void HTTPChannel::
set_allow_proxy(bool allow_proxy) {
  _allow_proxy = allow_proxy;
}

/**
 * If this is true (the normal case), the HTTPClient will be consulted for
 * information about the proxy to be used for each connection via this
 * HTTPChannel.  If this has been set to false by the user, then all
 * connections will be made directly, regardless of the proxy settings
 * indicated on the HTTPClient.
 */
INLINE bool HTTPChannel::
get_allow_proxy() const {
  return _allow_proxy;
}

/**
 * Normally, a proxy is itself asked for ordinary URL's, and the proxy decides
 * whether to hand the client a cached version of the document or to contact
 * the server for a fresh version.  The proxy may also modify the headers and
 * transfer encoding on the way.
 *
 * If this is set to true, then instead of asking for URL's from the proxy, we
 * will ask the proxy to open a connection to the server (for instance, on
 * port 80); if the proxy honors this request, then we contact the server
 * directly through this connection to retrieve the document.  If the proxy
 * does not honor the connect request, then the retrieve operation fails.
 *
 * SSL connections (e.g.  https), and connections through a Socks proxy, are
 * always tunneled, regardless of the setting of this flag.
 */
INLINE void HTTPChannel::
set_proxy_tunnel(bool proxy_tunnel) {
  _proxy_tunnel = proxy_tunnel;
}

/**
 * Returns true if connections always tunnel through a proxy, or false (the
 * normal case) if we allow the proxy to serve up documents.  See
 * set_proxy_tunnel().
 */
INLINE bool HTTPChannel::
get_proxy_tunnel() const {
  return _proxy_tunnel;
}

/**
 * Sets the maximum length of time, in seconds, that the channel will wait
 * before giving up on establishing a TCP connection.
 *
 * At present, this is used only for the nonblocking interfaces (e.g.
 * begin_get_document(), begin_connect_to()), but it is used whether
 * set_blocking_connect() is true or false.
 */
INLINE void HTTPChannel::
set_connect_timeout(double connect_timeout) {
  _connect_timeout = connect_timeout;
}

/**
 * Returns the length of time, in seconds, to wait for a new nonblocking
 * socket to connect.  See set_connect_timeout().
 */
INLINE double HTTPChannel::
get_connect_timeout() const {
  return _connect_timeout;
}

/**
 * If this flag is true, a socket connect will block even for nonblocking I/O
 * calls like begin_get_document(), begin_connect_to(), etc.  If false, a
 * socket connect will not block for nonblocking I/O calls, but will block for
 * blocking I/O calls (get_document(), connect_to(), etc.).
 *
 * Setting this true is useful when you want to use non-blocking I/O once you
 * have established the connection, but you don't want to bother with polling
 * for the initial connection.  It's also useful when you don't particularly
 * care about non-blocking I/O, but you need to respect timeouts like
 * connect_timeout and http_timeout.
 */
INLINE void HTTPChannel::
set_blocking_connect(bool blocking_connect) {
  _blocking_connect = blocking_connect;
}

/**
 * If this flag is true, a socket connect will block even for nonblocking I/O
 * calls like begin_get_document(), begin_connect_to(), etc.  If false, a
 * socket connect will not block for nonblocking I/O calls, but will block for
 * blocking I/O calls (get_document(), connect_to(), etc.).
 */
INLINE bool HTTPChannel::
get_blocking_connect() const {
  return _blocking_connect;
}

/**
 * Sets the maximum length of time, in seconds, that the channel will wait for
 * the HTTP server to finish sending its response to our request.
 *
 * The timer starts counting after the TCP connection has been established
 * (see set_connect_timeout(), above) and the request has been sent.
 *
 * At present, this is used only for the nonblocking interfaces (e.g.
 * begin_get_document(), begin_connect_to()), but it is used whether
 * set_blocking_connect() is true or false.
 */
INLINE void HTTPChannel::
set_http_timeout(double http_timeout) {
  _http_timeout = http_timeout;
}

/**
 * Returns the length of time, in seconds, to wait for the HTTP server to
 * respond to our request.  See set_http_timeout().
 */
INLINE double HTTPChannel::
get_http_timeout() const {
  return _http_timeout;
}

/**
 * Specifies the maximum number of bytes in a received (but unwanted) body
 * that will be skipped past, in order to reset to a new request.
 *
 * That is, if this HTTPChannel requests a file via get_document(), but does
 * not call download_to_ram(), download_to_file(), or open_read_body(), and
 * instead immediately requests a new file, then the HTTPChannel has a choice
 * whether to skip past the unwanted document, or to close the connection and
 * open a new one.  If the number of bytes to skip is more than this
 * threshold, the connection will be closed; otherwise, the data will simply
 * be read and discarded.
 */
INLINE void HTTPChannel::
set_skip_body_size(size_t skip_body_size) {
  _skip_body_size = skip_body_size;
}

/**
 * Returns the maximum number of bytes in a received (but unwanted) body that
 * will be skipped past, in order to reset to a new request.  See
 * set_skip_body_size().
 */
INLINE size_t HTTPChannel::
get_skip_body_size() const {
  return _skip_body_size;
}

/**
 * Specifies the amount of time, in seconds, in which a previously-established
 * connection is allowed to remain open and unused.  If a previous connection
 * has remained unused for at least this number of seconds, it will be closed
 * and a new connection will be opened; otherwise, the same connection will be
 * reused for the next request (for this particular HTTPChannel).
 */
INLINE void HTTPChannel::
set_idle_timeout(double idle_timeout) {
  _idle_timeout = idle_timeout;
}

/**
 * Returns the amount of time, in seconds, in which an previously-established
 * connection is allowed to remain open and unused.  See set_idle_timeout().
 */
INLINE double HTTPChannel::
get_idle_timeout() const {
  return _idle_timeout;
}

/**
 * Specifies whether nonblocking downloads (via download_to_file() or
 * download_to_ram()) will be limited so as not to use all available
 * bandwidth.
 *
 * If this is true, when a download has been started on this channel it will
 * be invoked no more frequently than get_max_updates_per_second(), and the
 * total bandwidth used by the download will be no more than
 * get_max_bytes_per_second().  If this is false, downloads will proceed as
 * fast as the server can send the data.
 *
 * This only has effect on the nonblocking I/O methods like
 * begin_get_document(), etc.  The blocking methods like get_document() always
 * use as much CPU and bandwidth as they can get.
 */
INLINE void HTTPChannel::
set_download_throttle(bool download_throttle) {
  _download_throttle = download_throttle;
}

/**
 * Returns whether the nonblocking downloads will be bandwidth-limited.  See
 * set_download_throttle().
 */
INLINE bool HTTPChannel::
get_download_throttle() const {
  return _download_throttle;
}

/**
 * When bandwidth throttling is in effect (see set_download_throttle()), this
 * specifies the maximum number of bytes per second that may be consumed by
 * this channel.
 */
INLINE void HTTPChannel::
set_max_bytes_per_second(double max_bytes_per_second) {
  _max_bytes_per_second = max_bytes_per_second;
  _bytes_per_update = int(_max_bytes_per_second * _seconds_per_update);
}

/**
 * Returns the maximum number of bytes per second that may be consumed by this
 * channel when get_download_throttle() is true.
 */
INLINE double HTTPChannel::
get_max_bytes_per_second() const {
  return _max_bytes_per_second;
}

/**
 * When bandwidth throttling is in effect (see set_download_throttle()), this
 * specifies the maximum number of times per second that run() will attempt to
 * do any downloading at all.
 */
INLINE void HTTPChannel::
set_max_updates_per_second(double max_updates_per_second) {
  nassertv(max_updates_per_second != 0.0f);
  _max_updates_per_second = max_updates_per_second;
  _seconds_per_update = 1.0f / _max_updates_per_second;
  _bytes_per_update = int(_max_bytes_per_second * _seconds_per_update);
}

/**
 * Returns the maximum number of times per second that run() will do anything
 * at all, when get_download_throttle() is true.
 */
INLINE double HTTPChannel::
get_max_updates_per_second() const {
  return _max_updates_per_second;
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
  reset_to_new();
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
  _send_extra_headers = std::string();
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
  _send_extra_headers += key;
  _send_extra_headers += ": ";
  _send_extra_headers += value;
  _send_extra_headers += "\r\n";
}

/**
 * Opens the named document for reading, if available.  Returns true if
 * successful, false otherwise.
 */
INLINE bool HTTPChannel::
get_document(const DocumentSpec &url) {
  begin_request(HTTPEnum::M_get, url, std::string(), false, 0, 0);
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
  begin_request(HTTPEnum::M_get, url, std::string(), false, first_byte, last_byte);
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
  begin_request(HTTPEnum::M_head, url, std::string(), false, 0, 0);
  while (run()) {
  }
  return is_valid();
}

/**
 * Posts form data to a particular URL and retrieves the response.
 */
INLINE bool HTTPChannel::
post_form(const DocumentSpec &url, const std::string &body) {
  begin_request(HTTPEnum::M_post, url, body, false, 0, 0);
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
  begin_request(HTTPEnum::M_put, url, body, false, 0, 0);
  while (run()) {
  }
  return is_valid();
}

/**
 * Requests the server to remove the indicated URL.
 */
INLINE bool HTTPChannel::
delete_document(const DocumentSpec &url) {
  begin_request(HTTPEnum::M_delete, url, std::string(), false, 0, 0);
  while (run()) {
  }
  return is_valid();
}

/**
 * Sends a TRACE message to the server, which should return back the same
 * message as the server received it, allowing inspection of proxy hops, etc.
 */
INLINE bool HTTPChannel::
get_trace(const DocumentSpec &url) {
  begin_request(HTTPEnum::M_trace, url, std::string(), false, 0, 0);
  while (run()) {
  }
  return is_valid();
}

/**
 * Establish a direct connection to the server and port indicated by the URL,
 * but do not issue any HTTP requests.  If successful, the connection may then
 * be taken to use for whatever purposes you like by calling get_connection().
 *
 * This establishes a blocking I/O socket.  Also see begin_connect_to().
 */
INLINE bool HTTPChannel::
connect_to(const DocumentSpec &url) {
  begin_request(HTTPEnum::M_connect, url, std::string(), false, 0, 0);
  while (run()) {
  }
  return is_connection_ready();
}

/**
 * Sends an OPTIONS message to the server, which should query the available
 * options, possibly in relation to a specified URL.
 */
INLINE bool HTTPChannel::
get_options(const DocumentSpec &url) {
  begin_request(HTTPEnum::M_options, url, std::string(), false, 0, 0);
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
 * Begins a non-blocking request to establish a direct connection to the
 * server and port indicated by the URL.  No HTTP requests will be issued
 * beyond what is necessary to establish the connection.  When run() has
 * finished, you may call is_connection_ready() to determine if the connection
 * was successfully established.
 *
 * If successful, the connection may then be taken to use for whatever
 * purposes you like by calling get_connection().
 *
 * This establishes a nonblocking I/O socket.  Also see connect_to().
 */
INLINE void HTTPChannel::
begin_connect_to(const DocumentSpec &url) {
  begin_request(HTTPEnum::M_connect, url, std::string(), true, 0, 0);
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
  return (_download_dest != DD_none &&
          (_state == S_read_body || _state == S_read_trailer));
}

/**
 *
 */
INLINE HTTPChannel::StatusEntry::
StatusEntry() {
  _status_code = SC_incomplete;
}
