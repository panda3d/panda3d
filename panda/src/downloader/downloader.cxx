// Filename: downloader.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "downloader.h"
#include "config_downloader.h"

#include <event.h>
#include <pt_Event.h>
#include <throw_event.h>
#include <eventParameter.h>
#include <circBuffer.h>
#include <filename.h>
#include <list>
#include <errno.h>

#if !defined(WIN32_VC)
// #define errno wsaGetLastError()
  #include <sys/time.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
#endif

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
enum send_status {
  SS_error,
  SS_timeout,
  SS_success
};

enum receive_status {
  RS_error,
  RS_timeout,
  RS_success,
  RS_eof
};

////////////////////////////////////////////////////////////////////
//       Class : DownloaderToken
// Description : Holds a request for the downloader.
////////////////////////////////////////////////////////////////////
class DownloaderToken : public ReferenceCount {
public:
  INLINE DownloaderToken(uint id, const string &file_name, 
	const Filename &file_dest, const string &event_name,
	int first_byte, int last_byte, int total_bytes,
	bool partial_content) : _id(id), _first_byte(first_byte), 
		_last_byte(last_byte), _total_bytes(total_bytes) {
    _file_name = file_name;
    _event_name = event_name;
    _file_dest = file_dest;
    _partial_content = partial_content;
  }
  uint _id;
  string _file_name; 
  Filename _file_dest;
  string _event_name;
  int _first_byte;
  int _last_byte; 
  int _total_bytes;
  bool _partial_content;
};

////////////////////////////////////////////////////////////////////
//     Function: Downloader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Downloader::
Downloader(void) : AsyncUtility() {
  PT(Buffer) buffer = new Buffer(downloader_buffer_size);
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Downloader::
Downloader(PT(Buffer) buffer) : AsyncUtility() {
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::init
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Downloader::
init(PT(Buffer) buffer) {
  nassertv(!buffer.is_null());
  _frequency = downloader_frequency;
  _bandwidth = downloader_bandwidth;
  _connected = false;
  _token_board = new DownloaderTokenBoard;
  _buffer = buffer; 
  _download_enabled = true;
  // We need to flush after every write in case we're interrupted
  _dest_stream.setf(ios::unitbuf, 0);
  _buffer_size = _buffer->get_length();
  _new_buffer_size = 0;

#if defined(WIN32)
  WSAData mydata;
  int answer1 = WSAStartup(0x0101, &mydata);
  if(answer1 != 0) {
    downloader_cat.error()
      << "Downloader::Downloader() - Error initializing TCP stack!"
      << endl;
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Downloader::
~Downloader() {
  if (_connected)
    disconnect_from_server();

  destroy_thread();

  delete _token_board;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::connect_to_server
//       Access: Public 
//  Description:
////////////////////////////////////////////////////////////////////
bool Downloader::
connect_to_server(const string &name, uint port) {
  if (downloader_cat.is_debug())
    downloader_cat.debug()
      << "Downloader connecting to server: " << name 
      << " on port: " << port << endl;

  _server_name = name;

  _sin.sin_family = PF_INET;
  _sin.sin_port = htons(port);
  ulong addr = (ulong)inet_addr(name.c_str());
  struct hostent *hp = NULL;

  if (addr == INADDR_NONE) {
    hp = gethostbyname(name.c_str());
    if (hp != NULL)
      (void)memcpy(&_sin.sin_addr, hp->h_addr, (uint)hp->h_length);
    else {
      downloader_cat.error()
	<< "Downloader::connect_to_server() - gethostbyname() failed: " 
	<< strerror(errno) << endl;
      return false;
    }
  } else
    (void)memcpy(&_sin.sin_addr, &addr, sizeof(addr));

  return connect_to_server();
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::connect_to_server
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool Downloader::
connect_to_server(void) {
  if (_connected == true)
    return true;

  _socket = 0xffffffff;
  _socket = socket(PF_INET, SOCK_STREAM, 0);
  if (_socket == (int)0xffffffff) {
    downloader_cat.error()
      << "Downloader::connect_to_server() - socket failed: "
      << strerror(errno) << endl;
    return false;
  }

  _connected = true;

  if (connect(_socket, (struct sockaddr *)&_sin, sizeof(_sin)) < 0) {
    downloader_cat.error()
        << "Downloader::connect_to_server() - connect() failed: "
        << strerror(errno) << endl;
    disconnect_from_server();
    _connected = false;
  }

  return _connected;
}

///////////////////////////////////////////////////////////////////
//     Function: Downloader::disconnect_from_server
//       Access: Public 
//  Description:
////////////////////////////////////////////////////////////////////
void Downloader::
disconnect_from_server(void) {
  if (downloader_cat.is_debug())
    downloader_cat.debug()
      << "Downloader disconnecting from server..." << endl;

#if defined(WIN32)
  (void)closesocket(_socket);
#else
  (void)close(_socket);
#endif

  _connected = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::request_download
//       Access: Public
//  Description: Requests the download of a complete file. 
////////////////////////////////////////////////////////////////////
int Downloader::
request_download(const string &file_name, const Filename &file_dest,
		const string &event_name) {
  return request_download(file_name, file_dest, event_name, 0, 0, 0,
				false);
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::request_download
//       Access: Public
//  Description: Requests an asynchronous load of a file.  The request
//               will be queued and served by the asynchronous thread.
//               If event_name is nonempty, it is the name of the
//               event that will be thrown (with the uint id as its
//               single parameter) when the loading is completed later.
//
//               The return value is an integer which can be used to
//               identify this particular request later to
//               fetch_load(), or 0 if there has been an error.
//
//		 Can be used to request a partial download of a file.
////////////////////////////////////////////////////////////////////
int Downloader::
request_download(const string &file_name, const Filename &file_dest,
			const string &event_name, int first_byte,
			int last_byte, int total_bytes,
			bool partial_content) {

  nassertr(first_byte <= last_byte && last_byte <= total_bytes, 0);

  PT(DownloaderToken) tok;
  if (_threads_enabled) {

    // Make sure we actually are threaded
    if (!_threaded) {
      downloader_cat.info()
        << "Downloader::request_download() - create_thread() was "
        << "never called!  Calling it now..." << endl;
      create_thread();
    }

    // We need to grab the lock in order to signal the condition variable
#ifdef HAVE_IPC
    _lock.lock();
#endif

      if (_token_board->_waiting.is_full()) {
        downloader_cat.error()
          << "Downloader::request_download() - Too many pending requests\n";
        return 0;
      }

      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Download requested for file: " << file_name << "\n";
      }

      tok = new DownloaderToken(_next_token++, file_name, file_dest, 
		event_name, first_byte, last_byte, total_bytes, 
					partial_content);
      _token_board->_waiting.insert(tok);

#ifdef HAVE_IPC
      _request_cond->signal();
    _lock.unlock();
#endif

  } else {
    // If we're not running asynchronously, process the load request
    // directly now.
    if (_token_board->_waiting.is_full()) {
      downloader_cat.error()
        << "Downloader::request_download() - Too many pending requests\n";
      return 0;
    }
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << "Load requested for file: " << file_name << "\n";
    }

    tok = new DownloaderToken(_next_token++, file_name, file_dest, 
		event_name, first_byte, last_byte, total_bytes, 
					partial_content);
    _token_board->_waiting.insert(tok);
    process_request();
  }

  return tok->_id;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::process_request
//       Access: Private
//  Description: Serves any requests on the token board, moving them
//               to the done queue.
////////////////////////////////////////////////////////////////////
bool Downloader::
process_request() {
  if (_shutdown) {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Downloader shutting down...\n";
    return false;
  }

  // If there is actually a request token - process it
  while (!_token_board->_waiting.is_empty()) {
    PT(DownloaderToken) tok = _token_board->_waiting.extract();
    if (download(tok->_file_name, tok->_file_dest, tok->_event_name,
		 tok->_first_byte, tok->_last_byte, tok->_total_bytes,
		 tok->_partial_content, tok->_id)) {
      _token_board->_done.insert(tok);

      // Throw a "done" event now.
      if (!tok->_event_name.empty()) {
        PT_Event done = new Event(tok->_event_name);
        done->add_parameter(EventParameter((int)tok->_id));
        throw_event(done);
      }

      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Downloader::process_request() - downloading complete for " 
	  << tok->_file_name << "\n";
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::safe_send
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int Downloader::
safe_send(int socket, const char *data, int length, long timeout) {
  if (length == 0) {
    downloader_cat.error()
      << "Downloader::safe_send() - requested 0 length send!" << endl;
    return SS_error;
  }
  int bytes = 0;
  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;
  fd_set wset;
  FD_ZERO(&wset);
  while (bytes < length) {
    FD_SET(socket, &wset);
    int sret = select(socket + 1, NULL, &wset, NULL, &tv);
    if (sret == 0) {
      downloader_cat.error()
	<< "Downloader::safe_send() - select timed out after: "
	<< timeout << " seconds" << endl;
      return SS_timeout;
    } else if (sret == -1) {
      downloader_cat.error()
	<< "Downloader::safe_send() - error: " << strerror(errno) << endl;
      return SS_error;
    }
    int ret = send(socket, data, length, 0);
    if (ret > 0)
      bytes += ret;
    else {
      downloader_cat.error()
	<< "Downloader::safe_send() - error: " << strerror(errno) << endl;
      return SS_error;
    }
  }
  return SS_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::safe_receive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int Downloader::
safe_receive(int socket, char *data, int length, long timeout, int &bytes) {
  char *data_ptr = data;
  if (length == 0) {
    downloader_cat.error()
      << "Downloader::safe_receive() - requested 0 length receive!" << endl;
    return RS_error;
  }
  bytes = 0;
  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;
  fd_set rset;
  FD_ZERO(&rset);
  while (bytes < length) {
    FD_SET(socket, &rset);
    int sret = select(socket + 1, &rset, NULL, NULL, &tv);
    if (sret == 0) {
      downloader_cat.warning()
	<< "Downloader::safe_receive() - select timed out after: "
	<< timeout << " seconds" << endl;
      return RS_timeout;
    } else if (sret == -1) {
      downloader_cat.error()
	<< "Downloader::safe_receive() - error: " << strerror(errno) << endl;
      return RS_error;
    }
    int ret = recv(socket, data_ptr, length - bytes, 0);
    if (ret > 0) {
      if (downloader_cat.is_debug())
        downloader_cat.debug()
	  << "Downloader::safe_receive() - recv() got: " << ret << " bytes"
	  << endl;
      bytes += ret;
      data_ptr += ret;
    } else if (ret == 0) {
      if (downloader_cat.is_debug())
        downloader_cat.debug()
	  << "Downloader::safe_receive() - End of file" << endl;
      return RS_eof;
    } else {
      downloader_cat.error()
	<< "Downloader::safe_receive() - error: " << strerror(errno) << endl;
      return RS_error;
    }
  }
  return RS_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::download
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool Downloader::
download(const string &file_name, Filename file_dest, 
		const string &event_name, int first_byte, int last_byte,
		int total_bytes, bool partial_content, uint id) {

  // Make sure we are still connected to the server
  if (connect_to_server() == false)
    return false;

  // Attempt to open the destination file
  file_dest.set_binary();
  bool result;
  if (partial_content == true && first_byte > 0)
    result = file_dest.open_append(_dest_stream);
  else
    result = file_dest.open_write(_dest_stream);
  if (result == false) {
    downloader_cat.error()
      << "Downloader::download() - Error opening file: " << file_dest
      << " for writing" << endl;
    return false;
  }

  // Send an HTTP request for the file to the server
  string request = "GET ";
  request += file_name;
  request += " HTTP/1.1\012Host: ";
  request += _server_name;
  request += "\012Connection: close";
  if (partial_content == true) {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Downloader::download() - Requesting byte range: " << first_byte 
        << "-" << last_byte << endl;
    request += "\012Range: bytes=";
    stringstream start_stream;
    start_stream << first_byte << "-" << last_byte;
    request += start_stream.str();
  }
  request += "\012\012";
  int outlen = request.size();
  if (downloader_cat.is_debug())
    downloader_cat.debug()
      << "Downloader::download() - Sending request:\n" << request << endl;
  int send_ret = safe_send(_socket, request.c_str(), outlen, 
			(long)downloader_timeout);

  if (send_ret == SS_timeout) {
    for (int sr = 0; sr < downloader_timeout_retries; sr++) {
      send_ret = safe_send(_socket, request.c_str(), outlen,
				(long)downloader_timeout);
      if (send_ret != SS_timeout)
        break;
    }
    if (send_ret == SS_timeout) {
      // We've really timed out - throw an event
      downloader_cat.error()
        << "Downloader::download() - send timed out after: " 
        << downloader_timeout_retries << " retries" << endl;
      PT_Event timeout_event = new Event(event_name);
      timeout_event->add_parameter(EventParameter((int)id));
      timeout_event->add_parameter(EventParameter(0));
      timeout_event->add_parameter(EventParameter(-1));
      throw_event(timeout_event);
      return false;
    }
  }

  if (send_ret == SS_error)
    return false;

  // Loop at the requested frequency until the download completes
  DownloadStatus status(_buffer->_buffer, event_name, first_byte, last_byte,
			total_bytes, partial_content, id);
  bool got_any_data = false;
  
  for (;;) {
    if (_download_enabled) { 
      // Ensure that these don't change while we're computing read_size
#ifdef HAVE_IPC
      _bandwidth_frequency_lock.lock();
#endif
	int read_size = (int)_bandwidth;
	if (_frequency > 0)
 	  read_size = (int)(_bandwidth * _frequency);	
#ifdef HAVE_IPC
      _bandwidth_frequency_lock.unlock();
#endif

      // Ensure we have enough room in the buffer to download read_size
      // If we don't have enough room, write the buffer to disk
      if (status._bytes_in_buffer + read_size > _buffer_size) {
	if (downloader_cat.is_debug())
	  downloader_cat.debug()
	    << "Downloader::download() - Flushing buffer" << endl;

	if (write_to_disk(status) == false)
	  return false;
      }

      // Grab the next chunk
      int bytes = 0;
      int ans = safe_receive(_socket, status._next_in, read_size,
					(long)downloader_timeout, bytes); 

      // Handle receive timeouts by trying again
      if (ans == RS_timeout) {
	nassertr(bytes == 0, false);
	for (int r = 0; r < downloader_timeout_retries; r++) {
	    ans = safe_receive(_socket, status._next_in, read_size,
					(long)downloader_timeout, bytes);
	    if (ans != RS_timeout)
	      break;
	}
	if (ans == RS_timeout) {
	  // We've really timed out - throw an event
	  downloader_cat.error()
	    << "Downloader::download() - receive timed out after: " 
	    << downloader_timeout_retries << " retries" << endl;
	  if (bytes > 0) {
	    status._bytes_in_buffer += bytes;
   	    status._next_in += bytes;
	    if (write_to_disk(status) == false) {
	      downloader_cat.error()
		<< "Downloader::download() - write to disk failed after "
		<< "timeout!" << endl;
	    }
	  }
          PT_Event timeout_event = new Event(status._event_name);
          timeout_event->add_parameter(EventParameter((int)status._id));
          timeout_event->add_parameter(
				EventParameter(status._total_bytes_written));
	  timeout_event->add_parameter(EventParameter(-1));
          throw_event(timeout_event);
	  return false;
	}
      }

      // Handle receive errors
      if (ans == RS_error) {
        downloader_cat.error()
          << "Downloader::download() - Error reading from socket: "
	  << strerror(errno) << endl;
	return false;
      }

      if (ans == RS_eof) {

	if (bytes > 0 || got_any_data == true) {
	  if (downloader_cat.is_debug())
	    downloader_cat.debug()
	      << "Download for: " << file_name << " completed" << endl;
	  if (bytes > 0) {
	    status._bytes_in_buffer += bytes;
   	    status._next_in += bytes;
	  }
	  bool ret = write_to_disk(status);
	  _dest_stream.close();

	  // The "Connection: close" line tells server to close connection
	  // when the download is complete
	  _connected = false;
	  return ret;
	} else {
	  if (downloader_cat.is_debug())
	    downloader_cat.debug()
	      << "Downloader::download() - Received 0 bytes" << endl;
	  nap();
	}

      } else { // ans == RS_success
	if (downloader_cat.is_debug())
	  downloader_cat.debug()
	    << "Downloader::download() - Got: " << bytes << " bytes" << endl;
	status._bytes_in_buffer += bytes;
   	status._next_in += bytes;
	got_any_data = true;

	// Sleep for the requested frequency
	nap();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::parse_http_response
//       Access: Private
//  Description: Check the HTTP response from the server
////////////////////////////////////////////////////////////////////
bool Downloader::
parse_http_response(const string &resp) {
  size_t ws = resp.find(" ", 0);
  string httpstr = resp.substr(0, ws);
  if (!(httpstr == "HTTP/1.1")) {
    downloader_cat.error()
      << "Downloader::parse_http_response() - not HTTP/1.1 - got: " 
      << httpstr << endl;
    return false;
  }
  size_t ws2 = resp.find(" ", ws);
  string numstr = resp.substr(ws, ws2);
  nassertr(numstr.length() > 0, false);
  int num = atoi(numstr.c_str());
  switch (num) {
    case 200:
    case 206:
      return true;
    case 202:
      // Accepted - server may not honor request, though
      if (downloader_cat.is_debug())
	downloader_cat.debug()
	  << "Downloader::parse_http_response() - got a 202 Accepted - "
	  << "server does not guarantee to honor this request" << endl;
      return true;
    case 201:
    case 203:
    case 204:
    case 205:
    default:
      break;
  }

  downloader_cat.error()
    << "Downloader::parse_http_response() - Invalid response: "
    << resp << endl;
  return false; 
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::parse_header
//       Access: Private
//  Description: Looks for a valid header.  If it finds one, it 
//		 calculates the header length and strips it from
//		 the download status structure.  Function returns false
//		 on an error condition, otherwise true.
////////////////////////////////////////////////////////////////////
bool Downloader::
parse_header(DownloadStatus &status) {

  if (status._header_is_complete == true)
    return true;

  string bufstr((char *)status._start, status._bytes_in_buffer);
  size_t p  = 0;
  while (p < bufstr.length()) {
    // Server sends out CR LF (\r\n) as newline delimiter
    size_t nl = bufstr.find("\015\012", p);
    if (nl == string::npos)
      return true;

    string component = bufstr.substr(p, nl - p);

    // The first line of the response should say whether
    // got an error or not
    if (status._first_line_complete == false) {
      status._first_line_complete = true;
      if (parse_http_response(component) == true) {
  	if (downloader_cat.is_debug())
          downloader_cat.debug()
	    << "Downloader::parse_header() - Header is valid: " 
	    << component << endl;
	status._header_is_valid = true;
      } else {
	return false;
      }
    }

    // Look for content length
    size_t cpos = component.find(":");
    string tline = component.substr(0, cpos);
    if (status._partial_content == true && tline == "Content-Length") {
      tline = component.substr(cpos + 2, string::npos);
      int server_download_bytes = atoi(tline.c_str());
      int client_download_bytes = status._last_byte - status._first_byte;
      if (status._first_byte == 0)
	client_download_bytes += 1;
      if (client_download_bytes != server_download_bytes) {
  	downloader_cat.error()
	  << "Downloader::parse_header() - server size = " 
	  << server_download_bytes << ", client size = " 
	  << client_download_bytes << " ("
	  << status._last_byte << "-" << status._first_byte << ")" << endl;
	return false;
      }
    } 

    // Two consecutive (CR LF)s indicates end of HTTP header
    if (nl == p) {
      if (downloader_cat.is_debug())
        downloader_cat.debug()
	  << "Downloader::parse_header() - Header is complete" << endl;
      status._header_is_complete = true;
      
      // Strip the header out of the status buffer
      int header_length = nl + 2;
      status._start += header_length;
      status._bytes_in_buffer -= header_length;

      if (downloader_cat.is_debug())
        downloader_cat.debug()
	  << "Downloader::parse_header() - Stripping out header of size: "
	  << header_length << endl;

      return true;
    }

    p = nl + 2;
  }

  if (status._total_bytes == 0) {
    downloader_cat.error()
      << "Downloader::parse_header() - Total bytes == 0!" << endl;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::write_to_disk
//       Access: Private
//  Description: Writes a download to disk.  If there is a header,
//		 the pointer and size are adjusted so the header
//		 is excluded.  Function returns false on error
//		 condition.
////////////////////////////////////////////////////////////////////
bool Downloader::
write_to_disk(DownloadStatus &status) {

  // Ensure the header has been parsed successfully first
  if (parse_header(status) == false)
    return false;

  if (status._header_is_complete == false) {
    downloader_cat.error()
      << "Downloader::write_to_disk() - Incomplete HTTP header - "
      << "(or header was larger than download buffer) - "
      << "try increasing download-buffer-size" << endl;
    return false;
  } 

  // Write what we have so far to disk
  if (status._bytes_in_buffer > 0) {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Downloader::write_to_disk() - Writing " 
        << status._bytes_in_buffer << " to disk" << endl;
      
    _dest_stream.write(status._start, status._bytes_in_buffer);
    status._total_bytes_written += status._bytes_in_buffer;

    // Throw an event to indicate how many bytes have been written so far
    if (!status._event_name.empty()) {
      PT_Event write_event = new Event(status._event_name);
      write_event->add_parameter(EventParameter((int)status._id));
      write_event->add_parameter(EventParameter(status._total_bytes_written));
      throw_event(write_event);
    }
  }

  status.reset();

  // Now see if we need to adjust the buffer size
  if (_new_buffer_size > 0) {
#ifdef HAVE_IPC
    _buffer_lock.lock();
#endif

    if (downloader_cat.is_debug())
      downloader_cat.debug()
	<< "Downloader::write_to_buffer() - resizing buffer to: "
	<< _new_buffer_size << endl;
    _buffer.clear();
    _buffer = new Buffer(_new_buffer_size);
    _buffer_size = _new_buffer_size;
    _new_buffer_size = 0;

#ifdef HAVE_IPC
    _buffer_lock.unlock();
#endif
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::DownloadStatus::constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
Downloader::DownloadStatus::
DownloadStatus(char *buffer, const string &event_name, int first_byte,
	int last_byte, int total_bytes, bool partial_content, uint id) {
  _first_line_complete = false;
  _header_is_complete = false;
  _header_is_valid = false;
  _buffer = buffer;
  _event_name = event_name;
  _first_byte = first_byte;
  _last_byte = last_byte;
  _total_bytes = total_bytes;
  _partial_content = partial_content;
  _id = id;
  reset();
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::DownloadStatus::reset
//       Access: Public 
//  Description: Resets the status buffer for more downloading after
//		 a write.
////////////////////////////////////////////////////////////////////
void Downloader::DownloadStatus::
reset(void) {
  _start = _buffer;
  _next_in = _start;
  _bytes_in_buffer = 0;
  _total_bytes_written = 0;
}
