/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpChannel.cxx
 * @author drose
 * @date 2002-09-24
 */

#include "httpChannel.h"
#include "httpClient.h"
#include "httpCookie.h"
#include "bioStream.h"
#include "chunkedStream.h"
#include "identityStream.h"
#include "config_downloader.h"
#include "virtualFileSystem.h"
#include "virtualFileMountHTTP.h"
#include "ramfile.h"
#include "globPattern.h"

#include <stdio.h>

#ifdef HAVE_OPENSSL

#include "openSSLWrapper.h"

#if defined(WIN32_VC) || defined(WIN64_VC)
  #include <WinSock2.h>
  #include <windows.h>  // for select()
  #undef X509_NAME
#endif  // WIN32_VC

using std::istream;
using std::min;
using std::ostream;
using std::ostringstream;
using std::string;

TypeHandle HTTPChannel::_type_handle;

#define _NOTIFY_HTTP_CHANNEL_ID   "[" << this << "] "

/**
 *
 */
HTTPChannel::
HTTPChannel(HTTPClient *client) :
  _client(client)
{
  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
    << "created.\n";
  }

  ConfigVariableDouble extra_ssl_handshake_time
    ("extra-ssl-handshake-time", 0.0,
     PRC_DESC("This specifies how much extra time to try to establish"
               "the ssl handshake before we bail."));
  _extra_ssl_handshake_time = extra_ssl_handshake_time;
  _proxy_next_index = 0;
  _persistent_connection = false;
  _allow_proxy = true;
  _proxy_tunnel = http_proxy_tunnel;
  _connect_timeout = http_connect_timeout;
  _http_timeout = http_timeout;
  _skip_body_size = http_skip_body_size;
  _idle_timeout = http_idle_timeout;
  _blocking_connect = false;
  _download_throttle = download_throttle;
  _max_bytes_per_second = downloader_byte_rate;
  _seconds_per_update = downloader_frequency;
  _max_updates_per_second = 1.0f / _seconds_per_update;
  _bytes_per_update = int(_max_bytes_per_second * _seconds_per_update);

  // _nonblocking is true if the socket is actually in non-blocking mode.
  _nonblocking = false;

  // _wanted_nonblocking is true if the user specifically requested one of the
  // non-blocking interfaces.  It is false if the socket is only incidentally
  // non-blocking (for instance, because SIMPLE_THREADS is on).
  _wanted_nonblocking = false;

  _want_ssl = false;
  _proxy_serves_document = false;
  _proxy_tunnel_now = false;
  _first_byte_requested = 0;
  _last_byte_requested = 0;
  _first_byte_delivered = 0;
  _last_byte_delivered = 0;
  _read_index = 0;
  _expected_file_size = 0;
  _file_size = 0;
  _transfer_file_size = 0;
  _got_expected_file_size = false;
  _got_file_size = false;
  _got_transfer_file_size = false;
  _bytes_downloaded = 0;
  _bytes_requested = 0;
  _status_entry = StatusEntry();
  _response_type = RT_none;
  _http_version = _client->get_http_version();
  _http_version_string = _client->get_http_version_string();
  _content_type = "application/x-www-form-urlencoded";
  _state = S_new;
  _done_state = S_new;
  _started_download = false;
  _sent_so_far = 0;
  _body_stream = nullptr;
  _owns_body_stream = false;
  _sbio = nullptr;
  _cipher_list = _client->get_cipher_list();
  _last_status_code = 0;
  _last_run_time = 0.0f;
  _download_to_ramfile = nullptr;
  _download_to_stream = nullptr;
}

/**
 *
 */
HTTPChannel::
~HTTPChannel() {
  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
    << "destroyed.\n";
  }

  close_connection();
  reset_download_to();
}

/**
 * Returns the string as returned by the server describing the status code for
 * humans.  This may or may not be meaningful.
 */
string HTTPChannel::
get_status_string() const {
  switch (_status_entry._status_code) {
  case SC_incomplete:
    return "Connection in progress";

  case SC_internal_error:
    return "Internal error";

  case SC_no_connection:
    return "No connection";

  case SC_timeout:
    return "Timeout on connection";

  case SC_lost_connection:
    return "Lost connection";

  case SC_non_http_response:
    return "Non-HTTP response";

  case SC_invalid_http:
    return "Could not understand HTTP response";

  case SC_socks_invalid_version:
    return "Unsupported SOCKS version";

  case SC_socks_no_acceptable_login_method:
    return "No acceptable SOCKS login method";

  case SC_socks_refused:
    return "SOCKS proxy refused connection";

  case SC_socks_no_connection:
    return "SOCKS proxy unable to connect";

  case SC_ssl_internal_failure:
    return "SSL internal failure";

  case SC_ssl_no_handshake:
    return "No SSL handshake";

  case SC_http_error_watermark:
    // This shouldn't be triggered.
    return "Internal error";

  case SC_ssl_invalid_server_certificate:
    return "SSL invalid server certificate";

  case SC_ssl_unexpected_server:
    return "Unexpected SSL server";

  case SC_download_open_error:
    return "Error opening file";

  case SC_download_write_error:
    return "Error writing to disk";

  case SC_download_invalid_range:
    return "Invalid subrange requested";
  }

  return _status_entry._status_string;
}

/**
 * Returns the HTML header value associated with the indicated key, or empty
 * string if the key was not defined in the message returned by the server.
 */
string HTTPChannel::
get_header_value(const string &key) const {
  Headers::const_iterator hi = _headers.find(downcase(key));
  if (hi != _headers.end()) {
    return (*hi).second;
  }
  return string();
}

/**
 * Returns true if the server has indicated it will close the connection after
 * this document has been read, or false if it will remain open (and future
 * documents may be requested on the same connection).
 */
bool HTTPChannel::
will_close_connection() const {
  if (get_http_version() < HTTPEnum::HV_11) {
    // pre-HTTP 1.1 always closes.
    return true;
  }

  string connection = get_header_value("Connection");
  if (downcase(connection) == "close") {
    // The server says it will close.
    return true;
  }

  if (connection.empty() && !get_persistent_connection()) {
    // The server didn't say, but we asked it to close.
    return true;
  }

  // Assume the server will keep it open.
  return false;
}

/**
 * Returns the size of the file, if it is known.  Returns the value set by
 * set_expected_file_size() if the file size is not known, or 0 if this value
 * was not set.
 *
 * If the file is dynamically generated, the size may not be available until a
 * read has started (e.g.  open_read_body() has been called); and even then it
 * may increase as more of the file is read due to the nature of HTTP/1.1
 * requests which can change their minds midstream about how much data they're
 * sending you.
 */
std::streamsize HTTPChannel::
get_file_size() const {
  if (_got_file_size) {
    return _file_size;
  } else if (_got_transfer_file_size) {
    return _transfer_file_size;
  } else if (_got_expected_file_size) {
    return _expected_file_size;
  } else {
    return 0;
  }
}

/**
 * Outputs a list of all headers defined by the server to the indicated output
 * stream.
 */
void HTTPChannel::
write_headers(ostream &out) const {
  Headers::const_iterator hi;
  for (hi = _headers.begin(); hi != _headers.end(); ++hi) {
    out << (*hi).first << ": " << (*hi).second << "\n";
  }
}

/**
 * This must be called from time to time when non-blocking I/O is in use.  It
 * checks for data coming in on the socket and writes data out to the socket
 * when possible, and does whatever processing is required towards completing
 * the current task.
 *
 * The return value is true if the task is still pending (and run() will need
 * to be called again in the future), or false if the current task is
 * complete.
 */
bool HTTPChannel::
run() {
  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "run().\n";
  }

  if (_state == _done_state || _state == S_failure) {
    clear_extra_headers();
    if (!reached_done_state()) {
      return false;
    }
  }

  if (_started_download) {
    if (_wanted_nonblocking && _download_throttle) {
      double now = TrueClock::get_global_ptr()->get_short_time();
      double elapsed = now - _last_run_time;
      if (elapsed < _seconds_per_update) {
        // Come back later.
        thread_yield();
        return true;
      }
      int num_potential_updates = (int)(elapsed / _seconds_per_update);
      _last_run_time = now;
      _bytes_requested += _bytes_per_update * num_potential_updates;
      if (downloader_cat.is_spam()) {
        downloader_cat.spam()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "elapsed = " << elapsed << " num_potential_updates = "
          << num_potential_updates << " bytes_requested = "
          << _bytes_requested << "\n";
      }
    }

    bool repeat_later = false;
    switch (_download_dest) {
    case DD_none:
      // We're done.
      break;

    case DD_file:
      repeat_later = run_download_to_file();
      break;

    case DD_ram:
      repeat_later = run_download_to_ram();
      break;

    case DD_stream:
      repeat_later = run_download_to_stream();
      break;
    }
    if (repeat_later) {
      thread_yield();
    }
    return repeat_later;
  }

  /*
  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "begin run(), _state = " << _state << ", _done_state = "
      << _done_state << "\n";
  }
  */

  if (_state == _done_state) {
    return reached_done_state();
  }

  bool repeat_later;
  do {
    // If we're in a state that expects to have a connection already (that is,
    // any state other that S_try_next_proxy), then reestablish the connection
    // if it has been dropped.
    if (_bio.is_null() && _state != S_try_next_proxy) {
      if (_connect_count > http_max_connect_count) {
        // Too many connection attempts; just give up.  We should never
        // trigger this failsafe, since the code in each individual case has
        // similar logic to prevent more than two consecutive lost
        // connections.
        downloader_cat.warning()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Too many lost connections, giving up.\n";
        _status_entry._status_code = SC_lost_connection;
        _state = S_failure;
        return false;
      }

      // No connection.  Attempt to establish one.
      URLSpec url;
      if (_proxy.empty()) {
        url = _request.get_url();
      } else {
        url = _proxy;
      }
      _bio = new BioPtr(url);
      _source = new BioStreamPtr(new BioStream(_bio));
      if (_nonblocking) {
        _bio->set_nbio(true);
      }

      if (downloader_cat.is_debug()) {
        if (_connect_count > 0) {
          downloader_cat.debug()
            << _NOTIFY_HTTP_CHANNEL_ID
            << "Reconnecting to " << _bio->get_server_name() << " port "
            << _bio->get_port() << "\n";
        } else {
          downloader_cat.debug()
            << _NOTIFY_HTTP_CHANNEL_ID
            << "Connecting to " << _bio->get_server_name() << " port "
            << _bio->get_port() << "\n";
        }
      }

      _state = S_connecting;
      _started_connecting_time =
        TrueClock::get_global_ptr()->get_short_time();
      _connect_count++;
    }

    /*
    if (downloader_cat.is_spam()) {
      downloader_cat.spam()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "continue run(), _state = " << _state << "\n";
    }
    */

    switch (_state) {
    case S_try_next_proxy:
      repeat_later = run_try_next_proxy();
      break;

    case S_connecting:
      repeat_later = run_connecting();
      break;

    case S_connecting_wait:
      repeat_later = run_connecting_wait();
      break;

    case S_http_proxy_ready:
      repeat_later = run_http_proxy_ready();
      break;

    case S_http_proxy_request_sent:
      repeat_later = run_http_proxy_request_sent();
      break;

    case S_http_proxy_reading_header:
      repeat_later = run_http_proxy_reading_header();
      break;

    case S_socks_proxy_greet:
      repeat_later = run_socks_proxy_greet();
      break;

    case S_socks_proxy_greet_reply:
      repeat_later = run_socks_proxy_greet_reply();
      break;

    case S_socks_proxy_connect:
      repeat_later = run_socks_proxy_connect();
      break;

    case S_socks_proxy_connect_reply:
      repeat_later = run_socks_proxy_connect_reply();
      break;

    case S_setup_ssl:
      repeat_later = run_setup_ssl();
      break;

    case S_ssl_handshake:
      repeat_later = run_ssl_handshake();
      break;

    case S_ready:
      repeat_later = run_ready();
      break;

    case S_request_sent:
      repeat_later = run_request_sent();
      break;

    case S_reading_header:
      repeat_later = run_reading_header();
      break;

    case S_start_direct_file_read:
      repeat_later = run_start_direct_file_read();
      break;

    case S_read_header:
      repeat_later = run_read_header();
      break;

    case S_begin_body:
      repeat_later = run_begin_body();
      break;

    case S_reading_body:
      repeat_later = run_reading_body();
      break;

    case S_read_body:
      repeat_later = run_read_body();
      break;

    case S_read_trailer:
      repeat_later = run_read_trailer();
      break;

    default:
      downloader_cat.warning()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Unhandled state " << _state << "\n";
      return false;
    }

    if (_state == _done_state || _state == S_failure) {
      clear_extra_headers();
      // We've reached our terminal state.
      return reached_done_state();
    }
    thread_consider_yield();
  } while (!repeat_later || _bio.is_null());

  /*
  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "later run(), _state = " << _state
      << ", _done_state = " << _done_state << "\n";
  }
  */

  thread_yield();
  return true;
}

/**
 * Returns a newly-allocated istream suitable for reading the body of the
 * document.  This may only be called immediately after a call to
 * get_document() or post_form(), or after a call to run() has returned false.
 *
 * Note that, in nonblocking mode, the returned stream may report an early
 * EOF, even before the actual end of file.  When this happens, you should
 * call stream->is_closed() to determine whether you should attempt to read
 * some more later.
 *
 * The user is responsible for passing the returned istream to
 * close_read_body() later.
 */
ISocketStream *HTTPChannel::
open_read_body() {
  reset_body_stream();

  if ((_state != S_read_header && _state != S_begin_body) || _source.is_null()) {
    return nullptr;
  }

  string transfer_coding = downcase(get_header_value("Transfer-Encoding"));

  ISocketStream *result;
  if (transfer_coding == "chunked") {
    // "chunked" transfer encoding.  This means we will have to decode the
    // length of the file as we read it in chunks.  The IChunkedStream does
    // this.
    _state = S_reading_body;
    _read_index++;
    result = new IChunkedStream(_source, this);

  } else {
    // If the transfer encoding is anything else, assume "identity". This is
    // just the literal characters following the header, up until _file_size
    // bytes have been read (if content-length was specified), or till end of
    // file otherwise.
    _state = S_reading_body;
    _read_index++;
    result = new IIdentityStream(_source, this, _got_file_size, _file_size);
  }

  result->_channel = this;
  _body_stream = result;
  _owns_body_stream = false;

  return result;
}

/**
 * Closes a file opened by a previous call to open_read_body().  This really
 * just deletes the istream pointer, but it is recommended to use this
 * interface instead of deleting it explicitly, to help work around compiler
 * issues.
 */
void HTTPChannel::
close_read_body(istream *stream) const {
  if (stream != nullptr) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting the
    // stream pointer does not call the appropriate global delete function;
    // instead apparently calling the system delete function.  So we call the
    // delete function by hand instead.
#if !defined(USE_MEMORY_NOWRAPPERS) && defined(REDEFINE_GLOBAL_OPERATOR_NEW)
    stream->~istream();
    (*global_operator_delete)(stream);
#else
    delete stream;
#endif
  }
}

/**
 * Specifies the name of a file to download the resulting document to.  This
 * should be called immediately after get_document() or begin_get_document()
 * or related functions.
 *
 * In the case of the blocking I/O methods like get_document(), this function
 * will download the entire document to the file and return true if it was
 * successfully downloaded, false otherwise.
 *
 * In the case of non-blocking I/O methods like begin_get_document(), this
 * function simply indicates an intention to download to the indicated file.
 * It returns true if the file can be opened for writing, false otherwise, but
 * the contents will not be completely downloaded until run() has returned
 * false.  At this time, it is possible that a communications error will have
 * left a partial file, so is_download_complete() may be called to test this.
 *
 * If subdocument_resumes is true and the document in question was previously
 * requested as a subdocument (i.e.  get_subdocument() with a first_byte value
 * greater than zero), this will automatically seek to the appropriate byte
 * within the file for writing the output.  In this case, the file must
 * already exist and must have at least first_byte bytes in it.  If
 * subdocument_resumes is false, a subdocument will always be downloaded
 * beginning at the first byte of the file.
 */
bool HTTPChannel::
download_to_file(const Filename &filename, bool subdocument_resumes) {
  reset_download_to();
  _download_to_filename = filename;
  _download_to_filename.set_binary();
  _subdocument_resumes = subdocument_resumes;

  _download_dest = DD_file;

  if (_wanted_nonblocking && _state != S_read_header) {
    // In nonblocking mode, we can't start the download yet; that will be done
    // later as run() is called.
    return true;
  }

  // In normal, blocking mode, go ahead and do the download.
  if (!open_download_file()) {
    reset_download_to();
    return false;
  }

  while (run()) {
  }
  return is_download_complete() && is_valid();
}

/**
 * Specifies a Ramfile object to download the resulting document to.  This
 * should be called immediately after get_document() or begin_get_document()
 * or related functions.
 *
 * In the case of the blocking I/O methods like get_document(), this function
 * will download the entire document to the Ramfile and return true if it was
 * successfully downloaded, false otherwise.
 *
 * In the case of non-blocking I/O methods like begin_get_document(), this
 * function simply indicates an intention to download to the indicated
 * Ramfile.  It returns true if the file can be opened for writing, false
 * otherwise, but the contents will not be completely downloaded until run()
 * has returned false.  At this time, it is possible that a communications
 * error will have left a partial file, so is_download_complete() may be
 * called to test this.
 *
 * If subdocument_resumes is true and the document in question was previously
 * requested as a subdocument (i.e.  get_subdocument() with a first_byte value
 * greater than zero), this will automatically seek to the appropriate byte
 * within the Ramfile for writing the output.  In this case, the Ramfile must
 * already have at least first_byte bytes in it.
 */
bool HTTPChannel::
download_to_ram(Ramfile *ramfile, bool subdocument_resumes) {
  nassertr(ramfile != nullptr, false);
  reset_download_to();
  ramfile->_pos = 0;
  _download_to_ramfile = ramfile;
  _download_dest = DD_ram;
  _subdocument_resumes = (subdocument_resumes && _first_byte_delivered != 0);

  if (_wanted_nonblocking && _state != S_read_header) {
    // In nonblocking mode, we can't start the download yet; that will be done
    // later as run() is called.
    return true;
  }

  // In normal, blocking mode, go ahead and do the download.
  if (!open_download_file()) {
    reset_download_to();
    return false;
  }

  while (run()) {
  }
  return is_download_complete() && is_valid();
}

/**
 * Specifies the name of an ostream to download the resulting document to.
 * This should be called immediately after get_document() or
 * begin_get_document() or related functions.
 *
 * In the case of the blocking I/O methods like get_document(), this function
 * will download the entire document to the file and return true if it was
 * successfully downloaded, false otherwise.
 *
 * In the case of non-blocking I/O methods like begin_get_document(), this
 * function simply indicates an intention to download to the indicated file.
 * It returns true if the file can be opened for writing, false otherwise, but
 * the contents will not be completely downloaded until run() has returned
 * false.  At this time, it is possible that a communications error will have
 * left a partial file, so is_download_complete() may be called to test this.
 *
 * If subdocument_resumes is true and the document in question was previously
 * requested as a subdocument (i.e.  get_subdocument() with a first_byte value
 * greater than zero), this will automatically seek to the appropriate byte
 * within the file for writing the output.  In this case, the file must
 * already exist and must have at least first_byte bytes in it.  If
 * subdocument_resumes is false, a subdocument will always be downloaded
 * beginning at the first byte of the file.
 */
bool HTTPChannel::
download_to_stream(ostream *strm, bool subdocument_resumes) {
  reset_download_to();
  _download_to_stream = strm;
  _download_to_stream->clear();
  _subdocument_resumes = subdocument_resumes;

  _download_dest = DD_stream;

  if (_wanted_nonblocking && _state != S_read_header) {
    // In nonblocking mode, we can't start the download yet; that will be done
    // later as run() is called.
    return true;
  }

  // In normal, blocking mode, go ahead and do the download.
  if (!open_download_file()) {
    reset_download_to();
    return false;
  }

  while (run()) {
  }
  return is_download_complete() && is_valid();
}

/**
 * Returns the connection that was established via a previous call to
 * connect_to() or begin_connect_to(), or NULL if the connection attempt
 * failed or if those methods have not recently been called.
 *
 * This stream has been allocated from the free store.  It is the user's
 * responsibility to delete this pointer when finished with it.
 */
SocketStream *HTTPChannel::
get_connection() {
  if (!is_connection_ready()) {
    return nullptr;
  }

  BioStream *stream = _source->get_stream();
  _source->set_stream(nullptr);

  // We're now passing ownership of the connection to the caller.
  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "passing ownership of connection to caller.\n";
  }
  reset_to_new();

  return stream;
}

/**
 * Returns the input string with all uppercase letters converted to lowercase.
 */
string HTTPChannel::
downcase(const string &s) {
  string result;
  result.reserve(s.size());
  string::const_iterator p;
  for (p = s.begin(); p != s.end(); ++p) {
    result += tolower(*p);
  }
  return result;
}

/**
 * Called by ISocketStream destructor when _body_stream is destructing.
 */
void HTTPChannel::
body_stream_destructs(ISocketStream *stream) {
  if (stream == _body_stream) {
    if (_state == S_reading_body) {
      switch (_body_stream->get_read_state()) {
      case ISocketStream::RS_complete:
        finished_body(false);
        break;

      case ISocketStream::RS_error:
        _state = HTTPChannel::S_failure;
        _status_entry._status_code = HTTPChannel::SC_lost_connection;
        break;

      default:
        break;
      }
    }
    _body_stream = nullptr;
    _owns_body_stream = false;
  }
}


/**
 * Called by run() after it reaches the done state, this simply checks to see
 * if a download was requested, and begins the download if it has been.
 */
bool HTTPChannel::
reached_done_state() {
  /*
  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "terminating run(), _state = " << _state
      << ", _done_state = " << _done_state << "\n";
  }
  */

  if (_state == S_failure) {
    // We had to give up.  Each proxy we tried, in sequence, failed.  But
    // maybe the last attempt didn't give us the most informative response; go
    // back and find the best one.
    if (!_status_list.empty()) {
      _status_list.push_back(_status_entry);
      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Reexamining failure responses.\n";
      }
      size_t best_i = 0;
      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "  " << 0 << ". " << _status_list[0]._status_code << " "
          << _status_list[0]._status_string << "\n";
      }
      for (size_t i = 1; i < _status_list.size(); i++) {
        if (downloader_cat.is_debug()) {
          downloader_cat.debug()
            << _NOTIFY_HTTP_CHANNEL_ID
            << "  " << i << ". " << _status_list[i]._status_code << " "
            << _status_list[i]._status_string << "\n";
        }
        if (more_useful_status_code(_status_list[i]._status_code,
                                    _status_list[best_i]._status_code)) {
          best_i = i;
        }
      }
      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "chose index " << best_i << ", above.\n";
      }
      _status_entry = _status_list[best_i];
      _status_list.clear();
    }

    return false;
  }

  // We don't need the list of previous failures any more--we've connected.
  _status_list.clear();

  if (_download_dest == DD_none) {
    // All done.
    return false;

  } else {
    // Oops, we have to download the body now.
    open_read_body();
    if (_body_stream == nullptr) {
      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Unable to download body: " << _request.get_url() << "\n";
      }
      return false;

    } else {
      _owns_body_stream = true;
      if (_state != S_reading_body) {
        reset_body_stream();
      }
      _started_download = true;

      _done_state = S_read_trailer;
      _last_run_time = TrueClock::get_global_ptr()->get_short_time();
      return true;
    }
  }
}

/**
 * This state is reached when a previous connection attempt fails.  If we have
 * multiple proxies in line to try, it sets us up for the next proxy and tries
 * to connect again; otherwise, it sets the state to S_failure.
 */
bool HTTPChannel::
run_try_next_proxy() {
  if (_proxy_next_index < _proxies.size()) {
    // Record the previous proxy's status entry, so we can come back to it
    // later if we get nonsense from the remaining proxies.
    _status_list.push_back(_status_entry);
    _status_entry = StatusEntry();

    // Now try the next proxy in sequence.
    _proxy = _proxies[_proxy_next_index];
    _proxy_auth = nullptr;
    _proxy_next_index++;
    close_connection();
    reconsider_proxy();
    _state = S_connecting;

    return false;
  }

  // No more proxies to try, or we're not using a proxy.
  _state = S_failure;
  return false;
}

/**
 * In this state, we have not yet established a network connection to the
 * server (or proxy).
 */
bool HTTPChannel::
run_connecting() {
  _status_entry = StatusEntry();

  if (!_bio->connect()) {
    if (_bio->should_retry()) {
      _state = S_connecting_wait;
      return false;
    }
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Could not connect to " << _bio->get_server_name() << " port "
      << _bio->get_port() << "\n";
    OpenSSLWrapper::get_global_ptr()->notify_ssl_errors();
    _status_entry._status_code = SC_no_connection;
    _state = S_try_next_proxy;
    return false;
  }

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Connected to " << _bio->get_server_name() << " port "
      << _bio->get_port() << "\n";
  }

  if (_proxy_tunnel_now) {
    if (_proxy.get_scheme() == "socks") {
      _state = S_socks_proxy_greet;
    } else {
      _state = S_http_proxy_ready;
    }

  } else {
    if (_want_ssl) {
      _state = S_setup_ssl;
    } else {
      _state = S_ready;
    }
  }
  return false;
}

/**
 * Here we have begun to establish a nonblocking connection, but we got a
 * come-back-later message, so we are waiting for the socket to finish
 * connecting.
 */
bool HTTPChannel::
run_connecting_wait() {
  int fd = -1;
  BIO_get_fd(*_bio, &fd);
  if (fd < 0) {
    downloader_cat.warning()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "nonblocking socket BIO has no file descriptor.\n";
    // This shouldn't be possible.
    _status_entry._status_code = SC_internal_error;
    _state = S_try_next_proxy;
    return false;
  }

  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "waiting to connect to " << _request.get_url().get_server_and_port() << ".\n";
  }
  fd_set wset;
  FD_ZERO(&wset);
  FD_SET(fd, &wset);
  struct timeval tv;
  if (get_blocking_connect()) {
    // Since we'll be blocking on this connect, fill in the timeout into the
    // structure.
    tv.tv_sec = (int)_connect_timeout;
    tv.tv_usec = (int)((_connect_timeout - tv.tv_sec) * 1000000.0);
  } else {
    // We won't block on this connect, so select() for 0 time.
    tv.tv_sec = 0;
    tv.tv_usec = 0;
  }
  int errcode = select(fd + 1, nullptr, &wset, nullptr, &tv);
  if (errcode < 0) {
    downloader_cat.warning()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Error in select.\n";
    // This shouldn't be possible.
    _status_entry._status_code = SC_internal_error;
    _state = S_try_next_proxy;
    return false;
  }

  if (errcode == 0) {
    // Nothing's happened so far; come back later.
    if (get_blocking_connect() ||
        (TrueClock::get_global_ptr()->get_short_time() -
         _started_connecting_time > get_connect_timeout())) {
      // Time to give up.
      downloader_cat.info()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Timeout connecting to "
        << _request.get_url().get_server_and_port()
        << " for " << _request.get_url()
        << ".\n";
      _status_entry._status_code = SC_timeout;
      _state = S_try_next_proxy;
      return false;
    }
    return true;
  }

  // The socket is now ready for writing.
  _state = S_connecting;
  return false;
}


/**
 * This state is reached only after first establishing a connection to the
 * proxy, if a proxy is in use and we are tunneling through it via a CONNECT
 * command.
 */
bool HTTPChannel::
run_http_proxy_ready() {
  // If there's a request to be sent to the proxy, send it now.
  nassertr(!_proxy_request_text.empty(), false);
  if (!server_send(_proxy_request_text, false)) {
    return true;
  }

  // All done sending request.
  _state = S_http_proxy_request_sent;
  _sent_request_time = TrueClock::get_global_ptr()->get_short_time();
  return false;
}

/**
 * This state is reached only after we have sent a special message to the
 * proxy and we are waiting for the proxy's response.  It is not used in the
 * normal http-over-proxy case, which does not require a special message to
 * the proxy.
 */
bool HTTPChannel::
run_http_proxy_request_sent() {
  // Wait for the first line to come back from the server.
  string line;
  if (!server_getline_failsafe(line)) {
    return true;
  }

  // Skip unexpected blank lines.  We're just being generous here.
  while (line.empty()) {
    if (!server_getline_failsafe(line)) {
      return true;
    }
  }

  if (!parse_http_response(line)) {
    return false;
  }

  _state = S_http_proxy_reading_header;
  _current_field_name = string();
  _current_field_value = string();
  _headers.clear();
  _got_file_size = false;
  _got_transfer_file_size = false;
  return false;
}

/**
 * In this state we are reading the header lines from the proxy's response to
 * our special message.
 */
bool HTTPChannel::
run_http_proxy_reading_header() {
  if (parse_http_header()) {
    return true;
  }

  _redirect = get_header_value("Location");
  // We can take the proxy's word for it that this is the actual URL for the
  // redirect.

  _server_response_has_no_body =
    (get_status_code() / 100 == 1 ||
     get_status_code() == 204 ||
     get_status_code() == 304);

  int last_status = _last_status_code;
  _last_status_code = get_status_code();

  if (get_status_code() == 407 && last_status != 407 && !_proxy.empty()) {
    // 407: not authorized to proxy.  Try to get the authorization.
    string authenticate_request = get_header_value("Proxy-Authenticate");
    _proxy_auth = _client->generate_auth(_proxy, true, authenticate_request);
    if (_proxy_auth != nullptr) {
      _proxy_realm = _proxy_auth->get_realm();
      _proxy_username = _client->select_username(_proxy, true, _proxy_realm);
      if (!_proxy_username.empty()) {
        make_proxy_request_text();

        // Roll the state forward to force a new request.
        _state = S_begin_body;
        return false;
      }
    }
  }

  if (!is_valid()) {
    // Proxy wouldn't open connection.

    // Change some of the status codes a proxy might return to differentiate
    // them from similar status codes the destination server might have
    // returned.
    if (get_status_code() != 407) {
      _status_entry._status_code += 1000;
    }

    _state = S_try_next_proxy;
    return false;
  }

  // Now we have a tunnel opened through the proxy.
  make_request_text();

  if (_want_ssl) {
    _state = S_setup_ssl;
  } else {
    _state = S_ready;
  }

  return false;
}

/**
 * This state is reached only after first establishing a connection to a SOCKS
 * proxy, with which we now have to negotiate a connection.
 */
bool HTTPChannel::
run_socks_proxy_greet() {
  static const char socks_greeting[] = {
    0x05, // Socks version 5
    0x01, // Number of supported login methods
    0x00, // Login method 0: no authentication
    /*
    0x01, // Login method 1: GSSAPI
    0x02  // Login method 2: username/password
    */
  };
  static const int socks_greeting_len = sizeof(socks_greeting);
  if (!server_send(string(socks_greeting, socks_greeting_len), true)) {
    return true;
  }
  _sent_request_time = TrueClock::get_global_ptr()->get_short_time();

  // All done sending request.
  _state = S_socks_proxy_greet_reply;
  return false;
}

/**
 * We are waiting for the SOCKS proxy to respond to our greeting.
 */
bool HTTPChannel::
run_socks_proxy_greet_reply() {
  string reply;

  // Get the two-byte reply from the SOCKS server.
  if (!server_get_failsafe(reply, 2)) {
    return true;
  }

  if (reply[0] != 0x05) {
    // We only speak Socks5.
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Rejecting Socks version " << (int)reply[0] << "\n";
    _status_entry._status_code = SC_socks_invalid_version;
    _state = S_try_next_proxy;
    return false;
  }

  if (reply[1] == (char)0xff) {
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Socks server does not accept our available login methods.\n";
    _status_entry._status_code = SC_socks_no_acceptable_login_method;
    _state = S_try_next_proxy;
    return false;
  }

  if (reply[1] == 0x00) {
    // No login method required.  Proceed directly to the connect message.
    _state = S_socks_proxy_connect;
    return false;
  }

  // The server accepted a login method we didn't offer!
  downloader_cat.info()
    << _NOTIFY_HTTP_CHANNEL_ID
    << "Socks server accepted unrequested login method "
    << (int)reply[1] << "\n";
  _status_entry._status_code = SC_socks_no_acceptable_login_method;
  _state = S_try_next_proxy;
  return false;
}

/**
 * The SOCKS proxy has accepted us, and now we may issue the connect request.
 */
bool HTTPChannel::
run_socks_proxy_connect() {
  static const char socks_connect[] = {
    0x05, // Socks version 5
    0x01, // Command 1: connect
    0x00, // reserved
    0x03, // DNS name
  };
  static const int socks_connect_len = sizeof(socks_connect);

  string hostname = _request.get_url().get_server();
  int port = _request.get_url().get_port();

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Requesting SOCKS5 connection to "
      << _request.get_url().get_server_and_port() << "\n";
  }

  string connect =
    string(socks_connect, socks_connect_len) +
    string(1, (char)hostname.length()) +
    hostname +
    string(1, (char)((port >> 8) & 0xff)) +
    string(1, (char)(port & 0xff));

  if (!server_send(connect, true)) {
    return true;
  }
  _sent_request_time = TrueClock::get_global_ptr()->get_short_time();

  _state = S_socks_proxy_connect_reply;
  return false;
}

/**
 * We are waiting for the SOCKS proxy to honor our connect request.
 */
bool HTTPChannel::
run_socks_proxy_connect_reply() {
  string reply;

  // Get the first two bytes of the connect reply.
  if (!server_get_failsafe(reply, 2)) {
    return true;
  }

  if (reply[0] != 0x05) {
    // We only speak Socks5.
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Rejecting Socks version " << (int)reply[0] << "\n";
    close_connection();  // connection is now bad.
    _status_entry._status_code = SC_socks_invalid_version;
    _state = S_try_next_proxy;
    return false;
  }

  if (reply[1] != 0x00) {
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Connection refused, SOCKS code " << (int)reply[1] << "\n";
    /*
      Socks error codes (from RFC1928):
             o  X'00' succeeded
             o  X'01' general SOCKS server failure
             o  X'02' connection not allowed by ruleset
             o  X'03' Network unreachable
             o  X'04' Host unreachable
             o  X'05' Connection refused
             o  X'06' TTL expired
             o  X'07' Command not supported
             o  X'08' Address type not supported
             o  X'09' to X'FF' unassigned
    */

    switch (reply[1]) {
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
      // These generally mean the same thing: the SOCKS proxy tried, but
      // couldn't reach the host.
      _status_entry._status_code = SC_socks_no_connection;
      break;

    default:
      _status_entry._status_code = SC_socks_refused;
    }

    close_connection();  // connection is now bad.
    _state = S_try_next_proxy;
    return false;
  }

  // Now put those bytes back, and get five bytes of the reply.
  _working_get = reply;
  if (!server_get_failsafe(reply, 5)) {
    return true;
  }

  // Figure out how many bytes total we will expect for the reply.
  int total_bytes = 6;

  switch (reply[3]) {
  case 0x01:  // IPv4
    total_bytes += 4;
    break;

  case 0x03:  // DNS
    total_bytes += (unsigned int)reply[4];
    break;

  case 0x04:  // IPv6
    total_bytes += 16;
    break;

  default:
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Unsupported SOCKS address type: " << (int)reply[3] << "\n";
    _status_entry._status_code = SC_socks_invalid_version;
    _state = S_try_next_proxy;
    return false;
  }

  // Now put back the bytes we've read so far, and get the rest of them.
  _working_get = reply;
  if (!server_get_failsafe(reply, total_bytes)) {
    return true;
  }

  if (downloader_cat.is_debug()) {
    // Finally, we can decode the whole thing.
    string connect_host;

    switch (reply[3]) {
    case 0x01:  // IPv4
      {
        ostringstream strm;
        strm << (unsigned int)(unsigned char)reply[4] << "."
             << (unsigned int)(unsigned char)reply[5] << "."
             << (unsigned int)(unsigned char)reply[6] << "."
             << (unsigned int)(unsigned char)reply[7];
        connect_host = strm.str();
      }
      break;

    case 0x03:  // DNS
      connect_host = string(&reply[5], (unsigned int)reply[4]);
      break;

    case 0x04:  // IPv6
      {
        char buf[48];
        sprintf(buf, "[%02hhx%02hhx:%02hhx%02hhx:%02hhx%02hhx:%02hhx%02hhx"
                     ":%02hhx%02hhx:%02hhx%02hhx:%02hhx%02hhx:%02hhx%02hhx]",
                reply[4], reply[5], reply[6], reply[7], reply[8], reply[9],
                reply[10], reply[11], reply[12], reply[13], reply[14],
                reply[15], reply[16], reply[17], reply[18], reply[19]);
        total_bytes += 16;
      }
      break;
    }

    int connect_port =
      (((unsigned int)(unsigned char)reply[total_bytes - 2]) << 8) |
      ((unsigned int)(unsigned char)reply[total_bytes - 1]);

    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
      << _proxy << " directed us to " << connect_host << ":"
      << connect_port << "\n";
  }

  if (_want_ssl) {
    _state = S_setup_ssl;
  } else {
    _state = S_ready;
  }

  return false;
}

/**
 * This state begins elevating our existing, unsecure connection to a secure,
 * SSL connection.
 */
bool HTTPChannel::
run_setup_ssl() {
  _sbio = BIO_new_ssl(_client->get_ssl_ctx(), true);
  BIO_push(_sbio, *_bio);

  SSL *ssl = nullptr;
  BIO_get_ssl(_sbio, &ssl);
  nassertr(ssl != nullptr, false);

  // We only take one word at a time from the _cipher_list.  If that
  // connection fails, then we take the next word.
  string cipher_list = _cipher_list;
  if (!cipher_list.empty()) {
    size_t space = cipher_list.find(" ");
    if (space != string::npos) {
      cipher_list = cipher_list.substr(0, space);
    }
  }

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Setting ssl-cipher-list '" << cipher_list << "'\n";
  }
  int result = SSL_set_cipher_list(ssl, cipher_list.c_str());
  if (result == 0) {
    downloader_cat.error()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Invalid cipher list: '" << cipher_list << "'\n";
    OpenSSLWrapper::get_global_ptr()->notify_ssl_errors();
    _status_entry._status_code = SC_ssl_internal_failure;
    _state = S_failure;
    return false;
  }

  string hostname = _request.get_url().get_server();
  result = SSL_set_tlsext_host_name(ssl, hostname.c_str());
  if (result == 0) {
    downloader_cat.error()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Could not set TLS SNI hostname to '" << hostname << "'\n";
  }

/*
 * It would be nice to use something like SSL_set_client_cert_cb() here to set
 * a callback to provide the certificate should it be requested, or even to
 * potentially provide any of a number of certificates according to the
 * server's CA presented, but that interface as provided by OpenSSL is broken
 * since there's no way to pass additional data to the callback function (and
 * hence no way to tie it back to the HTTPChannel object, other than by
 * building a messy mapping of SSL pointers back to HTTPChannel pointers).
 */
  if (_client->load_client_certificate()) {
    SSL_use_certificate(ssl, _client->_client_certificate_pub);
    SSL_use_PrivateKey(ssl, _client->_client_certificate_priv);
    if (!SSL_check_private_key(ssl)) {
      downloader_cat.warning()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Client private key does not match public key!\n";
    }
  }

  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "SSL Ciphers available:\n";
    const char *name;
    int pri = 0;
    name = SSL_get_cipher_list(ssl, pri);
    while (name != nullptr) {
      downloader_cat.spam()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "  " << pri + 1 << ". " << name << "\n";
      pri++;
      name = SSL_get_cipher_list(ssl, pri);
    }
  }

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "performing SSL handshake\n";
  }
  _state = S_ssl_handshake;

  // We start the connect timer over again when we reach the SSL handshake.
  _started_connecting_time =
    TrueClock::get_global_ptr()->get_short_time();

  return false;
}

/**
 * This state performs the SSL handshake with the server, and also verifies
 * the server's identity when the handshake has successfully completed.
 */
bool HTTPChannel::
run_ssl_handshake() {
  if (BIO_do_handshake(_sbio) <= 0) {
    if (BIO_should_retry(_sbio)) {
      double elapsed =
        TrueClock::get_global_ptr()->get_short_time() -
        _started_connecting_time;
      if (elapsed <= get_connect_timeout() + _extra_ssl_handshake_time) {
        // Keep trying.
        return true;
      }
      // Time to give up on the handshake.
    }

    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Could not establish SSL handshake with "
      << _request.get_url().get_server_and_port() << "\n";
    OpenSSLWrapper::get_global_ptr()->notify_ssl_errors();

    // It seems to be an error to free sbio at this point; perhaps it's
    // already been freed?

    if (!_cipher_list.empty()) {
      // If we've got another cipher to try, do so.
      size_t space = _cipher_list.find(" ");
      if (space != string::npos) {
        while (space < _cipher_list.length() && _cipher_list[space] == ' ') {
          ++space;
        }
        _cipher_list = _cipher_list.substr(space);
        if (!_cipher_list.empty()) {
          close_connection();
          reconsider_proxy();
          _state = S_connecting;
          return false;
        }
      }
    }

    // All done trying ciphers; they all failed.
    _cipher_list = _client->get_cipher_list();
    _status_entry._status_code = SC_ssl_no_handshake;
    _state = S_failure;
    return false;
  }

  SSL *ssl = nullptr;
  BIO_get_ssl(_sbio, &ssl);
  nassertr(ssl != nullptr, false);

  if (!_nonblocking) {
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
  }

  const SSL_CIPHER *cipher = SSL_get_current_cipher(ssl);
  if (cipher == nullptr) {
    downloader_cat.warning()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "No current cipher on SSL connection.\n";
  } else {
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Using cipher " << SSL_CIPHER_get_name((SSL_CIPHER *) cipher) << "\n";
    }
  }

  // Now that we've made an SSL handshake, we can use the SSL bio to do all of
  // our communication henceforth.
  _bio->set_bio(_sbio);
  _sbio = nullptr;

  X509 *cert = SSL_get_peer_certificate(ssl);
  if (cert == nullptr) {
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "No certificate was presented by server.\n";

    // This shouldn't be possible, per the SSL specs.
    _status_entry._status_code = SC_ssl_invalid_server_certificate;
    _state = S_failure;
    return false;
  }

  X509_NAME *subject = X509_get_subject_name(cert);
  if (downloader_cat.is_debug()) {
    string org_name = get_x509_name_component(subject, NID_organizationName);
    string org_unit_name = get_x509_name_component(subject, NID_organizationalUnitName);
    string common_name = get_x509_name_component(subject, NID_commonName);

    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Server is " << common_name << " from " << org_unit_name
      << " / " << org_name << "\n";

    if (downloader_cat.is_spam()) {
      downloader_cat.spam()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Received certificate from server:\n" << std::flush;
      X509_print_fp(stderr, cert);
      fflush(stderr);
    }
  }

  bool cert_preapproved = false;
  bool cert_name_preapproved = false;
  check_preapproved_server_certificate(cert, cert_preapproved, cert_name_preapproved);

  // Now verify the server certificate is valid.
  long verify_result = SSL_get_verify_result(ssl);
  bool cert_valid = true;

  if (verify_result == X509_V_ERR_CERT_HAS_EXPIRED) {
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Expired certificate from " << _request.get_url().get_server_and_port() << "\n";
    if (_client->get_verify_ssl() == HTTPClient::VS_normal && !cert_preapproved) {
      cert_valid = false;
    }

  } else if (verify_result == X509_V_ERR_CERT_NOT_YET_VALID) {
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Premature certificate from " << _request.get_url().get_server_and_port() << "\n";
    if (_client->get_verify_ssl() == HTTPClient::VS_normal && !cert_preapproved) {
      cert_valid = false;
    }

  } else if (verify_result == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT ||
             verify_result == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN) {
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Self-signed certificate from " << _request.get_url().get_server_and_port() << "\n";
    if (_client->get_verify_ssl() != HTTPClient::VS_no_verify && !cert_preapproved) {
      cert_valid = false;
    }

  } else if (verify_result != X509_V_OK) {
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Unable to verify identity of " << _request.get_url().get_server_and_port()
      << ", verify error code " << verify_result << "\n";
    if (_client->get_verify_ssl() != HTTPClient::VS_no_verify && !cert_preapproved) {
      cert_valid = false;
    }
  }

  if (!cert_valid) {
    _status_entry._status_code = SC_ssl_invalid_server_certificate;
    _state = S_failure;
    return false;
  }

  if (_client->get_verify_ssl() != HTTPClient::VS_no_verify && !cert_name_preapproved) {
    // Check that the server is someone we expected to be talking to.
    if (!validate_server_name(cert)) {
      _status_entry._status_code = SC_ssl_unexpected_server;
      _state = S_failure;
      return false;
    }
  }

  X509_free(cert);

  _state = S_ready;
  return false;
}

/**
 * This is the main "ready" state.  In this state, we have established a
 * (possibly secure) connection to the server (or proxy), and the server (or
 * proxy) is idle and waiting for us to send a request.
 *
 * If persistent_connection is true, we will generally come back to this state
 * after finishing each request on a given connection.
 */
bool HTTPChannel::
run_ready() {
  // If there's a request to be sent upstream, send it now.
  if (!_request_text.empty()) {
   if (!server_send(_request_text, false)) {
      return true;
    }
  }

  // All done sending request.
  _state = S_request_sent;
  _sent_request_time = TrueClock::get_global_ptr()->get_short_time();
  return false;
}

/**
 * In this state we have sent our request to the server (or proxy) and we are
 * waiting for a response.
 */
bool HTTPChannel::
run_request_sent() {
  // Wait for the first line to come back from the server.
  string line;
  if (!server_getline_failsafe(line)) {
    return true;
  }

  // Skip unexpected blank lines.  We're just being generous here.
  while (line.empty()) {
    if (!server_getline_failsafe(line)) {
      return true;
    }
  }

  if (!parse_http_response(line)) {
    // Not an HTTP response.  _state is already set appropriately.
    return false;
  }

  _state = S_reading_header;
  _current_field_name = string();
  _current_field_value = string();
  _headers.clear();
  _got_file_size = false;
  _got_transfer_file_size = false;
  return false;
}

/**
 * In this state we have received the first response to our request from the
 * server (or proxy) and we are reading the set of header lines preceding the
 * requested document.
 */
bool HTTPChannel::
run_reading_header() {
  if (parse_http_header()) {
    if (_bio.is_null()) {
      downloader_cat.info()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Connection lost while reading HTTP response.\n";
      if (_response_type == RT_http_hangup) {
        // This was our second hangup in a row.  Give up.
        _status_entry._status_code = SC_lost_connection;
        _state = S_try_next_proxy;

      } else {
        // Try again, once.
        _response_type = RT_http_hangup;
      }

    } else {
      double elapsed =
        TrueClock::get_global_ptr()->get_short_time() -
        _sent_request_time;
      if (elapsed > get_http_timeout()) {
        // Time to give up.
        downloader_cat.info()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Timeout waiting for "
          << _request.get_url().get_server_and_port()
          << " in run_reading_header (" << elapsed
          << " seconds elapsed).\n";
        _status_entry._status_code = SC_timeout;
        _state = S_try_next_proxy;
      }
    }
    return true;
  }
  _response_type = RT_http_complete;

  // Ok, we've established an HTTP connection to the server.  Our extra send
  // headers have done their job; clear them for next time.
  clear_extra_headers();

  _server_response_has_no_body =
    (get_status_code() / 100 == 1 ||
     get_status_code() == 204 ||
     get_status_code() == 304 ||
     _method == HTTPEnum::M_head);

  // Look for key properties in the header fields.
  if (get_status_code() == 206) {
    string content_range = get_header_value("Content-Range");
    if (content_range.empty()) {
      downloader_cat.warning()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Got 206 response without Content-Range header!\n";
      _status_entry._status_code = SC_invalid_http;
      _state = S_failure;
      return false;

    } else {
      if (!parse_content_range(content_range)) {
        downloader_cat.warning()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Couldn't parse Content-Range: " << content_range << "\n";
        _status_entry._status_code = SC_invalid_http;
        _state = S_failure;
        return false;
      }
    }

  } else {
    _first_byte_delivered = 0;
    _last_byte_delivered = 0;
  }
  if (downloader_cat.is_debug()) {
    if (_first_byte_requested != 0 || _last_byte_requested != 0 ||
        _first_byte_delivered != 0 || _last_byte_delivered != 0) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Requested byte range " << _first_byte_requested
        << " to " << _last_byte_delivered
        << "; server delivers range " << _first_byte_delivered
        << " to " << _last_byte_delivered
        << "\n";
    }
  }

  // Set the _document_spec to reflect what we just retrieved.
  _document_spec = DocumentSpec(_request.get_url());
  string tag = get_header_value("ETag");
  if (!tag.empty()) {
    _document_spec.set_tag(HTTPEntityTag(tag));
  }
  string date = get_header_value("Last-Modified");
  if (!date.empty()) {
    _document_spec.set_date(HTTPDate(date));
  }

  // In case we've got a download in effect, now we know what the first byte
  // of the subdocument request will be, so we can open the file and position
  // it.
  if (_server_response_has_no_body) {
    // Never mind on the download.
    reset_download_to();
  }

  if (!open_download_file()) {
    return false;
  }

  _got_expected_file_size = false;
  _got_file_size = false;
  _got_transfer_file_size = false;

  string content_length = get_header_value("Content-Length");
  if (!content_length.empty()) {
    _file_size = atoi(content_length.c_str());
    _got_file_size = true;

  } else if (get_status_code() == 206) {
    // Well, we didn't get a content-length from the server, but we can infer
    // the number of bytes based on the range we're given.
    _file_size = _last_byte_delivered - _first_byte_delivered + 1;
    _got_file_size = true;
  }
  _redirect = get_header_value("Location");

  // The server might have given us just a filename for the redirect.  In that
  // case, it's relative to the same server.  If it's a relative path, it's
  // relative to the same directory.
  if (_redirect.has_path() && !_redirect.has_authority()) {
    URLSpec url = _document_spec.get_url();
    Filename path = _redirect.get_path();
    if (path.is_local()) {
      Filename rel_to = Filename(url.get_path()).get_dirname();
      _redirect.set_path(Filename(rel_to, path));
    }
    _redirect.set_scheme(url.get_scheme());
    _redirect.set_authority(url.get_authority());
  }

  _state = S_read_header;

  if (_server_response_has_no_body && will_close_connection()) {
    // If the server said it will close the connection, we should close it
    // too.
    close_connection();
  }

  // Handle automatic retries and redirects.
  int last_status = _last_status_code;
  _last_status_code = get_status_code();

  if (get_status_code() == 407 && last_status != 407 && !_proxy.empty()) {
    // 407: not authorized to proxy.  Try to get the authorization.
    string authenticate_request = get_header_value("Proxy-Authenticate");
    _proxy_auth =
      _client->generate_auth(_proxy, true, authenticate_request);
    if (_proxy_auth != nullptr) {
      _proxy_realm = _proxy_auth->get_realm();
      _proxy_username = _client->select_username(_proxy, true, _proxy_realm);
      if (!_proxy_username.empty()) {
        make_request_text();

        // Roll the state forward to force a new request.
        _state = S_begin_body;
        return false;
      }
    }
  }

  if (get_status_code() == 401 && last_status != 401) {
    // 401: not authorized to remote server.  Try to get the authorization.
    string authenticate_request = get_header_value("WWW-Authenticate");
    _www_auth = _client->generate_auth(_request.get_url(), false, authenticate_request);
    if (_www_auth != nullptr) {
      _www_realm = _www_auth->get_realm();
      _www_username = _client->select_username(_request.get_url(), false, _www_realm);
      if (!_www_username.empty()) {
        make_request_text();

        // Roll the state forward to force a new request.
        _state = S_begin_body;
        return false;
      }
    }
  }

  if ((get_status_code() == 300 ||
       get_status_code() == 301 ||
       get_status_code() == 302 ||
       get_status_code() == 303 ||
       get_status_code() == 307) && !get_redirect().empty()) {
    // Redirect.  Should we handle it automatically?

    // According to the letter of RFC 2616, 301 and 302 responses to POST
    // requests must not be automatically redirected without confirmation by
    // the user.  In reality, browsers do allow automatic redirection of these
    // responses, changing the POST to a GET, and we reproduce this behavior
    // here.
    if (_method == HTTPEnum::M_post) {
      _method = HTTPEnum::M_get;
      _body = string();
    }

    if (_method == HTTPEnum::M_get || _method == HTTPEnum::M_head) {
      // Sure!
      URLSpec new_url = get_redirect();
      if (find(_redirect_trail.begin(), _redirect_trail.end(),
               new_url) != _redirect_trail.end()) {
        downloader_cat.warning()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "cycle detected in redirect to " << new_url << "\n";

      } else {
        _redirect_trail.push_back(new_url);

        if (downloader_cat.is_debug()) {
          downloader_cat.debug()
            << _NOTIFY_HTTP_CHANNEL_ID
            << "following redirect to " << new_url << "\n";
        }
        if (_request.get_url().has_username()) {
          new_url.set_username(_request.get_url().get_username());
        }
        reset_url(_request.get_url(), new_url);
        _request.set_url(new_url);
        _want_ssl = _request.get_url().is_ssl();
        reconsider_proxy();
        make_header();
        make_request_text();

        // Roll the state forward to force a new request.
        _state = S_begin_body;
        return false;
      }
    }
  }

  if (_state == S_read_header &&
      ((get_status_code() / 100) == 4 || (get_status_code() / 100) == 5) &&
      _proxy_serves_document && _proxy_next_index < _proxies.size()) {
    // If we were using a proxy (but not tunneling through the proxy) and we
    // got some kind of a server error, try the next proxy in sequence (if we
    // have one).  This handles the case of a working proxy that cannot see
    // the host (and so returns 504 or something along those lines).  Some
    // proxies are so broken they return a 404 in this case, so we have to
    // consider that along the same lines.
    _state = S_try_next_proxy;
    return false;
  }

  // Otherwise, we're good to go.
  return false;
}

/**
 * This is the first state when reading a file:// URL. All it does is skip
 * past the non-existent "header".
 */
bool HTTPChannel::
run_start_direct_file_read() {
  _state = S_read_header;
  if (!open_download_file()) {
    return false;
  }
  return false;
}

/**
 * In this state we have completely read the header lines returned by the
 * server (or proxy) in response to our request.  This state represents the
 * normal stopping point of a call to get_document(), etc.; further reads will
 * return the body of the request, the requested document.
 *
 * Normally run_read_header() is not called unless the user has elected not to
 * read the returned document himself.  In fact, the state itself only exists
 * so we can make a distinction between S_read_header and S_begin_body, where
 * S_read_header is safe to return to the user and S_begin_body means we need
 * to start skipping the document.
 */
bool HTTPChannel::
run_read_header() {
  _state = S_begin_body;
  return false;
}

/**
 * This state begins to skip over the body in preparation for making a new
 * request.
 */
bool HTTPChannel::
run_begin_body() {
  if (will_close_connection()) {
    // If the socket will close anyway, no point in skipping past the previous
    // body; just reset.
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "resetting to begin body; server would close anyway.\n";
    }
    reset_to_new();
    return false;
  }

  if (_server_response_has_no_body) {
    // We have already "read" the nonexistent body.
    _state = S_read_trailer;

  } else if (get_file_size() > (int)_skip_body_size) {
    // If we know the size of the body we are about to skip and it's too
    // large, then don't bother skipping it--just drop the connection and get
    // a new one.
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Dropping connection rather than skipping past "
        << get_file_size() << " bytes.\n";
    }
    reset_to_new();

  } else {
    open_read_body();
    if (_body_stream == nullptr) {
      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Unable to skip body.\n";
      }
      reset_to_new();

    } else {
      _owns_body_stream = true;
      if (_state != S_reading_body) {
        reset_body_stream();
      }
    }
  }

  return false;
}

/**
 * In this state we are in the process of reading the response's body.  We
 * will only come to this function if the user did not choose to read the
 * entire body himself (by calling open_read_body()).
 *
 * In this case we should skip past the body to reset the connection for
 * making a new request.
 */
bool HTTPChannel::
run_reading_body() {
  if (will_close_connection()) {
    // If the socket will close anyway, no point in skipping past the previous
    // body; just reset.
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "resetting to read body; server would close anyway.\n";
    }
    reset_to_new();
    return false;
  }

  // Skip the body we've already started.
  if (_body_stream == nullptr || !_owns_body_stream) {
    // Whoops, we're not in skip-body mode.  Better reset.
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "resetting, not in skip-body mode.\n";
    }
    reset_to_new();
    return false;
  }

  string line;
  std::getline(*_body_stream, line);
  while (!_body_stream->fail() && !_body_stream->eof()) {
    if (downloader_cat.is_spam()) {
      downloader_cat.spam()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "skip: " << line << "\n";
    }
    std::getline(*_body_stream, line);
  }

  if (!_body_stream->is_closed()) {
    // There's more to come later.
    return true;
  }

  reset_body_stream();

  // This should have been set by the call to finished_body(), above.
  nassertr(_state != S_reading_body, false);
  return false;
}

/**
 * In this state we have completely read (or skipped over) the body of the
 * response.  We should continue skipping past the trailer following the body.
 *
 * Not all bodies come with trailers; in particular, the "identity" transfer
 * encoding does not include a trailer.  It is therefore the responsibility of
 * the IdentityStreamBuf or ChunkedStreamBuf to set the state appropriately to
 * either S_read_body or S_read_trailer following the completion of the body.
 */
bool HTTPChannel::
run_read_body() {
  if (will_close_connection()) {
    // If the socket will close anyway, no point in skipping past the previous
    // body; just reset.
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "resetting to read body; server would close anyway.\n";
    }
    reset_to_new();
    return false;
  }
  // Skip the trailer following the recently-read body.

  string line;
  if (!server_getline(line)) {
    return true;
  }
  while (!line.empty()) {
    if (!server_getline(line)) {
      return true;
    }
  }

  _state = S_read_trailer;
  return false;
}

/**
 * In this state we have completely read the body and the trailer.  This state
 * is simply a pass-through back to S_ready.
 */
bool HTTPChannel::
run_read_trailer() {
  if (will_close_connection()) {
    // If the socket will close anyway, no point in skipping past the previous
    // body; just reset.
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "resetting to read trailer; server would close anyway.\n";
    }
    reset_to_new();
    return false;
  }

  _state = S_ready;
  return false;
}

/**
 * After the headers, etc.  have been read, this streams the download to the
 * named file.
 */
bool HTTPChannel::
run_download_to_file() {
  nassertr(_body_stream != nullptr && _owns_body_stream, false);

  bool do_throttle = _wanted_nonblocking && _download_throttle;

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  size_t remaining_this_pass = buffer_size;
  if (do_throttle) {
    remaining_this_pass = _bytes_per_update;
  }

  _body_stream->read(buffer, min(buffer_size, remaining_this_pass));
  size_t count = _body_stream->gcount();
  while (count != 0) {
    _download_to_stream->write(buffer, count);
    _bytes_downloaded += count;
    if (do_throttle) {
      nassertr(count <= remaining_this_pass, false);
      remaining_this_pass -= count;
      if (remaining_this_pass == 0) {
        // That's enough for now.
        return true;
      }
    }

    thread_consider_yield();
    _body_stream->read(buffer, min(buffer_size, remaining_this_pass));
    count = _body_stream->gcount();
  }

  if (_download_to_stream->fail()) {
    downloader_cat.warning()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Error writing to " << _download_to_filename << "\n";
    _status_entry._status_code = SC_download_write_error;
    _state = S_failure;
    reset_download_to();
    return false;
  }

  _download_to_stream->flush();

  if (_body_stream->is_closed()) {
    // Done.
    reset_body_stream();
    close_download_stream();
    _started_download = false;
    return false;
  } else {
    // More to come.
    return true;
  }
}

/**
 * After the headers, etc.  have been read, this streams the download to the
 * specified Ramfile object.
 */
bool HTTPChannel::
run_download_to_ram() {
  nassertr(_body_stream != nullptr && _owns_body_stream, false);
  nassertr(_download_to_ramfile != nullptr, false);

  bool do_throttle = _wanted_nonblocking && _download_throttle;

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  size_t remaining_this_pass = buffer_size;
  if (do_throttle) {
    remaining_this_pass = _bytes_per_update;
  }

  _body_stream->read(buffer, min(buffer_size, remaining_this_pass));
  size_t count = _body_stream->gcount();
  while (count != 0) {
    _download_to_ramfile->_data += string(buffer, count);
    _bytes_downloaded += count;
    if (do_throttle) {
      nassertr(count <= remaining_this_pass, false);
      remaining_this_pass -= count;
      if (remaining_this_pass == 0) {
        // That's enough for now.
        return true;
      }
    }

    thread_consider_yield();
    _body_stream->read(buffer, min(buffer_size, remaining_this_pass));
    count = _body_stream->gcount();
  }

  if (_body_stream->is_closed()) {
    // Done.
    reset_body_stream();
    close_download_stream();
    _started_download = false;
    return false;
  } else {
    // More to come.
    return true;
  }
}

/**
 * After the headers, etc.  have been read, this streams the download to the
 * named file.
 */
bool HTTPChannel::
run_download_to_stream() {
  nassertr(_body_stream != nullptr && _owns_body_stream, false);

  bool do_throttle = _wanted_nonblocking && _download_throttle;

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  size_t remaining_this_pass = buffer_size;
  if (do_throttle) {
    remaining_this_pass = _bytes_per_update;
  }

  _body_stream->read(buffer, min(buffer_size, remaining_this_pass));
  size_t count = _body_stream->gcount();
  while (count != 0) {
    _download_to_stream->write(buffer, count);
    _bytes_downloaded += count;
    if (do_throttle) {
      nassertr(count <= remaining_this_pass, false);
      remaining_this_pass -= count;
      if (remaining_this_pass == 0) {
        // That's enough for now.
        return true;
      }
    }

    thread_consider_yield();
    _body_stream->read(buffer, min(buffer_size, remaining_this_pass));
    count = _body_stream->gcount();
  }

  if (_download_to_stream->fail()) {
    downloader_cat.warning()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Error writing to stream\n";
    _status_entry._status_code = SC_download_write_error;
    _state = S_failure;
    reset_download_to();
    return false;
  }

  _download_to_stream->flush();

  if (_body_stream->is_closed()) {
    // Done.
    reset_body_stream();
    close_download_stream();
    _started_download = false;
    return false;
  } else {
    // More to come.
    return true;
  }
}


/**
 * Begins a new document request to the server, throwing away whatever request
 * was currently pending if necessary.
 */
void HTTPChannel::
begin_request(HTTPEnum::Method method, const DocumentSpec &url,
              const string &body, bool nonblocking,
              size_t first_byte, size_t last_byte) {

  downloader_cat.info()
    << _NOTIFY_HTTP_CHANNEL_ID
    << "begin " << method << " " << url << "\n";

  reset_for_new_request();

  _wanted_nonblocking = nonblocking;
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
  // In the presence of SIMPLE_THREADS, we always use non-blocking IO.  We
  // simulate blocking by yielding the thread.
  nonblocking = true;
#endif

  // Get the set of proxies that are appropriate for this URL.
  _proxies.clear();
  _proxy_next_index = 0;
  if (get_allow_proxy()) {
    _client->get_proxies_for_url(url.get_url(), _proxies);
  }

  // If we still have a live connection to a proxy that is on the list, that
  // proxy should be moved immediately to the front of the list (to minimize
  // restarting connections unnecessarily).
  if (!_bio.is_null() && !_proxies.empty() && !_proxy.empty()) {
    Proxies::iterator pi = find(_proxies.begin(), _proxies.end(), _proxy);
    if (pi != _proxies.end()) {
      _proxies.erase(pi);
      _proxies.insert(_proxies.begin(), _proxy);
    }
  }

  URLSpec new_proxy;
  if (_proxy_next_index < _proxies.size()) {
    new_proxy = _proxies[_proxy_next_index];
    _proxy_next_index++;
  }

  // Changing the proxy is grounds for dropping the old connection, if any.
  if (_proxy != new_proxy) {
    _proxy = new_proxy;
    _proxy_auth = nullptr;
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "resetting to change proxy to " << _proxy << "\n";
    }
    reset_to_new();
  }

  // Ditto with changing the nonblocking state.
  if (_nonblocking != nonblocking) {
    _nonblocking = nonblocking;
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "resetting to change nonblocking state to " << _nonblocking << ".\n";
    }
    reset_to_new();
  }

  reset_url(_request.get_url(), url.get_url());
  _request = url;
  _document_spec = DocumentSpec();
  _method = method;
  _body = body;

  // An https-style request means we'll need to establish an SSL connection.
  _want_ssl = _request.get_url().is_ssl();

  _first_byte_requested = first_byte;
  _last_byte_requested = last_byte;
  _connect_count = 0;

  reconsider_proxy();

  // Reset from whatever previous request might still be pending.
  if (_request.get_url().get_scheme() == "file") {
    // A "file" URL just means we're reading a raw file.  This only supports
    // actual disk files, not the VFS, because we use a BIO_new_file()
    // underneath this.
    reset_to_new();
    _bio = new BioPtr(_request.get_url());
    if (_bio->get_bio() != nullptr) {
      // Successfully opened the file.
      _source = new BioStreamPtr(new BioStream(_bio));
      _status_entry._status_code = 200;
      _state = S_start_direct_file_read;

      // Get the file size.
      FILE *fp = nullptr;
      BIO_get_fp(_bio->get_bio(), &fp);
      if (fp != nullptr) {
        if (fseek(fp, 0, SEEK_END) == 0) {
          _file_size = ftell(fp);
          _got_file_size = true;
          fseek(fp, 0, SEEK_SET);
        }
      }

    } else {
      // Couldn't read the file.
      OpenSSLWrapper::get_global_ptr()->notify_ssl_errors();
      _status_entry._status_code = SC_no_connection;
      _state = S_failure;
    }

  } else {
    // We're reading a normal network URL.
    if (_state == S_failure || (_state < S_read_header && _state != S_ready)) {
      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "resetting to clear previous request.\n";
      }
      reset_to_new();

    } else if (TrueClock::get_global_ptr()->get_short_time() - _last_run_time >= _idle_timeout) {
      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "resetting old connection: "
          << TrueClock::get_global_ptr()->get_short_time() - _last_run_time
          << " s old.\n";
      }
      reset_to_new();

    } else if (_state == S_read_header) {
      // Roll one step forwards to start skipping past the previous body.
      _state = S_begin_body;
    }
  }

  if (_method == HTTPEnum::M_connect) {
    _done_state = S_ready;
  } else {
    _done_state = S_read_header;
  }
}

/**
 * Reevaluates the flags and strings that are computed based on the particular
 * proxy we are attempting to connect to.  This should be called when we
 * initiate a request, and also whenever we change proxies while processing a
 * request.
 */
void HTTPChannel::
reconsider_proxy() {
  _proxy_tunnel_now = false;
  _proxy_serves_document = false;

  if (!_proxy.empty()) {
    // If the user insists we always tunnel through a proxy, or if we're
    // opening an SSL connection, or the user has explicitly asked for a
    // direct connection of some kind, or if we have a SOCKS-style proxy; each
    // of these demands a tunnel through the proxy to speak directly to the
    // http server.
    _proxy_tunnel_now =
      (get_proxy_tunnel() || _want_ssl ||
       _method == HTTPEnum::M_connect || _proxy.get_scheme() == "socks");

    // Otherwise (but we still have a proxy), then we ask the proxy to hand us
    // the document.
    _proxy_serves_document = !_proxy_tunnel_now;
  }

  make_header();
  make_request_text();

  if (_proxy_tunnel_now) {
    // Maybe we need to tunnel through the proxy to connect to the server
    // directly.
    ostringstream request;
    request
      << "CONNECT " << _request.get_url().get_server_and_port()
      << " " << _client->get_http_version_string() << "\r\n";
    if (_client->get_http_version() >= HTTPEnum::HV_11) {
      request
        << "Host: " << _request.get_url().get_server_and_port() << "\r\n";
    }
    _proxy_header = request.str();
    make_proxy_request_text();

  } else {
    _proxy_header = string();
    _proxy_request_text = string();
  }
}


/**
 * Resets the internal state variables in preparation for beginning a new
 * request.
 */
void HTTPChannel::
reset_for_new_request() {
  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "reset_for_new_request.\n";
  }

  reset_download_to();
  reset_body_stream();

  _last_status_code = 0;
  _status_entry = StatusEntry();

  _response_type = RT_none;
  _redirect_trail.clear();
  _bytes_downloaded = 0;
  _bytes_requested = 0;
}

/**
 * This is called by the body reading classes--ChunkedStreamBuf and
 * IdentityStreamBuf--when they have finished reading the body.  It advances
 * the state appropriately.
 *
 * has_trailer should be set true if the body type has an associated trailer
 * which should be read or skipped, or false if there is no trailer.
 */
void HTTPChannel::
finished_body(bool has_trailer) {
  if (will_close_connection() && _download_dest == DD_none) {
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "resetting to finish body; server would close anyway.\n";
    }
    reset_to_new();

  } else {
    if (has_trailer) {
      _state = HTTPChannel::S_read_body;
    } else {
      _state = HTTPChannel::S_read_trailer;
    }
  }
}

/**
 * If a download has been requested, opens the file on disk (or prepares the
 * RamFile or stream) and seeks within it to the appropriate
 * _first_byte_delivered position, so that downloaded bytes will be written to
 * the appropriate point within the file.  Returns true if the starting
 * position is valid, false otherwise (in which case the state is set to
 * S_failure).
 */
bool HTTPChannel::
open_download_file() {
  _subdocument_resumes = (_subdocument_resumes && _first_byte_delivered != 0);

  if (_download_dest == DD_file) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    _download_to_stream = vfs->open_write_file(_download_to_filename, false, !_subdocument_resumes);
    if (_download_to_stream == nullptr) {
      downloader_cat.info()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Could not open " << _download_to_filename << " for writing.\n";
      _status_entry._status_code = SC_download_open_error;
      _state = S_failure;
      return false;
    }
  }

  if (_subdocument_resumes) {
    if (_download_dest == DD_file) {
      // Windows doesn't complain if you try to seek past the end of file--it
      // happily appends enough zero bytes to make the difference.  Blecch.
      // That means we need to get the file size first to check it ourselves.
      _download_to_stream->seekp(0, std::ios::end);
      if (_first_byte_delivered > (size_t)_download_to_stream->tellp()) {
        downloader_cat.info()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Invalid starting position of byte " << _first_byte_delivered
          << " within " << _download_to_filename << " (which has "
          << _download_to_stream->tellp() << " bytes)\n";
        close_download_stream();
        _status_entry._status_code = SC_download_invalid_range;
        _state = S_failure;
        return false;
      }

      _download_to_stream->seekp(_first_byte_delivered);

    } else if (_download_dest == DD_ram) {
      if (_first_byte_delivered > _download_to_ramfile->_data.length()) {
        downloader_cat.info()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Invalid starting position of byte " << _first_byte_delivered
          << " within Ramfile (which has "
          << _download_to_ramfile->_data.length() << " bytes)\n";
        close_download_stream();
        _status_entry._status_code = SC_download_invalid_range;
        _state = S_failure;
        return false;
      }

      if (_first_byte_delivered == 0) {
        _download_to_ramfile->_data = string();
      } else {
        _download_to_ramfile->_data =
          _download_to_ramfile->_data.substr(0, _first_byte_delivered);
      }
    } else if (_download_dest == DD_stream) {
      // Windows doesn't complain if you try to seek past the end of file--it
      // happily appends enough zero bytes to make the difference.  Blecch.
      // That means we need to get the file size first to check it ourselves.
      _download_to_stream->seekp(0, std::ios::end);
      if (_first_byte_delivered > (size_t)_download_to_stream->tellp()) {
        downloader_cat.info()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Invalid starting position of byte " << _first_byte_delivered
          << " within stream (which has "
          << _download_to_stream->tellp() << " bytes)\n";
        close_download_stream();
        _status_entry._status_code = SC_download_invalid_range;
        _state = S_failure;
        return false;
      }

      _download_to_stream->seekp(_first_byte_delivered);
    }

  } else {
    // If _subdocument_resumes is false, we should be sure to reset to the
    // beginning of the file, regardless of the value of
    // _first_byte_delivered.
    if (_download_dest == DD_file || _download_dest == DD_stream) {
      _download_to_stream->seekp(0);
    } else if (_download_dest == DD_ram) {
      _download_to_ramfile->_data = string();
    }
  }

  return true;
}


/**
 * Reads a single line from the server's reply.  Returns true if the line is
 * successfully retrieved, or false if a complete line has not yet been
 * received or if the connection has been closed.
 */
bool HTTPChannel::
server_getline(string &str) {
  nassertr(!_source.is_null(), false);
  int ch = (*_source)->get();
  while (ch != EOF && !(*_source)->fail()) {
    switch (ch) {
    case '\n':
      // end-of-line character, we're done.
      str = _working_get;
      _working_get = string();
      {
        // Trim trailing whitespace.  We're not required to do this per the
        // HTTP spec, but let's be generous.
        size_t p = str.length();
        while (p > 0 && isspace(str[p - 1])) {
          --p;
        }
        str = str.substr(0, p);
      }
      if (downloader_cat.is_spam()) {
        downloader_cat.spam()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "recv: " << str << "\n";
      }
      return true;

    case '\r':
      // Ignore CR characters.
      break;

    default:
      _working_get += (char)ch;
    }
    ch = (*_source)->get();
  }

  check_socket();
  return false;
}

/**
 * Reads a line from the server's reply.  If the server disconnects or times
 * out before sending a reply, moves on to the next proxy server (or sets
 * failure mode) and returns false; otherwise, returns true.
 */
bool HTTPChannel::
server_getline_failsafe(string &str) {
  if (!server_getline(str)) {
    if (_bio.is_null()) {
      // Huh, the server hung up on us as soon as we tried to connect.
      if (_response_type == RT_hangup) {
        // This was our second immediate hangup in a row.  Give up.
        _status_entry._status_code = SC_lost_connection;
        _state = S_try_next_proxy;

      } else {
        // Try again, once.
        _response_type = RT_hangup;
      }

    } else {
      double elapsed =
        TrueClock::get_global_ptr()->get_short_time() -
        _sent_request_time;
      if (elapsed > get_http_timeout()) {
        // Time to give up.
        downloader_cat.info()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Timeout waiting for "
          << _request.get_url().get_server_and_port()
          << " in server_getline_failsafe (" << elapsed
          << " seconds elapsed).\n";
        _status_entry._status_code = SC_timeout;
        _state = S_try_next_proxy;
      }
    }

    return false;
  }
  return true;
}

/**
 * Reads a fixed number of bytes from the server's reply.  Returns true if the
 * indicated number of bytes are successfully retrieved, or false if the
 * complete set has not yet been received or if the connection has been
 * closed.
 */
bool HTTPChannel::
server_get(string &str, size_t num_bytes) {
  nassertr(!_source.is_null(), false);
  int ch = (*_source)->get();
  while (ch != EOF && !(*_source)->fail()) {
    _working_get += (char)ch;
    if (_working_get.length() >= num_bytes) {
      str = _working_get;
      _working_get = string();
      return true;
    }

    ch = (*_source)->get();
  }

  check_socket();
  return false;
}

/**
 * Reads a fixed number of bytes from the server.  If the server disconnects
 * or times out before sending a reply, moves on to the next proxy server (or
 * sets failure mode) and returns false; otherwise, returns true.
 */
bool HTTPChannel::
server_get_failsafe(string &str, size_t num_bytes) {
  if (!server_get(str, num_bytes)) {
    if (_bio.is_null()) {
      // Huh, the server hung up on us as soon as we tried to connect.
      if (_response_type == RT_hangup) {
        // This was our second immediate hangup in a row.  Give up.
        _status_entry._status_code = SC_lost_connection;
        _state = S_try_next_proxy;

      } else {
        // Try again, once.
        _response_type = RT_hangup;
      }

    } else {
      double elapsed =
        TrueClock::get_global_ptr()->get_short_time() -
        _sent_request_time;
      if (elapsed > get_http_timeout()) {
        // Time to give up.
        downloader_cat.info()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Timeout waiting for "
          << _request.get_url().get_server_and_port()
          << " in server_get_failsafe (" << elapsed
          << " seconds elapsed).\n";
        _status_entry._status_code = SC_timeout;
        _state = S_try_next_proxy;
      }
    }

    return false;
  }
  return true;
}

/**
 * Sends a series of lines to the server.  Returns true if the buffer is fully
 * sent, or false if some of it remains.  If this returns false, the function
 * must be called again later, passing in the exact same string, until the
 * return value is true.
 *
 * If the secret flag is true, the data is not echoed to the log (even in spam
 * mode).  This may be desirable if the data may contain binary data, or if it
 * may contain passwords etc.
 */
bool HTTPChannel::
server_send(const string &str, bool secret) {
  nassertr(str.length() > _sent_so_far, true);

  // Use the underlying BIO to write to the server, instead of the BIOStream,
  // which would insist on blocking (and might furthermore delay the send due
  // to collect-tcp mode being enabled).
  size_t bytes_to_send = str.length() - _sent_so_far;
  int write_count =
    BIO_write(*_bio, str.data() + _sent_so_far, bytes_to_send);

  if (write_count <= 0) {
    if (BIO_should_retry(*_bio)) {
      // Temporary failure: the pipe is full.  Wait till later.
      return false;
    }
    // Oops, the connection has been closed!
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Lost connection to server unexpectedly during write.\n";
    }
    reset_to_new();
    return false;
  }

  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "wrote " << write_count << " bytes to " << _bio << "\n";
  }

#ifndef NDEBUG
  if (!secret && downloader_cat.is_spam()) {
    show_send(str.substr(0, write_count));
  }
#endif

  if (write_count < (int)bytes_to_send) {
    _sent_so_far += write_count;
    return false;
  }

  // Buffer completely sent.
  _sent_so_far = 0;
  return true;
}

/**
 * Parses the first line sent back from an HTTP server or proxy and stores the
 * result in _status_code and _http_version, etc.  Returns true on success,
 * false on invalid response.
 */
bool HTTPChannel::
parse_http_response(const string &line) {
  // The first line back should include the HTTP version and the result code.
  if (line.length() < 5 || line.substr(0, 5) != string("HTTP/")) {
    // Not an HTTP response.
    _status_entry._status_code = SC_non_http_response;
    if (_response_type == RT_non_http) {
      // This was our second non-HTTP response in a row.  Give up.
      _state = S_try_next_proxy;

    } else {
      // Maybe we were just in some bad state.  Drop the connection and try
      // again, once.
      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "got non-HTTP response, resetting.\n";
      }
      reset_to_new();
      _response_type = RT_non_http;
    }
    return false;
  }

  // Split out the first line into its three components.
  size_t p = 5;
  while (p < line.length() && !isspace(line[p])) {
    p++;
  }
  _http_version_string = line.substr(0, p);
  _http_version = HTTPClient::parse_http_version_string(_http_version_string);

  while (p < line.length() && isspace(line[p])) {
    p++;
  }
  size_t q = p;
  while (q < line.length() && !isspace(line[q])) {
    q++;
  }
  string status_code = line.substr(p, q - p);
  _status_entry._status_code = atoi(status_code.c_str());

  while (q < line.length() && isspace(line[q])) {
    q++;
  }
  _status_entry._status_string = line.substr(q, line.length() - q);

  return true;
}

/**
 * Reads the series of header lines from the server and stores them in
 * _headers.  Returns true if there is more to read, false when done.
 */
bool HTTPChannel::
parse_http_header() {
  string line;
  if (!server_getline(line)) {
    return true;
  }

  while (!line.empty()) {
    if (isspace(line[0])) {
      // If the line begins with a space, that continues the previous field.
      size_t p = 0;
      while (p < line.length() && isspace(line[p])) {
        p++;
      }
      _current_field_value += line.substr(p - 1);

    } else {
      // If the line does not begin with a space, that defines a new field.
      if (!_current_field_name.empty()) {
        store_header_field(_current_field_name, _current_field_value);
        _current_field_value = string();
      }

      size_t colon = line.find(':');
      if (colon != string::npos) {
        _current_field_name = downcase(line.substr(0, colon));
        size_t p = colon + 1;
        while (p < line.length() && isspace(line[p])) {
          p++;
        }
        _current_field_value = line.substr(p);
      }
    }

    if (!server_getline(line)) {
      return true;
    }
  }

  // After reading an empty line, we're done with the headers.
  if (!_current_field_name.empty()) {
    store_header_field(_current_field_name, _current_field_value);
    _current_field_value = string();
  }

  return false;
}

/**
 * Interprets the "Content-Range" header in the reply, and fills in
 * _first_byte_delivered and _last_byte_delivered appropriately if the header
 * response can be understood.
 */
bool HTTPChannel::
parse_content_range(const string &content_range) {
  // First, get the units indication.
  size_t p = 0;
  while (p < content_range.length() && !isspace(content_range[p])) {
    p++;
  }

  string units = content_range.substr(0, p);
  while (p < content_range.length() && isspace(content_range[p])) {
    p++;
  }

  if (units == "bytes") {
    const char *c_str = content_range.c_str();
    char *endptr;
    if (p < content_range.length() && isdigit(content_range[p])) {
      long first_byte = strtol(c_str + p, &endptr, 10);
      p = endptr - c_str;
      if (p < content_range.length() && content_range[p] == '-') {
        p++;
        if (p < content_range.length() && isdigit(content_range[p])) {
          long last_byte = strtol(c_str + p, &endptr, 10);
          p = endptr - c_str;

          if (last_byte >= first_byte) {
            _first_byte_delivered = first_byte;
            _last_byte_delivered = last_byte;
            return true;
          }
        }
      }
    }
  }

  // Invalid or unhandled response.
  return false;
}


/**
 * Checks whether the connection to the server has been closed after a failed
 * read.  If it has, issues a warning and calls reset_to_new().
 */
void HTTPChannel::
check_socket() {
  nassertv(!_source.is_null());
  if ((*_source)->is_closed()) {
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Lost connection to server unexpectedly during read.\n";
    }
    reset_to_new();
  }
}

/*
   Certificate verify error codes:

0 X509_V_OK: ok

    the operation was successful.

2 X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT: unable to get issuer certificate

    the issuer certificate could not be found: this occurs if the
    issuer certificate of an untrusted certificate cannot be found.

3 X509_V_ERR_UNABLE_TO_GET_CRL unable to get certificate CRL

    the CRL of a certificate could not be found. Unused.

4 X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE: unable to decrypt
certificate's signature

    the certificate signature could not be decrypted. This means that
    the actual signature value could not be determined rather than it
    not matching the expected value, this is only meaningful for RSA
    keys.

5 X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE: unable to decrypt CRL's signature

    the CRL signature could not be decrypted: this means that the
    actual signature value could not be determined rather than it not
    matching the expected value. Unused.

6 X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY: unable to decode
issuer public key

    the public key in the certificate SubjectPublicKeyInfo could not
    be read.

7 X509_V_ERR_CERT_SIGNATURE_FAILURE: certificate signature failure

    the signature of the certificate is invalid.

8 X509_V_ERR_CRL_SIGNATURE_FAILURE: CRL signature failure

    the signature of the certificate is invalid. Unused.

9 X509_V_ERR_CERT_NOT_YET_VALID: certificate is not yet valid

    the certificate is not yet valid: the notBefore date is after the
    current time.

10 X509_V_ERR_CERT_HAS_EXPIRED: certificate has expired

    the certificate has expired: that is the notAfter date is before
    the current time.

11 X509_V_ERR_CRL_NOT_YET_VALID: CRL is not yet valid

    the CRL is not yet valid. Unused.

12 X509_V_ERR_CRL_HAS_EXPIRED: CRL has expired

    the CRL has expired. Unused.

13 X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD: format error in
certificate's notBefore field

    the certificate notBefore field contains an invalid time.

14 X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD: format error in
certificate's notAfter field

    the certificate notAfter field contains an invalid time.

15 X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD: format error in CRL's
lastUpdate field

    the CRL lastUpdate field contains an invalid time. Unused.

16 X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD: format error in CRL's
nextUpdate field

    the CRL nextUpdate field contains an invalid time. Unused.

17 X509_V_ERR_OUT_OF_MEM: out of memory

    an error occurred trying to allocate memory. This should never
    happen.

18 X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT: self signed certificate

    the passed certificate is self signed and the same certificate
    cannot be found in the list of trusted certificates.

19 X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN: self signed certificate in
certificate chain

    the certificate chain could be built up using the untrusted
    certificates but the root could not be found locally.

20 X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY: unable to get local
issuer certificate

    the issuer certificate of a locally looked up certificate could
    not be found. This normally means the list of trusted certificates
    is not complete.

21 X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE: unable to verify the
first certificate

    no signatures could be verified because the chain contains only
    one certificate and it is not self signed.

22 X509_V_ERR_CERT_CHAIN_TOO_LONG: certificate chain too long

    the certificate chain length is greater than the supplied maximum
    depth. Unused.

23 X509_V_ERR_CERT_REVOKED: certificate revoked

    the certificate has been revoked. Unused.

24 X509_V_ERR_INVALID_CA: invalid CA certificate

    a CA certificate is invalid. Either it is not a CA or its
    extensions are not consistent with the supplied purpose.

25 X509_V_ERR_PATH_LENGTH_EXCEEDED: path length constraint exceeded

    the basicConstraints pathlength parameter has been exceeded.

26 X509_V_ERR_INVALID_PURPOSE: unsupported certificate purpose

    the supplied certificate cannot be used for the specified purpose.

27 X509_V_ERR_CERT_UNTRUSTED: certificate not trusted

    the root CA is not marked as trusted for the specified purpose.

28 X509_V_ERR_CERT_REJECTED: certificate rejected

    the root CA is marked to reject the specified purpose.

29 X509_V_ERR_SUBJECT_ISSUER_MISMATCH: subject issuer mismatch

    the current candidate issuer certificate was rejected because its
    subject name did not match the issuer name of the current
    certificate. Only displayed when the -issuer_checks option is set.

30 X509_V_ERR_AKID_SKID_MISMATCH: authority and subject key identifier
mismatch

    the current candidate issuer certificate was rejected because its
    subject key identifier was present and did not match the authority
    key identifier current certificate. Only displayed when the
    -issuer_checks option is set.

31 X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH: authority and issuer serial
number mismatch

    the current candidate issuer certificate was rejected because its
    issuer name and serial number was present and did not match the
    authority key identifier of the current certificate. Only
    displayed when the -issuer_checks option is set.

32 X509_V_ERR_KEYUSAGE_NO_CERTSIGN:key usage does not include
certificate signing

    the current candidate issuer certificate was rejected because its
    keyUsage extension does not permit certificate signing.

50 X509_V_ERR_APPLICATION_VERIFICATION: application verification failure

    an application specific error. Unused.

*/

/**
 * Checks to see if the indicated certificate is on the pre-approved list for
 * the current server.
 *
 * If the full cert itself (including its key) is on the pre-approved list,
 * sets both cert_preapproved and cert_name_preapproved to true.
 *
 * If the full cert is not on the pre-approved list, but its name matches a
 * name on the pre-approved list, sets cert_name_preapproved to true, and
 * cert_preapproved to false.
 *
 * Otherwise, sets both values to false.  This doesn't mean the cert is
 * necessarily invalid, just that it wasn't on the pre-approved list (which is
 * usually empty anyway).
 */
void HTTPChannel::
check_preapproved_server_certificate(X509 *cert, bool &cert_preapproved,
                                     bool &cert_name_preapproved) const {
  return _client->check_preapproved_server_certificate(_request.get_url(),
                                                       cert, cert_preapproved,
                                                       cert_name_preapproved);
}

/**
 * Returns true if the name in the cert matches the hostname of the server,
 * false otherwise.
 */
bool HTTPChannel::
validate_server_name(X509 *cert) {
  string hostname = _request.get_url().get_server();

  vector_string cert_names;

  // According to RFC 2818, we should check the DNS name(s) in the
  // subjectAltName extension first, if that extension exists.
  STACK_OF(GENERAL_NAME) *subject_alt_names =
    (STACK_OF(GENERAL_NAME) *)X509_get_ext_d2i(cert, NID_subject_alt_name, nullptr, nullptr);
  if (subject_alt_names != nullptr) {
    int num_alts = sk_GENERAL_NAME_num(subject_alt_names);
    for (int i = 0; i < num_alts; ++i) {
      // Get the ith alt name.
      const GENERAL_NAME *alt_name =
        sk_GENERAL_NAME_value(subject_alt_names, i);

      if (alt_name->type == GEN_DNS) {
        char *buffer = nullptr;
        int len = ASN1_STRING_to_UTF8((unsigned char**)&buffer,
                                      alt_name->d.ia5);
        if (len > 0) {
          cert_names.push_back(string(buffer, len));
        }
        if (buffer != nullptr) {
          OPENSSL_free(buffer);
        }
      }
    }
  }

  if (cert_names.empty()) {
    // If there were no DNS names, use the common name instead.

    X509_NAME *xname = X509_get_subject_name(cert);
    if (xname != nullptr) {
      string common_name = get_x509_name_component(xname, NID_commonName);
      cert_names.push_back(common_name);
    }
  }

  if (cert_names.empty()) {
    downloader_cat.info()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Server certificate from " << hostname
      << " provides no name.\n";
    return false;
  }

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "Server certificate from " << hostname
      << " provides name(s):";
    vector_string::const_iterator si;
    for (si = cert_names.begin(); si != cert_names.end(); ++si) {
      const string &cert_name = (*si);
      downloader_cat.debug(false)
        << " " << cert_name;
    }
    downloader_cat.debug(false)
      << "\n";
  }

  // Now validate the names we found.  If any of them matches, the cert
  // matches.
  vector_string::const_iterator si;
  for (si = cert_names.begin(); si != cert_names.end(); ++si) {
    const string &cert_name = (*si);

    if (match_cert_name(cert_name, hostname)) {
      return true;
    }
  }

  downloader_cat.info()
    << _NOTIFY_HTTP_CHANNEL_ID
    << "Server certificate from " << hostname
    << " provides wrong name(s):";
  for (si = cert_names.begin(); si != cert_names.end(); ++si) {
    const string &cert_name = (*si);
    downloader_cat.info(false)
      << " " << cert_name;
  }
  downloader_cat.info(false)
    << "\n";

  return false;
}

/**
 * Returns true if this particular name from the certificate matches the
 * indicated hostname, false otherwise.
 */
bool HTTPChannel::
match_cert_name(const string &cert_name, const string &hostname) {
  // We use GlobPattern to match the name.  This isn't quite consistent with
  // RFC2818, since it also accepts additional wildcard characters like "?"
  // and "[]", but I think it's close enough.

  GlobPattern pattern(cert_name);
  pattern.set_case_sensitive(false);
  pattern.set_nomatch_chars(".");
  return pattern.matches(hostname);
}

/**
 * Returns the indicated component of the X509 name as a string, if defined,
 * or empty string if it is not.
 */
string HTTPChannel::
get_x509_name_component(X509_NAME *name, int nid) {
  ASN1_OBJECT *obj = OBJ_nid2obj(nid);

  if (obj == nullptr) {
    // Unknown nid.  See opensslobjects.h.
    return string();
  }

  int i = X509_NAME_get_index_by_OBJ(name, obj, -1);
  if (i < 0) {
    return string();
  }

  ASN1_STRING *data = X509_NAME_ENTRY_get_data(X509_NAME_get_entry(name, i));
  return string((char *)data->data, data->length);
}

/**
 * Formats the appropriate GET or POST (or whatever) request to send to the
 * server, based on the current _method, _document_spec, _body, and _proxy
 * settings.
 */
void HTTPChannel::
make_header() {
  _proxy_auth = _client->select_auth(_proxy, true, _proxy_realm);
  _proxy_username = string();
  if (_proxy_auth != nullptr) {
    _proxy_realm = _proxy_auth->get_realm();
    _proxy_username = _client->select_username(_proxy, true, _proxy_realm);
  }

  if (_method == HTTPEnum::M_connect) {
    // This method doesn't require an HTTP header at all; we'll just open a
    // plain connection.  (Except when we're using a proxy; but in that case,
    // it's the proxy_header we'll need, not the regular HTTP header.)
    _header = string();
    return;
  }

  _www_auth = _client->select_auth(_request.get_url(), false, _www_realm);
  _www_username = string();
  if (_www_auth != nullptr) {
    _www_realm = _www_auth->get_realm();
    _www_username = _client->select_username(_request.get_url(), false, _www_realm);
  }

  string request_path;
  if (_proxy_serves_document) {
    // If we'll be asking the proxy for the document, we need its full URL--
    // but we omit the username, which is information just for us.
    URLSpec url_no_username = _request.get_url();
    url_no_username.set_username(string());
    request_path = url_no_username.get_url();

  } else {
    // If we'll be asking the server directly for the document, we just want
    // its path relative to the server.
    request_path = _request.get_url().get_path_and_query();
  }

  // HTTP syntax always requires something in the request path.  If it is
  // empty, put in a star as a placeholder (OPTIONS, for instance, uses this).
  if (request_path.empty()) {
    request_path = "*";
  }

  ostringstream stream;

  stream
    << _method << " " << request_path << " "
    << _client->get_http_version_string() << "\r\n";

  if (_client->get_http_version() >= HTTPEnum::HV_11) {

    if (_request.get_url().has_port() && _request.get_url().is_default_port()) {
      // It appears that some servers (notably gstatic.com) might return a 404
      // if you include an explicit port number in with the Host: header, even
      // if it is the default port.  So, don't include the port number unless
      // we need to.
      string server = _request.get_url().get_server();
      if (server.find(':') != string::npos) {
        stream << "Host: [" << server << "]";
      } else {
        stream << "Host: " << server;
      }
    } else {
      stream << "Host: " << _request.get_url().get_server_and_port();
    }
    stream << "\r\n";
    if (!get_persistent_connection()) {
      stream
        << "Connection: close\r\n";
    }
  }

  if (_last_byte_requested != 0) {
    stream
      << "Range: bytes=" << _first_byte_requested << "-"
      << _last_byte_requested << "\r\n";

  } else if (_first_byte_requested != 0) {
    stream
      << "Range: bytes=" << _first_byte_requested << "-\r\n";
  }

  switch (_request.get_request_mode()) {
  case DocumentSpec::RM_any:
    // No particular request; give us any document that matches the URL.
    if (_first_byte_requested != 0) {
      // Unless we're requesting a subrange, in which case if the exact
      // document matches, retrieve the subrange indicated; otherwise,
      // retrieve the entire document.
      if (_request.has_tag()) {
        stream
          << "If-Range: " << _request.get_tag().get_string() << "\r\n";
      } else if (_request.has_date()) {
        stream
          << "If-Range: " << _request.get_date().get_string() << "\r\n";
      }
    }
    break;

  case DocumentSpec::RM_equal:
    // Give us only this particular version of the document, or nothing.
    if (_request.has_tag()) {
      stream
        << "If-Match: " << _request.get_tag().get_string() << "\r\n";
    }
    if (_request.has_date()) {
      stream
        << "If-Unmodified-Since: " << _request.get_date().get_string()
        << "\r\n";
    }
    break;

  case DocumentSpec::RM_newer:
    // Give us anything newer than this document, or nothing.
    if (_request.has_tag()) {
      stream
        << "If-None-Match: " << _request.get_tag().get_string() << "\r\n";
    }
    if (_request.has_date()) {
      stream
        << "If-Modified-Since: " << _request.get_date().get_string()
        << "\r\n";
    }
    break;

  case DocumentSpec::RM_equal_or_newer:
    // Just don't give us anything older.
    if (_request.has_date()) {
      // This is a little unreliable: we ask for any document that's been
      // modified since one second before our last-modified-date.  Who knows
      // whether the server will honor this properly.
      stream
        << "If-Modified-Since: " << (_request.get_date() - 1).get_string()
        << "\r\n";
    }
    break;
  }

  switch (_request.get_cache_control()) {
  case DocumentSpec::CC_allow_cache:
    // Normal, caching behavior.
    break;

  case DocumentSpec::CC_revalidate:
    // Request the server to revalidate its cache before returning it.
    stream
      << "Cache-Control: max-age=0\r\n";
    break;

  case DocumentSpec::CC_no_cache:
    // Request the server to get a fresh copy regardless of its cache.
    stream
      << "Cache-Control: no-cache\r\n"
      << "Pragma: no-cache\r\n";
    break;
  }

  _client->send_cookies(stream, _request.get_url());

  if (!_body.empty()) {
    stream
      << "Content-Type: " << _content_type << "\r\n"
      << "Content-Length: " << _body.length() << "\r\n";
  }

  _header = stream.str();
}

/**
 * Builds the _proxy_request_text string.  This is a special request that will
 * be sent directly to the proxy prior to the request tailored for the server.
 * Generally this is used to open a tunnelling connection for https-over-
 * proxy.
 */
void HTTPChannel::
make_proxy_request_text() {
  _proxy_request_text = _proxy_header;

  if (_proxy_auth != nullptr && !_proxy_username.empty()) {
    _proxy_request_text += "Proxy-Authorization: ";
    _proxy_request_text +=
      _proxy_auth->generate(HTTPEnum::M_connect, _request.get_url().get_server_and_port(),
                            _proxy_username, _body);
    _proxy_request_text += "\r\n";
  }

  _proxy_request_text += "\r\n";
}

/**
 * Builds the _request_text string.  This is the specific request that will be
 * sent to the server this pass, based on the current header and body.
 */
void HTTPChannel::
make_request_text() {
  _request_text = _header;

  if (_proxy_serves_document &&
      _proxy_auth != nullptr && !_proxy_username.empty()) {
    _request_text += "Proxy-Authorization: ";
    _request_text +=
      _proxy_auth->generate(_method, _request.get_url().get_url(), _proxy_username, _body);
    _request_text += "\r\n";
  }

  if (_www_auth != nullptr && !_www_username.empty()) {
    string authorization =
    _request_text += "Authorization: ";
    _request_text +=
      _www_auth->generate(_method, _request.get_url().get_path_and_query(), _www_username, _body);
    _request_text += "\r\n";
  }

  _request_text += _send_extra_headers;
  _request_text += "\r\n";
  _request_text += _body;
}

/**
 * Redirects the next connection to the indicated URL (from the previous URL).
 * This resets the socket if necessary when we are about to switch servers.
 */
void HTTPChannel::
reset_url(const URLSpec &old_url, const URLSpec &new_url) {
  // If we change between http and https, we have to reset the connection
  // regardless of proxy.  Otherwise, we have to drop the connection if the
  // server or port changes, unless we're communicating through a proxy.

  if (new_url.get_scheme() != old_url.get_scheme() ||
      (_proxy.empty() && (new_url.get_server() != old_url.get_server() ||
                          new_url.get_port() != old_url.get_port()))) {
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "resetting for new server "
        << new_url.get_server_and_port() << "\n";
    }
    reset_to_new();
  }
}

/**
 * Stores a single name: value pair in the header list, or appends the value
 * to the end of the existing value, if the header has been repeated.
 */
void HTTPChannel::
store_header_field(const string &field_name, const string &field_value) {
  std::pair<Headers::iterator, bool> insert_result =
    _headers.insert(Headers::value_type(field_name, field_value));

  if (!insert_result.second) {
    // It didn't insert; thus, the field already existed.  Append the new
    // value.
    Headers::iterator hi = insert_result.first;
    (*hi).second += ", ";
    (*hi).second += field_value;
  }

  if (field_name == "set-cookie") {
    _client->set_cookie(HTTPCookie(field_value, _request.get_url()));
  }
}

#ifndef NDEBUG
/**
 * Writes the outgoing message, one line at a time, to the debugging log.
 */
void HTTPChannel::
show_send(const string &message) {
  size_t start = 0;
  size_t newline = message.find('\n', start);
  while (newline != string::npos) {
    // Assume every \n is preceded by a \r.
    downloader_cat.spam()
      << "send: " << message.substr(start, newline - start - 1) << "\n";
    start = newline + 1;
    newline = message.find('\n', start);
  }

  if (start < message.length()) {
    downloader_cat.spam()
      << "send: " << message.substr(start) << " (no newline)\n";
  }
}
#endif   // NDEBUG

/**
 * Resets the indication of how the document will be downloaded.  This must be
 * re-specified after each get_document() (or related) call.
 */
void HTTPChannel::
reset_download_to() {
  _started_download = false;
  close_download_stream();
  _download_dest = DD_none;
}

/**
 * Ensures the file opened for receiving the download has been correctly
 * closed.
 */
void HTTPChannel::
close_download_stream() {
  if (_download_to_stream != nullptr) {
    _download_to_stream->flush();
    if (_download_dest == DD_file) {
      VirtualFileSystem::close_write_file(_download_to_stream);
    }
  }
  _download_to_ramfile = nullptr;
  _download_to_stream = nullptr;
}


/**
 * Closes the connection and resets the state to S_new.
 */
void HTTPChannel::
reset_to_new() {
  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << _NOTIFY_HTTP_CHANNEL_ID
      << "reset_to_new.\n";
  }

  close_connection();
  _state = S_new;
}

/**
 * Clears the _body_stream pointer, if it is set.
 */
void HTTPChannel::
reset_body_stream() {
  if (_owns_body_stream) {
    if (_body_stream != nullptr) {
      close_read_body(_body_stream);
      nassertv(_body_stream == nullptr && !_owns_body_stream);
    }
  } else {
    _body_stream = nullptr;
  }
}


/**
 * Closes the connection but leaves the _state unchanged.
 */
void HTTPChannel::
close_connection() {
  reset_body_stream();
  _source.clear();
  _bio.clear();
  _working_get = string();
  _sent_so_far = 0;
  _read_index++;
}

/**
 * Returns true if status code a is a more useful value (that is, it
 * represents a more-nearly successfully connection attempt, or contains more
 * information) than b, or false otherwise.
 */
bool HTTPChannel::
more_useful_status_code(int a, int b) {
  if (a >= 100 && b >= 100) {
    // Both represent HTTP responses.  Responses from a server (< 1000) are
    // better than those from a proxy; we take advantage of the fact that we
    // have already added 1000 to proxy responses.  Except for 407, so let's
    // fix that now.
    if (a == 407) {
      a += 1000;
    }
    if (b == 407) {
      b += 1000;
    }

    // Now just check the series.
    int series_a = (a / 100);
    int series_b = (b / 100);

    // In general, a lower series is a closer success.
    return (series_a < series_b);
  }

  if (a < 100 && b < 100) {
    // Both represent non-HTTP responses.  Here a larger number is better.
    return (a > b);
  }

  if (a < 100) {
    // a is a non-HTTP response, while b is an HTTP response.  HTTP is
    // generally, better, unless we exceeded SC_http_error_watermark.
    return (a > SC_http_error_watermark);
  }

  // Exactly the opposite case as above.
  return (b < SC_http_error_watermark);
}


/**
 *
 */
ostream &
operator << (ostream &out, HTTPChannel::State state) {
#ifdef NDEBUG
  return out << (int)state;
#else
  switch (state) {
  case HTTPChannel::S_new:
    return out << "new";

  case HTTPChannel::S_try_next_proxy:
    return out << "try_next_proxy";

  case HTTPChannel::S_connecting:
    return out << "connecting";

  case HTTPChannel::S_connecting_wait:
    return out << "connecting_wait";

  case HTTPChannel::S_http_proxy_ready:
    return out << "http_proxy_ready";

  case HTTPChannel::S_http_proxy_request_sent:
    return out << "http_proxy_request_sent";

  case HTTPChannel::S_http_proxy_reading_header:
    return out << "http_proxy_reading_header";

  case HTTPChannel::S_socks_proxy_greet:
    return out << "socks_proxy_greet";

  case HTTPChannel::S_socks_proxy_greet_reply:
    return out << "socks_proxy_greet_reply";

  case HTTPChannel::S_socks_proxy_connect:
    return out << "socks_proxy_connect";

  case HTTPChannel::S_socks_proxy_connect_reply:
    return out << "socks_proxy_connect_reply";

  case HTTPChannel::S_setup_ssl:
    return out << "setup_ssl";

  case HTTPChannel::S_ssl_handshake:
    return out << "ssl_handshake";

  case HTTPChannel::S_ready:
    return out << "ready";

  case HTTPChannel::S_request_sent:
    return out << "request_sent";

  case HTTPChannel::S_reading_header:
    return out << "reading_header";

  case HTTPChannel::S_start_direct_file_read:
    return out << "start_direct_file_read";

  case HTTPChannel::S_read_header:
    return out << "read_header";

  case HTTPChannel::S_begin_body:
    return out << "begin_body";

  case HTTPChannel::S_reading_body:
    return out << "reading_body";

  case HTTPChannel::S_read_body:
    return out << "read_body";

  case HTTPChannel::S_read_trailer:
    return out << "read_trailer";

  case HTTPChannel::S_failure:
    return out << "failure";
  }

  return out << "invalid state(" << (int)state << ")";
#endif  // NDEBUG
}

#endif  // HAVE_OPENSSL
