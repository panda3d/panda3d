// Filename: downloader.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef ASYNCDOWNLOADER_H
#define ASYNCDOWNLOADER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <notify.h>
#include <filename.h>
#include <tokenBoard.h>
#include <buffer.h>
#include "asyncUtility.h"

#if defined(WIN32_VC)
  #include <winsock2.h>
#else
  #include <netinet/in.h>  // Irix seems to require this one for resolv.h.
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <resolv.h>
#endif

class DownloaderToken;

////////////////////////////////////////////////////////////////////
//       Class : Downloader
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Downloader : public AsyncUtility {
PUBLISHED:
  enum DownloadCode {
    DS_write = 2,
    DS_success = 1,
    DS_abort = -1,
    DS_timeout = -2
  };
  Downloader(void);
  //Downloader(PT(Buffer) buffer);
  virtual ~Downloader(void);

  bool connect_to_server(const string &name, uint port=80);
  void disconnect_from_server(void);

  int request_sync_download(const string &file_name, const Filename &file_dest,
			const string &event_name);
  int request_sync_download(const string &file_name, const Filename &file_dest,
			const string &event_name, int first_byte,
			int last_byte, int total_bytes,
			bool partial_content = true);
  int request_download(const string &file_name, const Filename &file_dest,
			const string &event_name, bool sync = false);
  int request_download(const string &file_name, const Filename &file_dest,
			const string &event_name, int first_byte,
			int last_byte, int total_bytes, 
			bool partial_content = true, bool sync = false);

  INLINE void set_byte_rate(float bytes);
  INLINE float get_byte_rate(void) const;
  INLINE void set_disk_write_frequency(int frequency);
  INLINE int get_disk_write_frequency(void) const;
  INLINE void enable_download(bool val);
  INLINE bool is_download_enabled(void) const;
  INLINE bool get_last_attempt_stalled(void) const;

private:
  class DownloadStatus {
  public:
    DownloadStatus(char *buffer, const string &event_name, int first_byte,
			int last_byte, int total_bytes, bool partial_content,
			uint id);
    void reset(void);

  public:
    bool _first_line_complete;
    bool _header_is_complete;
    bool _header_is_valid;
    char *_start;
    char *_next_in;
    int _bytes_in_buffer;
    string _event_name;
    int _total_bytes_written;
    int _first_byte;
    int _last_byte;
    int _total_bytes;
    bool _partial_content;
    uint _id;
    char *_buffer;
  };

  void init();
  int download(const string &file_name, Filename file_dest, 
			const string &event_name, int first_byte,
			int last_byte, int total_bytes, bool partial_content,
			bool sync, uint id);
  virtual bool process_request(void);
  bool parse_header(DownloadStatus &status);
  bool write_to_disk(DownloadStatus &status);
  bool connect_to_server(void);
  int safe_send(int socket, const char *data, int length, long timeout);
  int safe_receive(int socket, DownloadStatus &status, int length, 
				long timeout, int &bytes);
  bool parse_http_response(const string &resp);
  int attempt_read(int length, DownloadStatus &status, int &bytes_read);

  typedef TokenBoard<DownloaderToken> DownloaderTokenBoard;
  DownloaderTokenBoard *_token_board;

  bool _connected;

#ifdef HAVE_IPC
  mutex _buffer_lock;
#endif

  int _socket;
  PT(Buffer) _buffer;
  int _disk_write_frequency;
  int _new_disk_write_frequency;
  float _byte_rate; 
  float _new_byte_rate;
  int _read_size;
  bool _download_enabled;
  ofstream _dest_stream;
  int _disk_buffer_size;
  bool _last_attempt_stalled;
  bool _current_attempt_stalled;

  string _server_name;
  struct sockaddr_in _sin;
};

#include "asyncDownloader.I"

#endif
