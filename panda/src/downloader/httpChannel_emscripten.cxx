/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpChannel_emscripten.cxx
 * @author rdb
 * @date 2021-02-10
 */

#include "httpChannel_emscripten.h"

#ifdef __EMSCRIPTEN__

#include "string_utils.h"

#include <emscripten/em_asm.h>

#define _NOTIFY_HTTP_CHANNEL_ID   "[" << this << "] "

#ifndef CPPPARSER
extern "C" char *EMSCRIPTEN_KEEPALIVE
_extend_string(std::string *str, int length) {
  size_t offset = str->size();
  str->resize(offset + (size_t)length);
  return (char *)str->data() + offset;
}

extern "C" void EMSCRIPTEN_KEEPALIVE
_write_stream(std::ostream *strm, const char *data, int length) {
  strm->write(data, (size_t)length);
}

extern "C" bool EMSCRIPTEN_KEEPALIVE
_http_channel_run(HTTPChannel *channel) {
  return channel->run();
}
#endif

TypeHandle HTTPChannel::_type_handle;

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

  EM_ASM({
    if (!window._httpChannels) {
      window._httpChannels = {};
    }
    var xhr = new XMLHttpRequest();
    window._httpChannels[$0] = xhr;
  }, this);

  // _nonblocking is true if the XHR is actually in non-blocking mode.
  _nonblocking = false;

  // _wanted_nonblocking is true if the user specifically requested one of the
  // non-blocking interfaces.  It is false if the XHR is only incidentally
  // non-blocking (for instance, because ASYNCIFY is on).
  _wanted_nonblocking = false;

  _first_byte_requested = 0;
  _last_byte_requested = 0;
  _first_byte_delivered = 0;
  _last_byte_delivered = 0;
  _expected_file_size = 0;
  _file_size = 0;
  _transfer_file_size = 0;
  _got_expected_file_size = false;
  _got_file_size = false;
  _got_transfer_file_size = false;
  _bytes_downloaded = 0;
  _bytes_requested = 0;
  _status_entry = StatusEntry();
  _content_type = "application/x-www-form-urlencoded";
  _download_dest = DD_none;
  _download_to_ramfile = nullptr;
  _download_to_stream = nullptr;
}

/**
 *
 */
HTTPChannel::
~HTTPChannel() {
  EM_ASM({
    var xhr = window._httpChannels[$0];
    if (xhr) {
      xhr.onprogress = null;
      xhr.onreadystatechange = null;
      delete window._httpChannels[$0];
    }
  }, this);

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << _NOTIFY_HTTP_CHANNEL_ID
    << "destroyed.\n";
  }

  reset_download_to();
}

/**
 * Returns the HTML header value associated with the indicated key, or empty
 * string if the key was not defined in the message returned by the server.
 */
std::string HTTPChannel::
get_header_value(const std::string &key) const {
  Headers::const_iterator hi = _headers.find(downcase(key));
  if (hi != _headers.end()) {
    return (*hi).second;
  }
  return std::string();
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
write_headers(std::ostream &out) const {
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

  State state = get_state();
  switch (state) {
  case S_unsent:
    // Invalid.
    return false;

  case S_opened:
    if (!run_send()) {
      return false;
    }
    break;

  case S_headers_received:
    if (!run_headers_received()) {
      return false;
    }
    break;

  case S_loading:
    if (_download_dest != DD_none) {
      return false;
    }
    break;

  case S_done:
    close_download_stream();
    return false;
  }

  // If we get here, we must be running in non-blocking mode.
  if (!_wanted_nonblocking && emscripten_has_asyncify()) {
    // But we are pretending to be in blocking mode, so we must yield until the
    // state changes.
    while (get_state() != state) {
      emscripten_sleep(0);
    }
  }

  return true;
}

/**
 * Returns the current readyState of the XMLHttpRequest object.
 */
HTTPChannel::State HTTPChannel::
get_state() const {
  return (State)EM_ASM_INT(return window._httpChannels[$0].readyState, this);
}

/**
 * Calls XHR.send().
 */
bool HTTPChannel::
run_send() {
  for (const ExtraHeader &header : _send_extra_headers) {
    EM_ASM({
      var xhr = window._httpChannels[$0];
      xhr.setRequestHeader(UTF8ToString($1), UTF8ToString($2));
    }, this, header.first.c_str(), header.second.c_str());
  }

  if (_method == HTTPEnum::M_get || _method == HTTPEnum::M_head) {
    // No body is sent with GET / HEAD requests.
    EM_ASM({
      var xhr = window._httpChannels[$0];
      xhr.send(null);
    }, this);
  }
  else {
    EM_ASM({
      var xhr = window._httpChannels[$0];
      xhr.setRequestHeader("Content-Type", UTF8ToString($2));
      xhr.send(UTF8ToString($1));
    }, this, _body.c_str(), _content_type.c_str());
  }

  return true;
}

/**
 * Called when the headers have been received.
 */
bool HTTPChannel::
run_headers_received() {
  char status_string[512];
  char *header_str = nullptr;
  status_string[0] = 0;

  // Fetch the status code, text and response headers from JavaScript.
  int status_code = EM_ASM_INT({
    var xhr = window._httpChannels[$0];
    stringToUTF8(xhr.statusText, $1, 512);

    var headers = xhr.getAllResponseHeaders();
    var len = lengthBytesUTF8(headers) + 1;
    var buffer = _malloc(len);
    stringToUTF8(headers, buffer, len);
    setValue($2, buffer, '*');

    return xhr.status;
  }, this, status_string, &header_str);
  _status_entry._status_code = status_code;

  // Parse the response header string.
  char *ptr = header_str;
  char *delim = strstr(ptr, ": ");
  while (delim != nullptr) {
    std::string key(ptr, delim);
    ptr = delim + 2;

    std::string value;
    delim = strstr(ptr, "\r\n");
    if (delim != nullptr) {
      value.assign(ptr, delim);
      ptr = delim + 2;
      delim = strstr(ptr, ": ");
    }
    else {
      // The XHR spec prescribes that there is always another CRLF
      // after the last header, but we handle this case anyway.
      value.assign(ptr);
    }

    _headers[std::move(key)] = std::move(value);
  }
  free(header_str);

  // Look for key properties in the header fields.
  if (status_code == 206) {
    std::string content_range = get_header_value("Content-Range");
    if (content_range.empty()) {
      downloader_cat.warning()
        << _NOTIFY_HTTP_CHANNEL_ID
        << "Got 206 response without Content-Range header!\n";
      _status_entry._status_code = SC_invalid_http;
      return false;

    } else {
      if (!parse_content_range(content_range)) {
        downloader_cat.warning()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Couldn't parse Content-Range: " << content_range << "\n";
        _status_entry._status_code = SC_invalid_http;
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
  std::string tag = get_header_value("ETag");
  if (!tag.empty()) {
    _document_spec.set_tag(HTTPEntityTag(tag));
  }
  std::string date = get_header_value("Last-Modified");
  if (!date.empty()) {
    _document_spec.set_date(HTTPDate(date));
  }

  // In case we've got a download in effect, now we know what the first byte
  // of the subdocument request will be, so we can open the file and position
  // it.
  if (status_code / 100 == 1 || status_code == 204 || status_code == 304) {
    // Never mind on the download.
    reset_download_to();
  }

  if (!open_download_file()) {
    return false;
  }

  if (_download_dest == DD_ram) {
    std::string *dest = &_download_to_ramfile->_data;
    EM_ASM({
      var xhr = window._httpChannels[$0];
      var loaded = 0;
      xhr.onprogress = function (ev) {
        var body = this.responseText;
        var ptr = __extend_string($1, ev.loaded - loaded);
        for (var i = loaded; i < ev.loaded; ++i) {
          HEAPU8[ptr + i] = body.charCodeAt(i);
        }
        loaded = ev.loaded;
      };
    }, this, dest);
  }
  else if (_download_dest == DD_stream) {
    std::ostream *dest = _download_to_stream;
    char buffer[4096];
    EM_ASM({
      var xhr = window._httpChannels[$0];
      var loaded = 0;
      xhr.onprogress = function (ev) {
        while (loaded < ev.loaded) {
          var body = this.responseText;
          var size = Math.min(ev.loaded - read, 4096);
          for (var i = read; i < read + size; ++i) {
            HEAPU8[$2 + i] = body.charCodeAt(i);
          }
          __write_stream($1, $2, size);
          loaded += size;
        }
      };
    }, this, dest, buffer);
  }

  _got_expected_file_size = false;
  _got_file_size = false;
  _got_transfer_file_size = false;

  std::string content_length = get_header_value("Content-Length");
  if (!content_length.empty()) {
    _file_size = atoi(content_length.c_str());
    _got_file_size = true;
  }
  else if (status_code == 206) {
    // Well, we didn't get a content-length from the server, but we can infer
    // the number of bytes based on the range we're given.
    _file_size = _last_byte_delivered - _first_byte_delivered + 1;
    _got_file_size = true;
  }

  // Reset these for the next request.
  clear_extra_headers();

  return (_download_dest != DD_none);
}

/**
 * Begins a new document request to the server, throwing away whatever request
 * was currently pending if necessary.
 */
bool HTTPChannel::
begin_request(HTTPEnum::Method method, const DocumentSpec &url,
              const std::string &body, bool nonblocking,
              size_t first_byte, size_t last_byte) {

  downloader_cat.info()
    << _NOTIFY_HTTP_CHANNEL_ID
    << "begin " << method << " " << url << "\n";

  reset_for_new_request();

  _wanted_nonblocking = nonblocking;
  _nonblocking = nonblocking || emscripten_has_asyncify();

  _request = url;
  _document_spec = DocumentSpec();
  _method = method;
  _body = body;

  _first_byte_requested = first_byte;
  _last_byte_requested = last_byte;

  bool result = (bool)EM_ASM_INT({
    var methods = (["OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT"]);
    var xhr = window._httpChannels[$0];
    try {
      xhr.open(methods[$1], UTF8ToString($2), !!$3);
      xhr.withCredentials = true;
      xhr.overrideMimeType("text/plain; charset=x-user-defined");
      if ($4 != 0 || $5 != 0) {
        xhr.setRequestHeader("Range", "bytes=" + $4 + "-" + ($5 || ""));
      }
      xhr.onprogress = null;
      xhr.onreadystatechange = null;
      return 1;
    }
    catch (ex) {
      (console.error || console.log)(ex);
      return 0;
    }
  }, this, method, _request.get_url().c_str(), (int)_nonblocking, (int)first_byte, (int)last_byte);

  if (!result) {
    return false;
  }

  if (_wanted_nonblocking) {
    // Call run() automatically when the state changes.
    EM_ASM({
      var xhr = window._httpChannels[$0];
      xhr.onreadystatechange = function () {
        var xhr = window._httpChannels[$0];
        if (!xhr || !__http_channel_run($0)) {
          xhr.onreadystatechange = null;
        }
      };
    }, this);
  }
  else {
    return run_send() && run_headers_received();
  }

  return true;
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

  EM_ASM({
    var xhr = window._httpChannels[$0];
    xhr.onprogress = null;
    xhr.onreadystatechange = null;
    if (xhr.readyState !== 0) {
      try {
        xhr.abort();
      }
      catch (ex) {
      }
    }
  }, this);

  reset_download_to();

  _status_entry = StatusEntry();

  _bytes_downloaded = 0;
  _bytes_requested = 0;
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

  if (_subdocument_resumes) {
    if (_download_dest == DD_ram) {
      if (_first_byte_delivered > _download_to_ramfile->_data.length()) {
        downloader_cat.info()
          << _NOTIFY_HTTP_CHANNEL_ID
          << "Invalid starting position of byte " << _first_byte_delivered
          << " within Ramfile (which has "
          << _download_to_ramfile->_data.length() << " bytes)\n";
        close_download_stream();
        _status_entry._status_code = SC_download_invalid_range;
        return false;
      }

      if (_first_byte_delivered == 0) {
        _download_to_ramfile->_data = string();
      }
      else {
        _download_to_ramfile->_data =
          _download_to_ramfile->_data.substr(0, _first_byte_delivered);
      }
    }
    else if (_download_dest == DD_stream) {
      _download_to_stream->seekp(_first_byte_delivered);
    }
  }
  else {
    // If _subdocument_resumes is false, we should be sure to reset to the
    // beginning of the file, regardless of the value of
    // _first_byte_delivered.
    if (_download_dest == DD_file || _download_dest == DD_stream) {
      _download_to_stream->seekp(0);
    }
    else if (_download_dest == DD_ram) {
      _download_to_ramfile->_data = string();
    }
  }

  return true;
}

/**
 * Interprets the "Content-Range" header in the reply, and fills in
 * _first_byte_delivered and _last_byte_delivered appropriately if the header
 * response can be understood.
 */
bool HTTPChannel::
parse_content_range(const std::string &content_range) {
  // First, get the units indication.
  size_t p = 0;
  while (p < content_range.length() && !isspace(content_range[p])) {
    p++;
  }

  std::string units = content_range.substr(0, p);
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
 * Resets the indication of how the document will be downloaded.  This must be
 * re-specified after each get_document() (or related) call.
 */
void HTTPChannel::
reset_download_to() {
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
  State state = get_state();
  nassertr(state != S_unsent, false);
  nassertr(ramfile != nullptr, false);

  reset_download_to();
  ramfile->_pos = 0;
  _download_dest = DD_ram;
  _download_to_ramfile = ramfile;
  _subdocument_resumes = (subdocument_resumes && _first_byte_delivered != 0);

  if (state != S_done && _wanted_nonblocking) {
    // In nonblocking mode, we just kick off the request.
    return state != S_opened || run_send();
  }

  // In normal, blocking mode, go ahead and do the download.
  if (!open_download_file()) {
    reset_download_to();
    return false;
  }

  if (state != S_done) {
    while (run()) {
    }
  }

  // Copy the entire response text.
  int bytes_read = EM_ASM_INT({
    var xhr = window._httpChannels[$0];
    var state = xhr.readyState;
    var body = xhr.responseText;
    var ptr = __extend_string($1, body.length);
    for (var i = 0; i < body.length; ++i) {
      HEAPU8[ptr + i] = body.charCodeAt(i);
    }
    return state;
  }, this, &ramfile->_data);

  _bytes_downloaded = bytes_read;

  close_download_stream();

  return is_valid();
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
download_to_stream(std::ostream *strm, bool subdocument_resumes) {
  State state = get_state();
  nassertr(state != S_unsent, false);

  reset_download_to();
  _download_dest = DD_stream;
  _download_to_stream = strm;
  _download_to_stream->clear();
  _subdocument_resumes = subdocument_resumes;

  if (state != S_done && _wanted_nonblocking) {
    // In nonblocking mode, we just kick off the request.
    return state != S_opened || run_send();
  }

  // In normal, blocking mode, go ahead and do the download.
  if (!open_download_file()) {
    reset_download_to();
    return false;
  }

  if (state != S_done) {
    while (run()) {
    }
  }

  // Copy the entire response text.
  char buffer[4096];
  int bytes_read = EM_ASM_INT({
    var xhr = window._httpChannels[$0];
    var state = xhr.readyState;
    var body = xhr.responseText;
    var read = 0;
    while (read < body.length) {
      var size = Math.min(body.length - read, 4096);
      for (var dest = $2; dest < $2 + size; ++dest) {
        HEAP8[(dest>>0)] = body.charCodeAt(read++) & 0xff;
      }
      __write_stream($1, $2, size);
    }
    return read;
  }, this, strm, buffer);

  strm->flush();
  _bytes_downloaded = bytes_read;

  close_download_stream();

  return is_valid();
}

#endif  // __EMSCRIPTEN__
