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

#include <filename.h>
#include <errno.h>
#include <math.h>

#if !defined(WIN32_VC)
  #include <sys/time.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
#else
  #include <winsock2.h>
#endif

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
enum SafeSendCode {
  SS_success = 1,
  SS_error = -1,
  SS_timeout = -2,
};

enum FastReceiveCode {
  FR_eof = 2,
  FR_success = 1,
  FR_error = -1,
  FR_timeout = -2,
  FR_no_data = -3,
};

const int MAX_RECEIVE_BYTES = 16384;

////////////////////////////////////////////////////////////////////
//     Function: Downloader::Constructor
//       Access: Published 
//  Description:
////////////////////////////////////////////////////////////////////
Downloader::
Downloader(void) {
  _frequency = downloader_frequency;
  _byte_rate = downloader_byte_rate;
  _disk_write_frequency = downloader_disk_write_frequency;
  nassertv(_frequency > 0 && _byte_rate > 0 && _disk_write_frequency > 0);
  _receive_size = _byte_rate * _frequency;
  _disk_buffer_size = _disk_write_frequency * _receive_size;
  _buffer = new Buffer(_disk_buffer_size);

  _connected = false;
  // We need to flush after every write in case we're interrupted
  _dest_stream.setf(ios::unitbuf, 0);
  _current_status = NULL;
  _recompute_buffer = false;

  _tfirst = 0.0;
  _tlast = 0.0;
  _got_any_data = false;
  _initiated = false;
  _ever_initiated = false;
  _TCP_stack_initialized = false;
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
  _buffer.clear();
  if (_initiated == true)
    cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::connect_to_server
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Downloader::
connect_to_server(const string &name, uint port) {

#if defined(WIN32)
  if (_TCP_stack_initialized == false) {
    WSAData mydata;
    int answer1 = WSAStartup(0x0101, &mydata);
    if(answer1 != 0) {
      downloader_cat.error()
        << "Downloader::connect_to_server() - WSAStartup() - error: "
        << handle_socket_error() << endl;
      return DL_error_abort;
    }
    _TCP_stack_initialized = true;
  }
#endif

  if (downloader_cat.is_debug())
    downloader_cat.debug()
      << "Downloader connecting to server: " << name << " on port: "
      << port << endl;

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
 	<< handle_socket_error() << endl;
      return get_network_error();
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
int Downloader::
connect_to_server(void) {
  if (_connected == true)
    return DL_success;

  _socket = 0xffffffff;
  _socket = socket(PF_INET, SOCK_STREAM, 0);
  if (_socket == (int)0xffffffff) {
    downloader_cat.error()
      << "Downloader::connect_to_server() - socket failed: "
      << handle_socket_error() << endl;
    return get_network_error();
  }

  if (connect(_socket, (struct sockaddr *)&_sin, sizeof(_sin)) < 0) {
    downloader_cat.error()
      << "Downloader::connect_to_server() - connect() failed: "
      << handle_socket_error() << endl;
    disconnect_from_server();
    _connected = false;
    return get_network_error();
  }

  _connected = true;
  return DL_success;
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
//     Function: Downloader::safe_send
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int Downloader::
safe_send(int socket, const char *data, int length, long timeout) {
  if (length == 0) {
    downloader_cat.error()
      << "Downloader::safe_send() - requested 0 length send!" << endl;
    return DL_error_abort;
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
      return DL_error_network_timeout;
    } else if (sret == -1) {
      downloader_cat.error()
        << "Downloader::safe_send() - error: " << handle_socket_error() 
        << endl;
      return get_network_error();
    }
    int ret = send(socket, data, length, 0);
    if (ret > 0)
      bytes += ret;
    else {
      downloader_cat.error()
        << "Downloader::safe_send() - error: " << handle_socket_error() 
        << endl;
      return get_network_error();
    }
  }
  return DL_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::fast_receive
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int Downloader::
fast_receive(int socket, DownloadStatus *status, int rec_size) {
  nassertr(status != NULL, FR_error);
  if (rec_size <= 0) {
    downloader_cat.error()
      << "Downloader::fast_receive() - Invalid receive size: " << rec_size
      << endl;
    return DL_error_abort;
  }

  // Poll the socket with select() to see if there is any data
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  fd_set rset;
  FD_ZERO(&rset);
  FD_SET(socket, &rset);
  int sret = select(socket, &rset, NULL, NULL, &tv);
  if (sret == 0) {
    return DL_network_no_data;
  } else if (sret == -1) {
    downloader_cat.error()
      << "Downloader::fast_receive() - select() error: " 
      << handle_socket_error() << endl;
    return get_network_error();
  }
  int ret = recv(socket, status->_next_in, rec_size, 0);
  if (ret == 0) {
    return DL_eof;
  } else if (ret == -1) {
    downloader_cat.error()
      << "Downloader::fast_receive() - recv() error: " 
      << handle_socket_error() << endl;
    return get_network_error();
  }
  if (downloader_cat.is_debug())
    downloader_cat.debug()
      << "Downloader::fast_receive() - recv() requested: " << rec_size
      << " got: " << ret << " bytes" << endl;
  status->_next_in += ret;
  status->_bytes_in_buffer += ret;
  status->_total_bytes += ret;
  return DL_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::initiate
//       Access: Published
//  Description: Initiate the download of a complete file from the server.
////////////////////////////////////////////////////////////////////
int Downloader::
initiate(const string &file_name, Filename file_dest) {
  return initiate(file_name, file_dest, 0, 0, 0, false);
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::initiate
//       Access: Published
//  Description: Initiate the download of a file from a server.
////////////////////////////////////////////////////////////////////
int Downloader::
initiate(const string &file_name, Filename file_dest,
        int first_byte, int last_byte, int total_bytes,
        bool partial_content) {

  if (_initiated == true) {
    downloader_cat.error()
      << "Downloader::initiate() - Download has already been initiated"
      << endl;
    return DL_error_abort;
  }

  // Connect to the server
  int connect_ret = connect_to_server();
  if (connect_ret < 0)
    return connect_ret;

  // Attempt to open the destination file
  file_dest.set_binary();
  _dest_stream.setf(ios::unitbuf, 0);
  bool result;
  if (partial_content == true && first_byte > 0)
    result = file_dest.open_append(_dest_stream);
  else
    result = file_dest.open_write(_dest_stream);
  if (result == false) {
    downloader_cat.error()
      << "Downloader::initiate() - Error opening file: " << file_dest
      << " for writing: " << strerror(errno) << endl;
    return DL_error_write;
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
        << "Downloader::initiate() - Requesting byte range: " << first_byte
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
      << "Downloader::initiate() - Sending request:\n" << request << endl;
  int send_ret = safe_send(_socket, request.c_str(), outlen,
                        (long)downloader_timeout);
  if (send_ret < 0)
    return send_ret;

  // Create a download status to maintain download progress information
  _current_status = new DownloadStatus(_buffer->_buffer,
                                first_byte, last_byte, total_bytes,
				partial_content);

  _tfirst = 0.0;
  _tlast = 0.0;
  _got_any_data = false;
  _initiated = true;
  _ever_initiated = true;
  return DL_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::cleanup
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Downloader::
cleanup(void) {
  if (_initiated == false) {
    downloader_cat.error()
      << "Downloader::cleanup() - Download has not been initiated"
      << endl;
    return;
  }

  // The "Connection: close" line tells the server to close the 
  // connection when the download is complete
  _connected = false;
  _dest_stream.close();
  delete _current_status;
  _initiated = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::run
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int Downloader::
run(void) {
  if (_initiated == false) {
    downloader_cat.error()
      << "Downloader::run() - Download has not been initiated"
      << endl;
    return DL_error_abort;
  }

  nassertr(_current_status != NULL, DL_error_abort);

  int connect_ret = connect_to_server();
  if (connect_ret < 0)
    return connect_ret;

  int ret = DL_ok;
  int write_ret;
  double t0 = _clock.get_real_time();
  if (_tfirst == 0.0) {
    _tfirst = t0;
  }
  if (t0 - _tlast < _frequency) 
    return DL_ok;

  // Recompute the buffer size if necessary
  if (_recompute_buffer == true) {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
	<< "Downloader::run() - Recomputing the buffer" << endl;

    // Flush the current buffer if it holds any data
    if (_current_status->_bytes_in_buffer > 0) {
      write_ret = write_to_disk(_current_status);
      if (write_ret < 0)
	return write_ret;

      ret = DL_write;
    }

    // Allocate a new buffer
    _buffer.clear();
    _receive_size = _frequency * _byte_rate;
    _disk_buffer_size = _receive_size * _disk_write_frequency;
    _buffer = new Buffer(_disk_buffer_size);
    _current_status->_buffer = _buffer->_buffer;
    _current_status->reset();
    // Reset the flag
    _recompute_buffer = false;
    // Reset the statistics
    _tfirst = t0;
    _current_status->_total_bytes = 0;

  } else if (_current_status->_bytes_in_buffer + _receive_size > 
						_disk_buffer_size) {

    // Flush the current buffer if the next request would overflow it
    if (downloader_cat.is_debug())
      downloader_cat.debug()
	<< "Downloader::run() - Flushing buffer" << endl;
    write_ret = write_to_disk(_current_status);
    if (write_ret < 0)
      return write_ret;
    ret = DL_write;
  }

  // Attempt to receive the bytes from the socket
  int fret;
  // Handle the case of a fast connection
  if (_receive_size > MAX_RECEIVE_BYTES) {
    int repeat = (int)(_receive_size / MAX_RECEIVE_BYTES);
    int remain = (int)fmod((double)_receive_size, (double)MAX_RECEIVE_BYTES);
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Downloader::run() - fast connection - repeat: " << repeat 
        << " remain: " << remain << endl;
    // Make multiple requests at once but do not exceed MAX_RECEIVE_BYTES
    // for any single request
    for (int i = 0; i <= repeat; i++) {
      if (i < repeat)
        fret = fast_receive(_socket, _current_status, MAX_RECEIVE_BYTES);
      else if (remain > 0)
        fret = fast_receive(_socket, _current_status, remain);
      if (fret == DL_eof || fret < 0) {
	break;
      } else if (fret == DL_success) {
        _got_any_data = true;
      }
    }
  } else { // Handle the normal speed connection case
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Downloader::run() - normal connection" << endl;
    fret = fast_receive(_socket, _current_status, _receive_size);
  }
  _tlast = _clock.get_real_time();

  // Check for end of file
  if (fret == DL_eof) {
    if (_got_any_data == true) {
      if (_current_status->_bytes_in_buffer > 0) {
	write_ret = write_to_disk(_current_status);
 	if (write_ret < 0)
          return write_ret;
      }
      if (downloader_cat.is_debug())
        downloader_cat.debug()
	  << "Downloader::run() - Got eof" << endl;
      cleanup();
      return DL_success;
    } else {
      if (downloader_cat.is_debug())
	downloader_cat.debug()
	  << "Downloader::run() - Got 0 bytes" << endl;
      return ret;
    }
  } else if (fret == DL_network_no_data) {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
	<< "Downloader::run() - No data" << endl;
      return ret;
  } else if (fret < 0) {
    return fret;
  }

  _got_any_data = true;
  return ret;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::parse_http_response
//       Access: Private
//  Description: Check the HTTP response from the server
////////////////////////////////////////////////////////////////////
int Downloader::
parse_http_response(const string &resp) {
  size_t ws = resp.find(" ", 0);
  string httpstr = resp.substr(0, ws);
  if (!(httpstr == "HTTP/1.1")) {
    downloader_cat.error()
      << "Downloader::parse_http_response() - not HTTP/1.1 - got: "
      << httpstr << endl;
    return DL_error_abort;
  }
  size_t ws2 = resp.find(" ", ws);
  string numstr = resp.substr(ws, ws2);
  nassertr(numstr.length() > 0, false);
  int num = atoi(numstr.c_str());
  switch (num) {
    case 200:
    case 206:
      return DL_success;
    case 202:
      // Accepted - server may not honor request, though
      if (downloader_cat.is_debug())
        downloader_cat.debug()
          << "Downloader::parse_http_response() - got a 202 Accepted - "
          << "server does not guarantee to honor this request" << endl;
      return DL_success;
    case 201:
    case 203:
    case 204:
    case 205:
      break;
    case 408:
      return DL_error_http_server_timeout;
    case 503:
      return DL_error_http_service_unavailable;
    case 504:
      return DL_error_http_gateway_timeout;
    default:
      break;
  }

  downloader_cat.error()
    << "Downloader::parse_http_response() - Invalid response: "
    << resp << endl;
  return DL_error_abort;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::parse_header
//       Access: Private
//  Description: Looks for a valid header.  If it finds one, it
//               calculates the header length and strips it from
//               the download status structure.  Function returns false
//               on an error condition, otherwise true.
////////////////////////////////////////////////////////////////////
int Downloader::
parse_header(DownloadStatus *status) {
  nassertr(status != NULL, DL_error_abort);

  if (status->_header_is_complete == true)
    return DL_success;

  if (status->_bytes_in_buffer == 0) {
    downloader_cat.error()
      << "Downloader::parse_header() - Empty buffer!" << endl;
    return DL_error_abort;
  }

  string bufstr((char *)status->_start, status->_bytes_in_buffer);
  size_t p  = 0;
  while (p < bufstr.length()) {
    // Server sends out CR LF (\r\n) as newline delimiter
    size_t nl = bufstr.find("\015\012", p);
    if (nl == string::npos) {
      downloader_cat.error()
        << "Downloader::parse_header() - No newlines in buffer of "
        << "length: " << status->_bytes_in_buffer << endl;
      return DL_error_abort;
    } else if (p == 0 && nl == p) {
      downloader_cat.error()
        << "Downloader::parse_header() - Buffer begins with newline!"
        << endl;
        return DL_error_abort;
    }

    string component = bufstr.substr(p, nl - p);

    // The first line of the response should say whether
    // got an error or not
    if (status->_first_line_complete == false) {
      status->_first_line_complete = true;
      int parse_ret = parse_http_response(component);
      if (parse_ret == DL_success) {
        if (downloader_cat.is_debug())
          downloader_cat.debug()
            << "Downloader::parse_header() - Header is valid: "
            << component << endl;
        status->_header_is_valid = true;
      } else {
        return parse_ret;
      }
    }

    // Look for content length
    size_t cpos = component.find(":");
    string tline = component.substr(0, cpos);
    if (status->_partial_content == true && tline == "Content-Length") {
      tline = component.substr(cpos + 2, string::npos);
      int server_download_bytes = atoi(tline.c_str());
      int client_download_bytes = status->_last_byte - status->_first_byte;
      if (status->_first_byte == 0)
        client_download_bytes += 1;
      if (client_download_bytes != server_download_bytes) {
        downloader_cat.error()
          << "Downloader::parse_header() - server size = "
          << server_download_bytes << ", client size = "
          << client_download_bytes << " ("
          << status->_last_byte << "-" << status->_first_byte << ")" << endl;
        return DL_error_abort;
      }
    }

    // Two consecutive (CR LF)s indicates end of HTTP header
    if (nl == p) {
      if (downloader_cat.is_debug())
        downloader_cat.debug()
          << "Downloader::parse_header() - Header is complete" << endl;
      status->_header_is_complete = true;

      // Strip the header out of the status buffer
      int header_length = nl + 2;
      status->_start += header_length;
      status->_bytes_in_buffer -= header_length;

      if (downloader_cat.is_debug())
        downloader_cat.debug()
          << "Downloader::parse_header() - Stripping out header of size: "
          << header_length << endl;

      return DL_success;
    }

    p = nl + 2;
  }

  if (status->_header_is_complete == false) {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Downloader::parse_header() - Reached end of buffer without "
        << "successfully parsing the header - buffer size: "
        << status->_bytes_in_buffer << endl;
    return DL_error_abort;
  }

  return DL_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::write_to_disk
//       Access: Private
//  Description: Writes a download to disk.  If there is a header,
//               the pointer and size are adjusted so the header
//               is excluded.  Function returns false on error
//               condition.
////////////////////////////////////////////////////////////////////
int Downloader::
write_to_disk(DownloadStatus *status) {
  nassertr(status != NULL, DL_error_abort);

  // Ensure the header has been parsed successfully first
  int parse_ret = parse_header(status);
  if (parse_ret < 0)
    return parse_ret;

  if (status->_header_is_complete == false) {
    downloader_cat.error()
      << "Downloader::write_to_disk() - Incomplete HTTP header - "
      << "(or header was larger than download buffer) - "
      << "try increasing download-buffer-size" << endl;
    return DL_error_abort;
  }

  // Write what we have so far to disk
  if (status->_bytes_in_buffer > 0) {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Downloader::write_to_disk() - Writing "
        << status->_bytes_in_buffer << " to disk" << endl;

    _dest_stream.write(status->_start, status->_bytes_in_buffer);
    status->_total_bytes_written += status->_bytes_in_buffer;
  }

  status->reset();

  return DL_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::DownloadStatus::constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
Downloader::DownloadStatus::
DownloadStatus(char *buffer, int first_byte, int last_byte, 
				int total_bytes, bool partial_content) {
  _first_line_complete = false;
  _header_is_complete = false;
  _header_is_valid = false;
  _buffer = buffer;
  _first_byte = first_byte;
  _last_byte = last_byte;
  _total_bytes = total_bytes;
  _total_bytes_written = 0;
  _partial_content = partial_content;
  reset();
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::DownloadStatus::reset
//       Access: Public
//  Description: Resets the status buffer for more downloading after
//               a write.
////////////////////////////////////////////////////////////////////
void Downloader::DownloadStatus::
reset(void) {
  _start = _buffer;
  _next_in = _start;
  _bytes_in_buffer = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::handle_socket_error
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
char *Downloader::
handle_socket_error(void) const {
#ifndef WIN32
  return strerror(errno);
#else
  switch (WSAGetLastError()) {
    case 10022:
      return "An invalid argument was supplied";
    case 10036:
      return "A blocking operation is currently executing";
    case 10040:
      return "Message was larger than internal buffer or network limit";
    case 10050:
      return "Network dead";
    case 10051:
      return "Network unreachable";
    case 10052:
      return "Connection broken because keep-alive detected a failure";
    case 10053:
      return "Connection aborted by local host software";
    case 10054:
      return "Connection closed by remote host";
    case 10055:
      return "Out of buffer space or queue overflowed";
    case 10057:
      return "Socket was not connected";
    case 10058:
      return "Socket was previously shut down";
    case 10060:
      return "Connection timed out";
    case 10061:
      return "Connection refused by remote host";
    case 10064:
      return "Remote host is down";
    case 10065:
      return "Remote host is unreachable";
    case 10093:
      return "WSAStartup() was not called";
    default:
      return "Unknown WSA error";
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Downloader::error_gethostbyname
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int Downloader::
get_network_error(void) const {
#ifndef WIN32
  return DL_error_abort;
#else
  switch (WSAGetLastError()) {
    case 10050:
      return DL_error_network_dead;
    case 10051:
      return DL_error_network_unreachable;
    case 10052:
    case 10057:
    case 10058:
      return DL_error_network_disconnected;
    case 10053:
      return DL_error_network_disconnected_locally;
    case 10054:
    case 10061:
      return DL_error_network_remote_host_disconnected;
    case 10055:
      return DL_error_network_buffer_overflow;
    case 10060:
      return DL_error_network_timeout;
    case 10064:
      return DL_error_network_remote_host_down;
    case 10065:
      return DL_error_network_remote_host_unreachable;
    case 10069:
      return DL_error_network_disk_quota_exceeded;
    case 11001:
      return DL_error_network_remote_host_not_found;
    case 11002:
      return DL_error_network_remote_host_no_response;
    default:
      return DL_error_abort;
  }
#endif
}
